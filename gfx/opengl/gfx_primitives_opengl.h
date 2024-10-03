#pragma once
#include <glad/gl.h>
#include "../gfx_core.h"

global read_only const float32 shape_vertices_quad[] = {
    // positions   // texture coords  // colors
    0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,   // top right
    0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // bottom right
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // bottom left
    -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top left
};

global read_only const uint32 shape_indices_quad[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

global read_only const float32 shape_vertices_triangle[] = {
    // positions, // texture coords, // colors
    -0.5f, -0.5f, 0.0f, 0.0f, 1.f, 0.f, 0.f, 1.0f,
    0.5f, -0.5f, 1.0f, 0.0f, 0.f, 1.f, 0.f, 1.0f,
    0.0f, 0.5f, 0.5f, 1.0f, 0.f, 0.f, 1.f, 1.0f};

global read_only const uint32 shape_indices_triangle[] = {0, 1, 2};

internal uint32 _gfx_ogl_mesh_quad_create();
internal uint32 _gfx_ogl_mesh_triangle_create();
internal uint32 _gfx_ogl_mesh_buffer_create(GFX_VertexAttributeInfo* attributes);
