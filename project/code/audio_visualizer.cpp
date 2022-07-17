#define WindowName "Audio Visualizer"
#define ClientHeight (ScreenHeight - 200)
#define ClientWidth ClientHeight

#define Permanent_Storage_Size Megabytes(126)
#define Transient_Storage_Size Megabytes(1)

#define IconFileName "icon.ico"
#define CurrentDirectory "../project/data"

enum app_asset_sound_id
{
    AASI_BornInTheUSA,
    SOUND_COUNT
};
enum app_asset_font_id
{
    Rubik,
    FONT_COUNT
};
#define SOUND_COUNT SOUND_COUNT
#define BITMAP_COUNT 1
#define FONT_COUNT FONT_COUNT

#define QLIB_WINDOW_APPLICATION
#include "qlib/application.h"

#include "sound_visualizer.h"

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
    app_state *AppState = (app_state*)p->Memory.PermanentStorage;
    assets *Assets = &AppState->Assets;
    menu *MainMenu = &AppState->MainMenu;
    platform_work_queue *Queue = &p->Queue;
    camera *C = &AppState->Camera;
    
    if (!AppState->Initialized) {
        Manager.Next = (char*)p->Memory.PermanentStorage;
        qalloc(sizeof(app_state));
        
        asset *Sounds = Assets->Sounds;
        AssetInitTag(Sounds, "bornintheusa.wav", AASI_BornInTheUSA);
        for (int i = 0; i < sound_count; i++)
            WorkQueueAddEntry(Queue, LoadSoundAsset, &Sounds[i]);
        
        asset *Fonts = Assets->Fonts;
        AssetInitTag(Fonts, "Rubik-Medium.ttf", Rubik);
        for (int i = 0; i < font_count; i++)
            WorkQueueAddEntry(Queue, LoadFontAsset, &Fonts[i]);
        
        WorkQueueCompleteAllWork(Queue);
        
        InitializeAudioState(&p->AudioState);
        
        *C = {};
        C->Position = v3(0, 0, -900);
        C->Target = v3(0, 0, 0);
        C->Up = v3(0, 1, 0);
        C->FOV = 90.0f;
        
        AppState->Initialized = true;
    }
    
    C->WindowDim = v2(p->Dimension.Width, p->Dimension.Height);
    playing_sound *PlayingSound = &p->AudioState.PlayingSounds[0];
    
    if (DoMenu(MainMenu, "main.menu", p, &AppState->Assets, &AppState->EditMenu, IDs, MCI_Count))
    {
        if (MainMenu->Events.ButtonClicked == MCI_Play)
        {
            SetTrue(&p->AudioState.Paused);
            if (PlayingSound->CurrentVolume.x == 0)
                PlaySound(&p->AudioState, GetSound(Assets, AASI_BornInTheUSA));
        }
        if (MainMenu->Events.ButtonClicked == MCI_Pause)
        {
            Toggle(&p->AudioState.Paused);
        }
    }
    
    v2 WindowDim = GetDim(p);
    v2 Corner = GetTopLeftCornerCoords(p);
    real32 Width = 2.0f;
    real32 JumpWidth = 0.0f;
    
    if (PlayingSound->CurrentVolume.x != 0) {
        real32 PercentDone = (real32)PlayingSound->SamplesPlayed / (real32)PlayingSound->LoadedSound->SampleCount;
        
        for (int i = AppState->SaveSample; i < PlayingSound->SamplesPlayed; i+=1) {
            
            real32 X = (i / WindowDim.x) - (AppState->SaveSample / WindowDim.x);
            real32 Height = (PlayingSound->LoadedSound->Samples[0][i]) * (WindowDim.y / 21000);
            
            if (PlayingSound->LoadedSound->Samples[0][i] > AppState->BiggestSound)
                AppState->BiggestSound = PlayingSound->LoadedSound->Samples[0][i];
            
            //fprintf(stderr, "%d\n", PlayingSound->LoadedSound->Samples[0][i]);
            
            if (X + JumpWidth > WindowDim.x) {
                AppState->SaveSample = i;
                JumpWidth = 0;
            }
            
            if (Height > 0)
                Push(v3(Corner.x + X +JumpWidth, 0, 0), v2(Width, Height), 0xFFB22727, 0.0f);
            else if (Height < 0)
                Push(v3(Corner.x + X +JumpWidth, Height, 0), v2(Width, -Height), 0xFFB22727, 0.0f);
            JumpWidth += Width;
        }
        
        Push(v3(Corner.x, Corner.y, 0), v2(PercentDone * WindowDim.x, 50.f), 0xFFF8CB2E, 0.0f);
    }
    
    DrawFPS(p->Input.WorkSecondsElapsed, GetDim(p), GetFont(Assets, Rubik));
    DrawMenu(MainMenu, GetTopLeftCornerCoords(p), GetDim(p), 100.0f);
    
    BeginMode2D(*C);
    RenderPieceGroup();
}