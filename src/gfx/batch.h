#pragma once
#include "base.h"
#include "sprite.h"

// NOTE(selim): when a new batch is created all underlying buffers initialized at this size.
// upon calling `batch_end` buffers are resized to their counts
#define R_DEFAULT_BATCH_VERTEX_CAPACITY 8192

typedef struct
{
    RenderKey key;
    uint64    element_count;
    uint64    vertex_count;

    VertexAtrribute_TexturedI* vertex_data;
    void*                      uniform_buffer;
} R_BatchTexturedQuad;

internal R_BatchTexturedQuad* r_batch_textured_quad_begin(RenderKey key, uint64 element_count);
internal void                 r_batch_textured_quad_push_vertex(R_BatchTexturedQuad* batch, Vec2 pos, Vec2 tex_coord, uint64 instance_id);
internal void                 r_batch_textured_quad_push(R_BatchTexturedQuad* batch, Rect rect, Bounds tex_coord);
internal void                 r_batch_textured_quad_push_sprite(R_BatchTexturedQuad* batch, SpriteAtlas* atlas, SpriteIndex sprite, Vec2 pos);

// TODO(selim): add `batch_end`
typedef struct
{
    RenderKey key;
    uint64    element_count;
    uint64    vertex_count;

    VertexAtrribute_Colored* vertex_data;
    void*                    uniform_buffer;
} R_BatchTriangle;

internal R_BatchTriangle* r_batch_triangle_begin(RenderKey key, uint64 capacity);
internal void             r_batch_triangle_push_vertex(R_BatchTriangle* batch, Vec2 pos, Vec4 color);
internal void             r_batch_triangle_push_strip(R_BatchTriangle* batch, Vec2 pos, Vec4 color);