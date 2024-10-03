#include "gfx_primitives_opengl.h"

internal uint32
_gfx_ogl_mesh_quad_create()
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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float32), (void*)0);
    glEnableVertexAttribArray(0);

    // texcoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float32), (void*)(2 * sizeof(float32)));
    glEnableVertexAttribArray(1);

    // color
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float32), (void*)(4 * sizeof(float32)));
    glEnableVertexAttribArray(2);

    return vao;
}

internal uint32
_gfx_ogl_mesh_triangle_create()
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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float32), (void*)0);
    glEnableVertexAttribArray(0);

    // texcoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float32), (void*)(2 * sizeof(float32)));
    glEnableVertexAttribArray(1);

    // color
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float32), (void*)(4 * sizeof(float32)));
    glEnableVertexAttribArray(2);

    return vao;
}

internal uint32
_gfx_ogl_mesh_buffer_create(GFX_VertexAttributeInfo* attributes)
{
    uint32 vao;
    uint32 vbo;
    uint32 ebo;

    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float32) * 8192, 0, GL_DYNAMIC_DRAW);

    uint32                          size_per_vertex = 0;
    GFX_VertexAttributeElementNode* n;
    for_each(n, attributes->first)
    {
        size_per_vertex += n->v.size;
    }

    uint64 offset = 0;
    for_each(n, attributes->first)
    {
        glVertexAttribPointer(n->v.index, n->v.component_count, n->v.type, GL_FALSE, size_per_vertex, (void*)offset);
        glEnableVertexAttribArray(n->v.index);
        offset += n->v.size;
    }

    return vao;
}