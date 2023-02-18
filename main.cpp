#include <glad.h>
#include <glad.c>
#include <SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include "types.h"
#include "log.h"
#include "math.h"
#include "rend.h"

function b32
equal(const char *l, const char *r)
{
    u32 i = 0;
    while (l[i] != 0 && r[i] != 0)
    {
        if (l[i] != r[i])
            return false;
        i++;
    }
    
    if (r[i] != 0)
        return false;
    
    return true;
}

function u32
get_length(const char *in)
{
    int count = 0;
    while(in[count] != 0)
        count++;
    return count;
}

struct File
{
    u32 size;
    void *memory;
};

function File
read_file(const char *filename)
{
    File result = {};
    
    FILE *in = fopen(filename, "rb");
    if(in) 
    {
        fseek(in, 0, SEEK_END);
        result.size = ftell(in);
        fseek(in, 0, SEEK_SET);
        
        result.memory = malloc(result.size);
        fread(result.memory, result.size, 1, in);
        fclose(in);
    }
    else 
        SDL_Log("ERROR: Cannot open file %s.\n", filename);
    
    return result;
}

struct Bitmap
{
    u32 handle;
    u8 *memory;
    s32 width;
    s32 height;
    s32 pitch;
    s32 channels;
};

#include "rend.cpp"

function Bitmap
load_bitmap(const char *filename)
{
    Bitmap bitmap = {};
    bitmap.memory = stbi_load(filename, &bitmap.width, &bitmap.height, &bitmap.channels, 0);
    if (bitmap.memory == 0)
        error("load_bitmap() could not load bitmap");
    return bitmap;
}

function void
init_bitmap_handle(Bitmap *bitmap)
{
    glGenTextures(1, &bitmap->handle);
    glBindTexture(GL_TEXTURE_2D, bitmap->handle);
    
    if (bitmap->channels == 3) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bitmap->width, bitmap->height, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap->memory);
    }
    else if (bitmap->channels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap->width, bitmap->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap->memory);
    
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Tile
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

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

function void
log_font_chars(Font *font)
{
    printf("start log\n");
    for (u32 i = 0; i < font->font_chars_cached; i++)
    {
        Font_Char *fc = &font->font_chars[i];
        printf("%d, %f, %f, %f, %f, %f, %d, %d\n", fc->codepoint, fc->scale, fc->color.x, fc->color.y,
               fc->color.z, fc->color.w, fc->ax, fc->lsb);
    }
    printf("end log\n");
}

function Font_Char*
load_font_char(Font *font, u32 codepoint, f32 scale, v4 color)
{
    // search cache for font char
    for (u32 i = 0; i < font->font_chars_cached; i++)
    {
        Font_Char *font_char = &font->font_chars[i];
        //printf("wow: %d\n", codepoint);
        if (font_char->codepoint == codepoint && font_char->color == color && font_char->scale == scale)
            return font_char;
    }
    
    // where to cache new font char
    Font_Char *font_char = &font->font_chars[font->font_chars_cached];
    memset(font_char, 0, sizeof(Font_Char));
    if (font->font_chars_cached + 1 < array_count(font->font_chars))
        font->font_chars_cached++;
    else
        font->font_chars_cached = 0;
    
    font_char->codepoint = codepoint;
    font_char->scale = scale;
    font_char->color = color;
    
    
    // how wide is this character
    stbtt_GetCodepointHMetrics(&font->info, font_char->codepoint, &font_char->ax, &font_char->lsb);
    
    // get bounding box for character (may be offset to account for chars that dip above or below the line
    stbtt_GetCodepointBitmapBox(&font->info, font_char->codepoint, font_char->scale, font_char->scale, 
                                &font_char->c_x1, &font_char->c_y1, &font_char->c_x2, &font_char->c_y2);
    
    u8 *mono_bitmap = stbtt_GetCodepointBitmap(&font->info, 0, scale, codepoint, 
                                               &font_char->bitmap.width, &font_char->bitmap.height,
                                               0, 0);
    font_char->bitmap.channels = 4;
    font_char->bitmap.memory = (u8*)SDL_malloc(font_char->bitmap.width * 
                                               font_char->bitmap.height * 
                                               font_char->bitmap.channels);
    u32 *dest = (u32*)font_char->bitmap.memory;
    
    for (s32 x = 0; x < font_char->bitmap.width; x++)
    {
        for (s32 y = 0; y < font_char->bitmap.height; y++)
        {
            u8 alpha = *mono_bitmap++;
            *dest++ = ((alpha << 24) | (alpha << 16) | (alpha <<  8) | (alpha <<  0));
        }
    }
    
    init_bitmap_handle(&font_char->bitmap);
    
    return font_char;
}

