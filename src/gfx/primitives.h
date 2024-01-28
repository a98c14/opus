#pragma once
#include "base.h"
#include <base/math.h>


const float32 shape_vertices_quad[] =
    {
        // positions          // colors         // texture coords
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f   // top left
};

const uint32 shape_indices_quad[] =
    {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
};

// internal const float32 shape_vertices_triangle[] =
// {
//     // positions,          // colors       // texture coords
//     -0.5f, -0.288f, 0.0f,  1.f, 0.f, 0.f,    0.0f, 0.0f,
//      0.5f, -0.288f, 0.0f,  0.f, 1.f, 0.f,    1.0f, 0.0f,
//      0.0f,  0.587f, 0.0f,  0.f, 0.f, 1.f,    0.5f, 0.875f
// };

internal const float32 shape_vertices_triangle[] =
    {
        // positions,        // colors       // texture coords
        -0.5f, -0.5f, 0.0f, 1.f, 0.f, 0.f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.f, 1.f, 0.f, 1.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.f, 0.f, 1.f, 0.5f, 1.0f};

const uint32 shape_indices_triangle[] =
    {
        0, 1, 2};

internal GeometryIndex geometry_quad_create(Renderer* renderer);

internal GeometryIndex geometry_triangle_create(Renderer* renderer);
