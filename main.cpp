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
#include "assets.h"
#include "rend.h"

#include "tools.cpp"
#include "rend.cpp"
#include "assets.cpp"

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