function Font
load_font(const char *filename)
{
    Font font = {};
    SDL_memset(font.font_scales, 0, sizeof(Font_Scale) * array_count(font.font_scales));
    SDL_memset(font.font_chars, 0, sizeof(Font_Char) * array_count(font.font_chars));
    SDL_memset(font.font_strings, 0, sizeof(Font_String) * array_count(font.font_strings));
    File file = read_file(filename);
    stbtt_InitFont(&font.info, (u8*)file.memory, stbtt_GetFontOffsetForIndex((u8*)file.memory, 0));
    return font;
}

function void
draw_string(Font *font, const char *string, v3 coords, f32 pixel_height, v4 color)
{
    f32 scale = stbtt_ScaleForPixelHeight(&font->info, pixel_height);
    
    f32 string_x_coord = coords.x;
    u32 i = 0;
    while (string[i] != 0)
    {
        Font_Char *font_char = load_font_char(font, string[i], scale, color);
        
        f32 y = coords.y + font_char->c_y1;
        f32 x = string_x_coord + (font_char->lsb * scale);
        
        v2 dim = { f32(font_char->c_x2 - font_char->c_x1), f32(font_char->c_y2 - font_char->c_y1) };
        draw_rect({x, y, 0}, dim, &font_char->bitmap);
        
        //printf("char: %c\n", font_char->codepoint);
        int kern = stbtt_GetCodepointKernAdvance(&font->info, string[i], string[i + 1]);
        string_x_coord += ((kern + font_char->ax) * scale);
        
        i++;
    }
}

function v2
get_string_dim(Font *font, const char *in, f32 pixel_height, v4 color)
{
    Font_String *string = 0;
    for (u32 i = 0; i < font->strings_cached; i++)
    {
        string = &font->font_strings[i];
        if (equal(string->memory, in) && string->pixel_height == pixel_height)
            return string->dim;
    }
    
    string = &font->font_strings[font->strings_cached];
    if (font->strings_cached + 1 < array_count(font->font_strings))
        font->strings_cached++;
    else
        font->strings_cached = 0;
    
    f32 scale = stbtt_ScaleForPixelHeight(&font->info, pixel_height);
    string->memory = (char*)SDL_malloc(get_length(in) + 1);
    SDL_memset(string->memory, 0, get_length(in) + 1);
    
    f32 string_x_coord = 0.0f;
    f32 string_height = 0.0f;
    
    int i = 0;
    while (in[i] != 0)
    {
        string->memory[i] = in[i];
        Font_Char *font_char = load_font_char(font, string->memory[i], scale, color);
        
        f32 y = -1 * font_char->c_y1;
        f32 x = string_x_coord + (font_char->lsb * scale);
        
        if (y > string_height)
            string_height = y;
        
        int kern = stbtt_GetCodepointKernAdvance(&font->info, string->memory[i], string->memory[i + 1]);
        string_x_coord += ((kern + font_char->ax) * scale);
        
        i++;
    }
    string->memory[i] = 0;
    
    string->pixel_height = pixel_height;
    string->dim = { string_x_coord, string_height };
    
    return string->dim;
}

struct Assets
{
    u32 *shaders;
    u32 num_of_shaders;
    
    SDL_AudioSpec *sounds;
    u32 num_of_sounds;
    
    Bitmap bitmap;
    Font font;
};

struct Controller
{
    v2 mouse_coords;
};

struct Application
{
    v2s window_dim;
    Assets assets;
    Controller controller;
    
    f32 run_time_s;
    f32 frame_time_s;
};

function u32
get_length(u32 i)
{
    u32 count = 0;
    u32 t = i;
    while ((t = t / 10) != 0)
        count++;
    count++;
    return count;
}

function char*
u32_to_string(u32 in)
{
    char *out;
    
    u32 digits = get_length(in);
    out = (char*)SDL_malloc(digits + 1);
    
    u32 help = 0;
    for (u32 i = 0; i < digits; i++)
    {
        if (help == 0)
            help++;
        else
            help *= 10;
    }
    
    for (u32 i = 0; i < digits; i++)
    {
        out[i] = ((char)(in / help) + '0');
        in = in - ((in / help) * help);
        help = help / 10;
    }
    
    out[digits] = 0;
    return out;
}

