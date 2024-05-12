#include "batch.h"

internal void
r_batch_push_vertex(Vec2 pos, Vec2 tex_coord, uint64 instance_id)
{
    VertexAtrribute_TexturedI attr;
    attr.pos       = pos;
    attr.tex_coord = tex_coord;
    // attr.instance_id = instance_id;

    R_Batch* batch = r_active_batch();
    memcpy((uint8*)batch->vertex_buffer + batch->vertex_buffer_size, &attr, sizeof(attr));
    batch->vertex_buffer_size += sizeof(attr);
    batch->vertex_count++;
}

internal void
r_batch_sprite_push(Rect rect, Bounds tex_coord)
{
    R_Batch* batch       = r_active_batch();
    uint64   instance_id = batch->element_count;
    r_batch_push_vertex(rect_bl(rect), bounds_bl(tex_coord), instance_id);
    r_batch_push_vertex(rect_tl(rect), bounds_tl(tex_coord), instance_id);
    r_batch_push_vertex(rect_br(rect), bounds_br(tex_coord), instance_id);
    r_batch_push_vertex(rect_br(rect), bounds_br(tex_coord), instance_id);
    r_batch_push_vertex(rect_tl(rect), bounds_tl(tex_coord), instance_id);
    r_batch_push_vertex(rect_tr(rect), bounds_tr(tex_coord), instance_id);
    batch->element_count++;
}

internal void
r_batch_sprite_push_sprite(SpriteAtlas* atlas, SpriteIndex sprite, Vec2 pos)
{
    r_batch_sprite_push(r_sprite_rect_scaled(atlas, sprite, pos, 1), r_sprite_tex_coords(atlas, sprite));
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
