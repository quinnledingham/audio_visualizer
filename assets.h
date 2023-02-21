#ifndef ASSETS_H
#define ASSETS_H

struct File
{
    u32 size;
    void *memory;
};

struct Bitmap
{
    u32 handle;
    u8 *memory;
    s32 width;
    s32 height;
    s32 pitch;
    s32 channels;
};

struct Sound
{
    SDL_AudioSpec spec;
    u8 *buffer;
    u32 length;
};

struct Playing_Sound
{
    u8 *position;
    u32 length_remaining;
    SDL_AudioDeviceID device_id;
};

struct Audio
{
    b32 playing;
    
    Playing_Sound sounds[10];
    u32 num_of_playing_sounds;
};

//
// Font
//

struct Font_Scale
{
    f32 pixel_height;
    
    f32 scale;
    s32 ascent;
    s32 descent;
    s32 line_gap;
    f32 scaled_ascent;
    f32 scaled_descent;
};

struct Font_Char
{
    u32 codepoint;
    f32 scale;
    v4 color;
    
    s32 ax;
    s32 lsb;
    s32 c_x1;
    s32 c_y1;
    s32 c_x2;
    s32 c_y2;
    
    Bitmap bitmap;
};

struct Font_String
{
    char *memory;
    v2 dim;
    f32 pixel_height;
};

struct Font
{
    stbtt_fontinfo info;
    
    s32 font_scales_cached;
    s32 font_chars_cached;
    s32 strings_cached;
    Font_Scale font_scales[10];
    Font_Char font_chars[300];
    Font_String font_strings[10];
};

//
// End of Font
//

#endif //ASSETS_H
