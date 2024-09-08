// clang-format off
#pragma once
#include "base.h"

global read_only const float32 shape_vertices_quad[] = {
    // positions   // texture coords  // colors          
     0.5f,  0.5f,  1.0f, 1.0f,        1.0f, 0.0f, 0.0f, 1.0f,  // top right
     0.5f, -0.5f,  1.0f, 0.0f,        0.0f, 1.0f, 0.0f, 1.0f,  // bottom right
    -0.5f, -0.5f,  0.0f, 0.0f,        0.0f, 0.0f, 1.0f, 1.0f,  // bottom left
    -0.5f,  0.5f,  0.0f, 1.0f,        0.0f, 0.0f, 1.0f, 1.0f,  // top left
};

global read_only const uint32 shape_indices_quad[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

global read_only const float32 shape_vertices_triangle[] = {
    // positions,  // texture coords  // colors       
    -0.5f, -0.5f,  0.0f, 0.0f,        1.f, 0.f, 0.f, 1.0f,   
     0.5f, -0.5f,  1.0f, 0.0f,        0.f, 1.f, 0.f, 1.0f,   
     0.0f,  0.5f,  0.5f, 1.0f,        0.f, 0.f, 1.f, 1.0f} ;

global read_only const uint32 shape_indices_triangle[] = {0, 1, 2};

internal uint32 mesh_quad_create(Renderer* renderer);
internal uint32 mesh_triangle_create(Renderer* renderer);
internal uint32 mesh_buffer_create(Renderer* renderer, VertexAttributeInfo* attributes, uint32 vertex_count);
