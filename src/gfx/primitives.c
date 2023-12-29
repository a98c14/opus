#include "primitives.h"

internal GeometryIndex
geometry_quad_create(Renderer* renderer)
{
    uint32 vao;
    uint32 vbo;
    uint32 ebo;

    // generate object names
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);

    // bind buffers
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(shape_vertices_quad), shape_vertices_quad, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(shape_indices_quad), shape_indices_quad, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float32), (void*)0);
    glEnableVertexAttribArray(0);

    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float32), (void*)(3 * sizeof(float32)));
    glEnableVertexAttribArray(1);

    // texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float32), (void*)(6 * sizeof(float32)));
    glEnableVertexAttribArray(2);

    GeometryIndex result = geometry_new(renderer, 6, vao);
    return result;
}

internal GeometryIndex
geometry_triangle_create(Renderer* renderer)
{
    uint32 vao;
    uint32 ebo;
    uint32 vbo;

    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(shape_vertices_triangle), shape_vertices_triangle, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(shape_indices_triangle), shape_indices_triangle, GL_STATIC_DRAW);

    // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float32), (void*)0);
    glEnableVertexAttribArray(0);
    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float32), (void*)(sizeof(float32) * 3));
    glEnableVertexAttribArray(1);
    // texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float32), (void*)(6 * sizeof(float32)));
    glEnableVertexAttribArray(2);

    GeometryIndex result = geometry_new(renderer, 3, vao);
    return result;
}