#include "batch.h"

internal R_BatchTexturedQuad*
r_batch_textured_quad_begin(RenderKey key, uint64 element_count)
{
    Arena* arena = g_renderer->frame_arena;

    MaterialIndex material_index = render_key_mask(key, RenderKeyMaterialIndexBitStart, RenderKeyMaterialIndexBitCount);

    R_BatchTexturedQuad* batch = arena_push_struct_zero(arena, R_BatchTexturedQuad);
    batch->vertex_data         = arena_push_array(arena, VertexAtrribute_TexturedI, R_DEFAULT_BATCH_VERTEX_CAPACITY);
    batch->uniform_buffer      = arena_push(arena, g_renderer->materials[material_index].uniform_data_size * element_count);

    return batch;
}

internal void
r_batch_textured_quad_push_vertex(R_BatchTexturedQuad* batch, Vec2 pos, Vec2 tex_coord, uint64 instance_id)
{
    batch->vertex_data[batch->vertex_count].pos       = pos;
    batch->vertex_data[batch->vertex_count].tex_coord = tex_coord;
    // batch->vertex_data[batch->vertex_count].instance_id = instance_id;
    batch->vertex_count++;
}

internal void
r_batch_textured_quad_push(R_BatchTexturedQuad* batch, Rect rect, Bounds tex_coord)
{
    uint64 instance_id = batch->element_count;
    r_batch_textured_quad_push_vertex(batch, rect_bl(rect), bounds_bl(tex_coord), instance_id);
    r_batch_textured_quad_push_vertex(batch, rect_tl(rect), bounds_tl(tex_coord), instance_id);
    r_batch_textured_quad_push_vertex(batch, rect_br(rect), bounds_br(tex_coord), instance_id);
    r_batch_textured_quad_push_vertex(batch, rect_br(rect), bounds_br(tex_coord), instance_id);
    r_batch_textured_quad_push_vertex(batch, rect_tl(rect), bounds_tl(tex_coord), instance_id);
    r_batch_textured_quad_push_vertex(batch, rect_tr(rect), bounds_tr(tex_coord), instance_id);
    batch->element_count++;
}

internal void
r_batch_textured_quad_push_sprite(R_BatchTexturedQuad* batch, SpriteAtlas* atlas, SpriteIndex sprite, Vec2 pos)
{
    r_batch_textured_quad_push(batch, r_sprite_rect_scaled(atlas, sprite, pos, 10), r_sprite_tex_coords(atlas, sprite));
}

internal R_BatchTriangle*
r_batch_triangle_begin(RenderKey key, uint64 capacity)
{
    Arena* arena = g_renderer->frame_arena;

    MaterialIndex material_index = render_key_mask(key, RenderKeyMaterialIndexBitStart, RenderKeyMaterialIndexBitCount);

    R_BatchTriangle* batch = arena_push_struct_zero(arena, R_BatchTriangle);

    batch->element_count  = 1; // TODO(selim): this shouldn't be 1
    batch->vertex_data    = arena_push_array(arena, VertexAtrribute_Colored, R_DEFAULT_BATCH_VERTEX_CAPACITY);
    batch->uniform_buffer = arena_push(arena, g_renderer->materials[material_index].uniform_data_size * batch->element_count);

    return batch;
}

internal void
r_batch_triangle_push_vertex(R_BatchTriangle* batch, Vec2 pos, Vec4 color)
{
    batch->vertex_data[batch->vertex_count++].pos   = pos;
    batch->vertex_data[batch->vertex_count++].color = color;
    xassert(batch->vertex_count < R_DEFAULT_BATCH_VERTEX_CAPACITY, "exceeded vertex capacity");
}

internal void
r_batch_triangle_push_strip(R_BatchTriangle* batch, Vec2 pos, Vec4 color)
{
    if (batch->vertex_count > 2)
    {
        batch->vertex_data[batch->vertex_count++] = batch->vertex_data[batch->vertex_count - 2];
        batch->vertex_data[batch->vertex_count++] = batch->vertex_data[batch->vertex_count - 2];
        xassert(batch->vertex_count < R_DEFAULT_BATCH_VERTEX_CAPACITY, "exceeded vertex capacity");
    }

    r_batch_triangle_push_vertex(batch, pos, color);
}
