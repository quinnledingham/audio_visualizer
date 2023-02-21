//
// File
//

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

//
// Bitmap
//

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

//
// Sounds
//

function void
sdl_print_audio_spec(SDL_AudioSpec *audio_spec)
{
    SDL_Log("\naudio_spec\n");
    SDL_Log("freq %d\n", audio_spec->freq);
    SDL_Log("format %d\n", SDL_AUDIO_BITSIZE(audio_spec->format));
    SDL_Log("channels %d\n", audio_spec->channels);
    SDL_Log("silence %d\n", audio_spec->silence);
    SDL_Log("samples %d\n", audio_spec->samples);
    SDL_Log("size %d\n", audio_spec->size);
}

function void
sdl_print_audio_device_status(SDL_AudioDeviceID dev)
{
    printf("audio device id: %d, ", dev);
    switch (SDL_GetAudioDeviceStatus(dev))
    {
        case SDL_AUDIO_STOPPED: printf("stopped\n"); break;
        case SDL_AUDIO_PLAYING: printf("playing\n"); break;
        case SDL_AUDIO_PAUSED: printf("paused\n"); break;
        default: printf("???"); break;
    }
}

function SDL_AudioDeviceID
sdl_load_audio_device(SDL_AudioSpec *spec)
{
    SDL_Log("loading audio device\n");
    u32 num_of_audio_devices = SDL_GetNumAudioDevices(0);
    SDL_Log("num audio devices: %d\n", num_of_audio_devices);
    for (u32 i = 0; i < num_of_audio_devices; i++)
        SDL_Log("name: %s\n", SDL_GetAudioDeviceName(i, 0));
    
    SDL_AudioDeviceID device_id = SDL_OpenAudioDevice(NULL, 0, spec, NULL, 0);
    SDL_Log("id: %d\n", device_id);
    sdl_print_audio_spec(spec);
    
    return device_id;
}

// this could be a generic function that you call when you want to
// fill the sound buffer
void
sound_callback(void *userdata, u8 *stream, s32 length_requested)
{
    Playing_Sound *playing_sound = (Playing_Sound*)userdata;
    
    s32 length = length_requested;
    if (length > playing_sound->length_remaining)
        length = playing_sound->length_remaining;
    
    for (int i = 0; i < length; i++)
        stream[i] = playing_sound->position[i];
    
    playing_sound->position += length;
    playing_sound->length_remaining -= length;
    
    printf("%d\n", playing_sound->length_remaining);
    if (playing_sound->length_remaining == 0)
    {
        SDL_CloseAudioDevice(playing_sound->device_id);
    }
}

function Sound
load_sound(const char *filename)
{
    Sound sound = {};
    //SDL_AudioSpec spec = {};
    SDL_LoadWAV(filename, &sound.spec, &sound.buffer, &sound.length);
    //sdl_print_audio_spec(&sound.spec);
    return sound;
}

function void
play_sound(Audio *audio, Sound *sound)
{
    Playing_Sound *playing_sound = &audio->sounds[audio->num_of_playing_sounds++];
    
    playing_sound->position = sound->buffer;
    playing_sound->length_remaining = sound->length;
    sound->spec.callback = sound_callback;
    sound->spec.userdata = playing_sound;
    
    SDL_AudioDeviceID device_id;
    device_id = sdl_load_audio_device(&sound->spec);
    SDL_PauseAudioDevice(device_id, 0);
    playing_sound->device_id = device_id;
}

function void
unpause_sound(Playing_Sound *playing_sound)
{
    SDL_PauseAudioDevice(playing_sound->device_id, 0);
    sdl_print_audio_device_status(playing_sound->device_id);
}

function void
pause_sound(Playing_Sound *playing_sound)
{
    SDL_PauseAudioDevice(playing_sound->device_id, 1);
    sdl_print_audio_device_status(playing_sound->device_id);
}

//
// Font
//

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
draw_string(Font *font, const char *string, v2 coords, f32 pixel_height, v4 color)
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
        draw_rect({x, y}, dim, &font_char->bitmap);
        
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