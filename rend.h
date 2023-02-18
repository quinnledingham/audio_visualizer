#ifndef REND_H
#define REND_H

struct Vertex
{
    v3 position;
    v3 normal;
    v2 texture_coordinate;
};

struct Mesh
{
    Vertex *vertices;
    u32 vertices_count;
    
    u32 *indices;
    u32 indices_count;
    
    u32 vao;
    u32 vbo;
    u32 ebo;
};

global_variable u32 shader;
global_variable u32 tex_shader;
global_variable m4x4 orthographic_matrix;

#endif //REND_H
