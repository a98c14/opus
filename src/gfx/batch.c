#include "batch.h"

internal void
r_batch_sprite_push_vertex(R_Batch* batch, Vec2 pos, Vec2 tex_coord, uint64 instance_id)
{
    VertexAtrribute_TexturedI attr;
    attr.pos       = pos;
    attr.tex_coord = tex_coord;
    // attr.instance_id = instance_id;

    memcpy((uint8*)batch->vertex_buffer + batch->vertex_buffer_size, &attr, sizeof(attr));
    batch->vertex_buffer_size += sizeof(attr);
    batch->vertex_count++;
}

internal void
r_batch_sprite_push(R_Batch* batch, Rect rect, Bounds tex_coord)
{
    uint64 instance_id = batch->element_count;
    r_batch_sprite_push_vertex(batch, rect_bl(rect), bounds_bl(tex_coord), instance_id);
    r_batch_sprite_push_vertex(batch, rect_tl(rect), bounds_tl(tex_coord), instance_id);
    r_batch_sprite_push_vertex(batch, rect_br(rect), bounds_br(tex_coord), instance_id);
    r_batch_sprite_push_vertex(batch, rect_br(rect), bounds_br(tex_coord), instance_id);
    r_batch_sprite_push_vertex(batch, rect_tl(rect), bounds_tl(tex_coord), instance_id);
    r_batch_sprite_push_vertex(batch, rect_tr(rect), bounds_tr(tex_coord), instance_id);
    batch->element_count++;
}

internal void
r_batch_sprite_push_sprite(R_Batch* batch, SpriteAtlas* atlas, SpriteIndex sprite, Vec2 pos)
{
    r_batch_sprite_push(batch, r_sprite_rect_scaled(atlas, sprite, pos, 1), r_sprite_tex_coords(atlas, sprite));
}

// internal void
// r_batch_triangle_push_vertex(R_BatchNode* batch, Vec2 pos, Vec4 color)
// {
//     batch->vertex_data[batch->vertex_count++].pos   = pos;
//     batch->vertex_data[batch->vertex_count++].color = color;
//     xassert(batch->vertex_count < R_DEFAULT_BATCH_VERTEX_CAPACITY, "exceeded vertex capacity");
// }

// internal void
// r_batch_triangle_push_strip(R_BatchNode* batch, Vec2 pos, Vec4 color)
// {
//     if (batch->vertex_count > 2)
//     {
//         batch->vertex_data[batch->vertex_count++] = batch->vertex_data[batch->vertex_count - 2];
//         batch->vertex_data[batch->vertex_count++] = batch->vertex_data[batch->vertex_count - 2];
//         xassert(batch->vertex_count < R_DEFAULT_BATCH_VERTEX_CAPACITY, "exceeded vertex capacity");
//     }

//     r_batch_triangle_push_vertex(batch, pos, color);
// }
