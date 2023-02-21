void GLAPIENTRY
opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                              GLsizei length, const GLchar* message, const void* userParam )
{
    SDL_Log("GL CALLBACK:");
    SDL_Log("message: %s\n", message);
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR: SDL_Log("type: ERROR"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: SDL_Log("type: DEPRECATED_BEHAVIOR"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: SDL_Log("type: UNDEFINED_BEHAVIOR"); break;
        case GL_DEBUG_TYPE_PORTABILITY: SDL_Log("type: PORTABILITY"); break;
        case GL_DEBUG_TYPE_PERFORMANCE: SDL_Log("type: PERFORMANCE"); break;
        case GL_DEBUG_TYPE_OTHER: SDL_Log("type: OTHER"); break;
    }
    SDL_Log("id: %d", id);
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_LOW: SDL_Log("severity: LOW\n"); break;
        case GL_DEBUG_SEVERITY_MEDIUM: SDL_Log("severity: MEDIUM\n"); break;
        case GL_DEBUG_SEVERITY_HIGH: SDL_Log("severity: HIGH\n"); break;
    }
}

function void
debug_opengl(u32 type, u32 id)
{
    GLint length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    if (length > 0)
    {
        GLchar info_log[512];
        GLint size;
        
        switch(type)
        {
            case GL_SHADER: glGetShaderInfoLog(id, 512, &size, info_log); break;
            case GL_PROGRAM: glGetProgramInfoLog(id, 512, &size, info_log); break;
        }
        
        log(info_log);
    }
}

function const char*
load_shader_file(const char* filename)
{
    FILE *file = fopen(filename, "rb");
    if (file == 0)
    {
        error("load_shader_file() could not open file");
        return 0;
    }
    
    fseek(file, 0, SEEK_END);
    u32 size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* shader_file = (char*)malloc(size + 1);
    fread(shader_file, size, 1, file);
    shader_file[size] = 0;
    fclose(file);
    
    return shader_file;
}

function bool
compile_opengl_shader(u32 handle, const char *filename, int type)
{
    const char *file = load_shader_file(filename);
    if (file == 0)
        return false;
    
    u32 s =  glCreateShader((GLenum)type);
    glShaderSource(s, 1, &file, NULL);
    glCompileShader(s);
    
    GLint compiled_s = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &compiled_s);
    if (!compiled_s)
        debug_opengl(GL_SHADER, s);
    else
        glAttachShader(handle, s);
    
    glDeleteShader(s);
    free((void*)file);
    
    return compiled_s;
}

function u32
load_opengl_shader(const char *vs_filename, const char *fs_filename)
{
    u32 handle = glCreateProgram();
    
    if (vs_filename == 0)
    {
        error("load_opengl_shader() No vertex filename");
        return 0;
    }
    if (fs_filename == 0)
    {
        error("load_opengl_shader() No fragment filename");
        return 0;
    }
    
    if (!compile_opengl_shader(handle, vs_filename, GL_VERTEX_SHADER))
    {
        error("load_opengl_shader() compiling vertex shader failed");
        return 0;
    }
    
    if (!compile_opengl_shader(handle, fs_filename, GL_FRAGMENT_SHADER))
    {
        error("load_opengl_shader() compiling fragment shader failed");
        return 0;
    }
    
    glLinkProgram(handle);
    GLint linked_program = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &linked_program);
    if (!linked_program)
    {
        debug_opengl(GL_PROGRAM, handle);
        error("load_opengl_shader() link failed");
        return 0;
    }
    
    return handle;
}

function void
opengl_setup_mesh(Mesh *mesh)
{
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);
    
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices_count * sizeof(Vertex), &mesh->vertices[0], GL_STATIC_DRAW);  
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_count * sizeof(u32), &mesh->indices[0], GL_STATIC_DRAW);
    
    // vertex positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture_coordinate));
    
    glBindVertexArray(0);
}

function void
opengl_draw_mesh(Mesh *mesh)
{
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->indices_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

function void
init_rect_indices(u32 *indices, 
                  u32 top_left, 
                  u32 top_right,
                  u32 bottom_left,
                  u32 bottom_right)
{
    indices[0] = top_left;
    indices[1] = bottom_left;
    indices[2] = bottom_right;
    indices[3] = top_left;
    indices[4] = bottom_right;
    indices[5] = top_right;
}

function void
init_rect_mesh(Mesh *rect)
{
    rect->vertices_count = 4;
    rect->vertices = (Vertex*)SDL_malloc(sizeof(Vertex) * rect->vertices_count);
    rect->vertices[0] = { {0, 0, 0}, {0, 0, 1}, {0, 0} };
    rect->vertices[1] = { {0, 1, 0}, {0, 0, 1}, {0, 1} };
    rect->vertices[2] = { {1, 0, 0}, {0, 0, 1}, {1, 0} };
    rect->vertices[3] = { {1, 1, 0}, {0, 0, 1}, {1, 1} };
    
    rect->indices_count = 6;
    rect->indices = (u32*)SDL_malloc(sizeof(u32) * rect->indices_count);
    init_rect_indices(rect->indices, 1, 3, 0, 2);
    
    opengl_setup_mesh(rect);
}

function void
draw_rect(v2 coords, v2 dim, v4 color)
{
    local_persist Mesh rect = {};
    if (rect.vertices_count == 0)
        init_rect_mesh(&rect);
    
    glUseProgram(shader);
    glUniform4fv(glGetUniformLocation(shader, "user_color"), (GLsizei)1, (float*)&color);
    m4x4 model = create_transform_m4x4({coords.x, coords.y, 0}, get_rotation(0, {1, 0, 0}), {dim.x, dim.y, 1.0f});
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), (GLsizei)1, false, (float*)&model);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), (GLsizei)1, false, (float*)&orthographic_matrix);
    opengl_draw_mesh(&rect);
}

function void
draw_rect(v2 coords, v2 dim, Bitmap *bitmap)
{
    local_persist Mesh rect = {};
    if (rect.vertices_count == 0)
        init_rect_mesh(&rect);
    
    glUseProgram(tex_shader);
    m4x4 model = create_transform_m4x4({coords.x, coords.y, 0}, get_rotation(0, {1, 0, 0}), {dim.x, dim.y, 1.0f});
    glUniformMatrix4fv(glGetUniformLocation(tex_shader, "model"), (GLsizei)1, false, (float*)&model);
    glUniformMatrix4fv(glGetUniformLocation(tex_shader, "projection"), (GLsizei)1, false, (float*)&orthographic_matrix);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bitmap->handle);
    glUniform1i(glGetUniformLocation(tex_shader, "tex0"), 0);
    
    opengl_draw_mesh(&rect);
}