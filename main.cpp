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
#include "input.h"

#include "tools.cpp"
#include "rend.cpp"
#include "assets.cpp"

/*
 List of options to set:
coords
 size
color
active_color

text
font
text size
text color
text active_color

Input:
*/

struct Menu_Button
{
    v2 coords;
    v2 dim;
    v4 back_color;
    v4 active_color;
    
    const char *text;
    Font *font;
    f32 text_pixel_height;
    v4 text_color;
    v4 active_text_color;
};

function b32
coords_in_bounds(v2 coords, v2 top_left, v2 bottom_right)
{
    if (coords.x > top_left.x && 
        coords.x < bottom_right.x &&
        coords.y > top_left.y && 
        coords.y < bottom_right.y)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// returns true if the button is pressed
function b32
menu_button(v2 coords, v2 dim, v4 back_color, v4 active_back_color,
            const char *text, Font *font, f32 pixel_height, v4 text_color, v4 active_text_color,
            v2 mouse_coords, b32 press)
{
    b32 button_pressed = false;
    
    v4 b_color = back_color;
    v4 t_color = text_color;
    
    if (coords_in_bounds(mouse_coords, coords, coords + dim))
    {
        b_color = active_back_color;
        t_color = active_text_color;
        
        if (press)
            button_pressed = true;
    }
    
    // drawing
    draw_rect(coords, dim, b_color);
    
    v2 text_dim = get_string_dim(font, text, pixel_height, t_color);
    f32 text_x_coord = coords.x + (dim.x / 2.0f) - (text_dim.x / 2.0f);
    f32 text_y_coord = coords.y + (dim.y / 2.0f) + (text_dim.y / 2.0f);
    
    draw_string(font, text, { text_x_coord, text_y_coord }, pixel_height, t_color);
    
    return button_pressed;
}

struct Assets
{
    u32 *shaders;
    u32 num_of_shaders;
    
    Sound *sounds;
    u32 num_of_sounds;
    
    Bitmap bitmap;
    Font font;
};

struct Controller
{
    b32 mouse_enabled;
    v2 mouse_coords;
    v2 mouse_rel_coords;
    
    
    union
    {
        struct
        {
            Button left_mouse;
            
            Button up;
        };
        Button buttons[2];
    };
};

struct Application
{
    v2s window_dim;
    Assets assets;
    Controller controller;
    Audio audio;
    
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
    
    assets->num_of_sounds = 1;
    assets->sounds = (Sound*)malloc(sizeof(Sound) * assets->num_of_sounds);
    assets->sounds[0] = load_sound("../assets/gulp.wav");
}

function void
do_one_frame(Application *app)
{
    
    // DRAW
    orthographic_matrix = orthographic_projection(0.0f,
                                                  (r32)app->window_dim.Width, (r32)app->window_dim.Height,
                                                  0.0f, -3.0f, 3.0f);
    
    
    Font *font = &app->assets.font;
    Controller *controller = &app->controller;\
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    draw_rect({0, 0}, {100, 100}, &app->assets.bitmap);
    
    // Menu
    v2 menu_coords = { 100, 100 };
    v2 active_coords = {};
    v2 button_dim = { 130, 50 };
    b32 press = false;
    
    if (controller->mouse_enabled)
    {
        active_coords = controller->mouse_coords;
        press = on_down(controller->left_mouse);
    }
    
    if (menu_button(menu_coords, button_dim, {0, 255, 0, 1}, {0, 150, 0, 1},
                    "Play", font, 40, {0, 0, 255, 1}, {0, 0, 150, 1},
                    active_coords, press))
    {
        log("play");
        play_sound(&app->audio, &app->assets.sounds[0]);
    }
    
    menu_coords.x += button_dim.x + 5;
    
    if (menu_button(menu_coords, button_dim, {0, 255, 0, 1}, {0, 150, 0, 1},
                    "Pause", font, 40, {0, 0, 255, 1}, {0, 0, 150, 1},
                    active_coords, press))
    {
        log("pause");
    }
    
    // FPS
    f32 fps = 1000.0f;
    if (app->frame_time_s != 0)
        fps = 1.0f / app->frame_time_s;
    char *fps_string = u32_to_string((u32)fps);
    
    v2 dim = get_string_dim(font, fps_string, 50, {1.0f, 0.0f, 0.0f, 1.0f});
    draw_string(font, fps_string, {app->window_dim.x - dim.x, dim.y}, 50, {1.0f, 0.0f, 0.0f, 1.0f});
}

function void
main_loop(SDL_Window *window)
{
    Application app = {};
    
    Controller *controller = &app.controller;
    controller->left_mouse.id = SDL_BUTTON_LEFT;
    
    SDL_GetWindowSize(window, &app.window_dim.Width, &app.window_dim.Height);
    load_assets(&app.assets);
    u32 last_frame_run_time_ms = 0;
    
    while(1)
    {
        controller->mouse_rel_coords = {};
        for (int i = 0; i < array_count(controller->buttons); i++)
            controller->buttons[i].previous_state =controller->buttons[i].current_state;
        
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT: return;
                
                case SDL_WINDOWEVENT_RESIZED:
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    SDL_WindowEvent *window_event = &event.window;
                    
                    app.window_dim.Width = window_event->data1;
                    app.window_dim.Height = window_event->data2;
                    glViewport(0, 0, app.window_dim.Width, app.window_dim.Height);
                } break;
                
                case SDL_MOUSEMOTION:
                {
                    controller->mouse_enabled = true;
                    controller->mouse_coords.x = event.motion.x;
                    controller->mouse_coords.y = event.motion.y;
                    controller->mouse_rel_coords.x = event.motion.xrel;
                    controller->mouse_rel_coords.y = event.motion.yrel;
                } break;
                
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEBUTTONDOWN:
                {
                    SDL_MouseButtonEvent *mouse_button_event = &event.button;
                    s32 button_id = mouse_button_event->button;
                    s32 button_state = mouse_button_event->state;
                    
                    b32 state = false;
                    if (button_state == SDL_PRESSED)
                        state = true;
                    
                    if (controller->left_mouse.id == button_id)
                        controller->left_mouse.current_state = state;
                    
                } break;
            }
        }
        
        // getting run time and frame time
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
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
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