function void
load_assets(Assets *assets)
{
    assets->num_of_shaders = 1;
    //assets->shaders = (u32*)malloc(sizeof(u32) * assets->num_of_sounds);
    //assets->shaders[0] = load_opengl_shader("../assets/shaders/color2D.vs", "../assets/shaders/color.fs");
    shader = load_opengl_shader("../assets/shaders/color2D.vs", "../assets/shaders/color.fs");
    tex_shader = load_opengl_shader("../assets/shaders/color2D.vs", "../assets/shaders/tex.fs");
    
    assets->bitmap = load_bitmap("../assets/tom.png");
    init_bitmap_handle(&assets->bitmap);
    
    assets->font = load_font("../assets/Rubik-Medium.ttf");
}

function void
do_one_frame(Application *app)
{
    orthographic_matrix = orthographic_projection(0.0f,
                                                  (r32)app->window_dim.Width, (r32)app->window_dim.Height,
                                                  0.0f, -3.0f, 3.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    
    draw_rect({0, 0, 0}, {100, 100}, &app->assets.bitmap);
    //draw_rect({0, 0, 0}, {100, 100}, {255, 255, 0, 1});
    draw_string(&app->assets.font, "Hello", {400, 400, 0}, 100, {1.0f, 0.0f, 0.0f, 1.0f});
    
    
    f32 fps = 1000.0f;
    if (app->frame_time_s != 0)
    {
        fps = 1.0f / app->frame_time_s;
    }
    char *fps_string = u32_to_string((u32)fps);
    
    //log_font_chars(&app->assets.font);
    
    v2 dim = get_string_dim(&app->assets.font, fps_string, 50, {1.0f, 0.0f, 0.0f, 1.0f});
    //v2 dim = {};
    //printf("%f\n", dim.y);
    draw_string(&app->assets.font, fps_string, {app->window_dim.x - dim.x, dim.y, 0}, 50, {1.0f, 0.0f, 0.0f, 1.0f});
}

function void
main_loop(SDL_Window *window)
{
    Application app = {};
    SDL_GetWindowSize(window, &app.window_dim.Width, &app.window_dim.Height);
    load_assets(&app.assets);
    u32 last_frame_run_time_ms = 0;
    
    while(1)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT: return;
                case SDL_WINDOWEVENT:
                {
                    switch(event.window.event)
                    {
                        case SDL_WINDOWEVENT_RESIZED:
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                        {
                            app.window_dim.Width = event.window.data1;
                            app.window_dim.Height = event.window.data2;
                            glViewport(0, 0, app.window_dim.Width, app.window_dim.Height);
                        } break;
                    }
                } break;
            }
        }
        
        u32 run_time_ms = SDL_GetTicks();
        app.run_time_s = (f32)run_time_ms / 1000.0f;
        u32 frame_time_ms = run_time_ms - last_frame_run_time_ms;
        app.frame_time_s = (f32)frame_time_ms / 1000.0f;
        last_frame_run_time_ms = run_time_ms;
        
        do_one_frame(&app);
        
        SDL_GL_SwapWindow(window);
    }
}

function void
sdl_init_opengl(SDL_Window *window)
{
    SDL_GL_LoadLibrary(NULL);
    
    // Request an OpenGL 4.6 context (should be core)
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    
    // Also request a depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    SDL_GLContext Context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(0);
    
    // Check OpenGL properties
    gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);
    SDL_Log("OpenGL loaded\n");
    SDL_Log("Vendor:   %s", glGetString(GL_VENDOR));
    SDL_Log("Renderer: %s", glGetString(GL_RENDERER));
    SDL_Log("Version:  %s", glGetString(GL_VERSION));
    
    // Default settings
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glPointSize(5.0f);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(opengl_debug_message_callback, 0);
    
    v2s window_dim = {};
    SDL_GetWindowSize(window, &window_dim.Width, &window_dim.Height);
    glViewport(0, 0, window_dim.Width, window_dim.Height);
}

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO | 
             SDL_INIT_GAMECONTROLLER | 
             SDL_INIT_HAPTIC | 
             SDL_INIT_AUDIO);
    
    SDL_Window *window = SDL_CreateWindow("Audio Visualizer", 
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                          800, 800, 
                                          SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    sdl_init_opengl(window);
    main_loop(window);
    
    return 0;
}