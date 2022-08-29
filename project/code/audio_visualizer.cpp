enum asset_font_type
{
    FontType_Default = 0,
};

enum asset_tag_id
{
    Tag_UnicodeCodepoint,
    Tag_FontType,
    
    Tag_Count
};

enum asset_type_id
{
    Asset_None,
    // Menu
    Asset_MenuBackground,
    Asset_MenuLogo,
    Asset_DefaultCheckBox,
    Asset_HighlightCheckBox,
    Asset_ActiveCheckBox,
    Asset_ActiveHighlightCheckBox,
    
    //
    // Fonts
    //
    Asset_Font,
    Asset_FontGlyph,
    
    //
    // Sounds
    //
    Asset_Song,
    
    Asset_Count
};

#define WindowName "Audio Visualizer"
#define ClientHeight 0.80f
#define ClientWidth 0.0f

#define Permanent_Storage_Size Megabytes(126)
#define Transient_Storage_Size Megabytes(1)

#define IconFileName "Capture.png"
#define CurrentDirectory "../project/data"

#define QLIB_SDL
#define QLIB_OPENGL 1
#define QLIB_WINDOW_APPLICATION
#include "qlib/application.h"

#include "sound_visualizer.h"

internal void
MakeAssetFile(platform_work_queue *Queue, assets *Assets)
{
    debug_builder_assets BuilderAssets = {};
    
    //
    // Sounds
    //
    BuilderAddSound(&BuilderAssets, "bonk.wav", Asset_Song);
    
    //
    // Fonts
    //
    BuilderAddFont(&BuilderAssets, "Rubik-Medium.ttf", Asset_Font);
    
    BuilderMakeFile(&BuilderAssets);
}

internal void
LoadAssetFile(platform_work_queue *Queue, assets *Assets)
{
    BuilderLoadFile(Assets);
}

enum menu_component_id
{
    MCI_Play,
    MCI_Pause,
    MCI_Count
};
pair_int_string IDs[] = {
    pairintstring(MCI_Play),
    pairintstring(MCI_Pause),
    pairintstring(MCI_Count)
};

void
UpdateRender(platform *p)
{
    app_state *AppState = MemStart(app_state);
    assets *Assets = &AppState->Assets;
    menu *MainMenu = &AppState->MainMenu;
    platform_work_queue *Queue = &p->Queue;
    camera *C = &AppState->Camera;
    
    if (!AppState->Initialized) {
        AppState->Initialized = true;
        
        InitializeAudioState(&p->AudioState);
        p->AudioState.Assets = (void*)Assets;
        
        MakeAssetFile(&p->Queue, Assets);
        LoadAssetFile(&p->Queue, Assets);
        
        C->Position = v3(0, 0, 900);
        C->Target = v3(0, 0, 0);
        C->Up = v3(0, 1, 0);
        C->FOV = 90.0f;
        C->Mode3D = false;
        
        SetFalse(&p->AudioState.Paused);
    }
    
    C->PlatformDim = v2(p->Dimension.Width, p->Dimension.Height);
    
    playing_sound *PlayingSound = &p->AudioState.PlayingSounds[0];
    loaded_sound *LoadedSound = GetSound(Assets, GetFirstSound(Assets, Asset_Song));
    
    if (DoMenu(MainMenu, "main.menu", p, &AppState->Assets, IDs, MCI_Count))
    {
        if (MainMenu->Events.ButtonClicked == MCI_Play)
        {
            PlaySound(&p->AudioState, GetFirstSound(Assets, Asset_Song));
            SetTrue(&p->AudioState.Paused);
        }
        if (MainMenu->Events.ButtonClicked == MCI_Pause)
        {
            Toggle(&p->AudioState.Paused);
        }
    }
    
    qlibCoordSystem(QLIB_TOP_RIGHT);
    
    v2 WindowDim = GetDim(p);
    v2 Corner = v2(0, 0);
    real32 Width = 2.0f;
    real32 JumpWidth = 0.0f;
    
    if (KeyDown(&p->Input.Mouse.Left)) {
        v2 MouseCoords = v2(p->Input.Mouse.X, p->Input.Mouse.Y);
        if (CoordsInRect(MouseCoords, Corner, v2(WindowDim.x, 50.0f))) {
            
            real32 NewPercentDone = MouseCoords.x / WindowDim.x;
            PlayingSound->SamplesPlayed = (int32)(NewPercentDone * LoadedSound->SampleCount);
            AppState->SaveSample = PlayingSound->SamplesPlayed;
        }
    }
    
    // Progress Bar
    real32 PercentDone = (real32)PlayingSound->SamplesPlayed / (real32)LoadedSound->SampleCount;
    Push(v3(Corner.x, Corner.y, 0), v2(PercentDone * WindowDim.x, 50.f), 0xFFF8CB2E, 0.0f);
    
    // Sound Visual
    if (PlayingSound->Playing) {
        real32 PercentDone = (real32)PlayingSound->SamplesPlayed / (real32)LoadedSound->SampleCount;
        
        for (int i = AppState->SaveSample; i < PlayingSound->SamplesPlayed; i+=1) {
            
            real32 X = (i / WindowDim.x) - (AppState->SaveSample / WindowDim.x);
            real32 Height = (LoadedSound->Samples[0][i]) * (WindowDim.y / 21000);
            
            if (LoadedSound->Samples[0][i] > AppState->BiggestSound)
                AppState->BiggestSound = LoadedSound->Samples[0][i];
            
            if (X + JumpWidth > WindowDim.x) {
                AppState->SaveSample = i;
                JumpWidth = 0;
            }
            
            if (Height > 0)
                Push(v3(Corner.x + X +JumpWidth, WindowDim.y/2, 0), v2(Width, Height), 0xFFB22727, 0.0f);
            else if (Height < 0)
                Push(v3(Corner.x + X +JumpWidth, Height + WindowDim.y/2, 0), v2(Width, -Height), 0xFFB22727, 0.0f);
            JumpWidth += Width;
        }
    }
    
    //DrawFPS(p->Input.MillisecondsElapsed, GetDim(p), GetFont(Assets, GetFirstFont(Assets, Asset_Font)));
    DrawMenu(MainMenu, v2(0, 0), GetDim(p), 100.0f);
    
    RenderPieceGroup(C, Assets);
}