#ifndef SOUND_VISUALIZER_H
#define SOUND_VISUALIZER_H

struct app_state
{
    menu MainMenu;
    qlib_bool EditMenu;
    camera Camera;
    
    assets Assets;
    
    int SaveSample;
    real32 X;
    
    int BiggestSound;
    
    bool32 Initialized;
};

#endif //SOUND_VISUALIZER_H
