#include "draw_core.h"

internal void
d_context_init(Arena* persistent_arena, Arena* frame_arena, String asset_path)
{
    d_context              = arena_push_struct_zero(persistent_arena, D_Context);
    d_context->frame_arena = frame_arena;

    ArenaTemp temp             = scratch_begin(&persistent_arena, 1);
    d_context->material_text   = r_material_create(g_renderer, d_shader_opengl_font_vert, d_shader_opengl_font_frag, 0, R_DrawTypePackedBuffer);
    d_context->material_sprite = r_material_create(g_renderer, d_shader_opengl_sprite_vert, d_shader_opengl_sprite_frag, sizeof(D_ShaderDataSprite), R_DrawTypeInstanced);
    d_context->material_basic  = r_material_create(g_renderer, d_shader_opengl_basic_vert, d_shader_opengl_basic_frag, 0, R_DrawTypePackedBuffer);
    d_context->material_rect   = r_material_create(g_renderer, d_shader_opengl_rect_vert, d_shader_opengl_rect_frag, 0, R_DrawTypePackedBuffer);
    d_context->material_circle = r_material_create(g_renderer, d_shader_opengl_circle_vert, d_shader_opengl_circle_frag, sizeof(D_ShaderDataCircle), R_DrawTypeInstanced);
    d_context->active_pass     = 0;
    d_context->active_layer    = 5;

    StringList path = string_list();
    string_list_push(temp.arena, &path, asset_path);
    string_list_pushf(temp.arena, &path, "\\IBMPlexMono-Bold.ttf");
    String font_path       = string_list_join(temp.arena, &path, 0);
    d_context->active_font = font_load(string("ibx_mono"), font_path, GlyphAtlasTypeFreeType);

    scratch_end(temp);
}

internal void
d_mesh_push_vertex(VertexAtrribute_TexturedColored* vertex_buffer, uint64* vertex_count, Vec2 pos, Vec2 tex_coord, Color color)
{
    vertex_buffer[*vertex_count].pos       = pos;
    vertex_buffer[*vertex_count].tex_coord = tex_coord;
    vertex_buffer[*vertex_count].color     = color_v4(color);
    (*vertex_count)++;
}

internal void
d_mesh_push_triangle_strip(VertexAtrribute_TexturedColored* vertex_buffer, uint64* vertex_count, Vec2 pos, Vec2 tex_coord, Color color)
{
    vertex_buffer[*vertex_count].pos       = vertex_buffer[*(vertex_count)-2].pos;
    vertex_buffer[*vertex_count].tex_coord = vertex_buffer[*(vertex_count)-2].tex_coord;
    vertex_buffer[*vertex_count].color     = vertex_buffer[*(vertex_count)-2].color;
    (*vertex_count)++;

    vertex_buffer[*vertex_count].pos       = vertex_buffer[*(vertex_count)-2].pos;
    vertex_buffer[*vertex_count].tex_coord = vertex_buffer[*(vertex_count)-2].tex_coord;
    vertex_buffer[*vertex_count].color     = vertex_buffer[*(vertex_count)-2].color;
    (*vertex_count)++;

    vertex_buffer[*vertex_count].pos       = pos;
    vertex_buffer[*vertex_count].tex_coord = tex_coord;
    vertex_buffer[*vertex_count].color     = color_v4(color);
    (*vertex_count)++;
}

internal void
d_mesh_push_rect(VertexAtrribute_TexturedColored* vertex_buffer, uint64* vertex_count, Rect rect, Bounds tex_coord, Color color)
{
    d_mesh_push_vertex(vertex_buffer, vertex_count, rect_bl(rect), bounds_bl(tex_coord), color);
    d_mesh_push_vertex(vertex_buffer, vertex_count, rect_tl(rect), bounds_tl(tex_coord), color);
    d_mesh_push_vertex(vertex_buffer, vertex_count, rect_br(rect), bounds_br(tex_coord), color);
    d_mesh_push_vertex(vertex_buffer, vertex_count, rect_br(rect), bounds_br(tex_coord), color);
    d_mesh_push_vertex(vertex_buffer, vertex_count, rect_tl(rect), bounds_tl(tex_coord), color);
    d_mesh_push_vertex(vertex_buffer, vertex_count, rect_tr(rect), bounds_tr(tex_coord), color);
}

internal void
d_mesh_push_glyph(VertexAtrribute_TexturedColored* vertex_buffer, uint64* vertex_count, Glyph glyph, Vec2 pos, float32 size, Color color)
{
    float32 w = size * (glyph.plane_bounds.right - glyph.plane_bounds.left);
    float32 h = size * (glyph.plane_bounds.top - glyph.plane_bounds.bottom);

    pos.x = pos.x + glyph.plane_bounds.left * size;
    pos.y = pos.y + glyph.plane_bounds.bottom * size;

    d_mesh_push_rect(vertex_buffer, vertex_count, rect_at(pos, vec2(w, h), AlignmentBottomLeft), glyph.atlas_bounds, color);
}

internal void
d_mesh_push_string(VertexAtrribute_TexturedColored* vertex_buffer, uint64* vertex_count, GlyphAtlas* atlas, String str, Vec2 pos, float32 size, Color c)
{
    float32 advance_x = 0;
    for (uint32 i = 0; i < str.length; i++)
    {
        Glyph glyph = atlas->glyphs[str.value[i] - 32];

        Vec2 glyph_pos = vec2(pos.x + advance_x, pos.y);
        d_mesh_push_glyph(vertex_buffer, vertex_count, glyph, glyph_pos, size, c);
        advance_x += glyph.advance * size;
    }
}

internal void
d_mesh_push_line(VertexAtrribute_TexturedColored* vertex_buffer, uint64* vertex_count, Vec2 start, Vec2 end, float32 thickness, Color c)
{
    Vec2 heading = heading_to_vec2(start, end);
    Vec2 A1      = add_vec2(start, mul_vec2_f32(rotate90_vec2(heading), thickness));
    Vec2 A2      = add_vec2(start, mul_vec2_f32(rotate90i_vec2(heading), thickness));
    Vec2 B1      = add_vec2(end, mul_vec2_f32(rotate90_vec2(heading), thickness));
    Vec2 B2      = add_vec2(end, mul_vec2_f32(rotate90i_vec2(heading), thickness));

    d_mesh_push_vertex(vertex_buffer, vertex_count, A1, vec2(0, 0), c);
    d_mesh_push_vertex(vertex_buffer, vertex_count, A2, vec2(0, 1), c);
    d_mesh_push_vertex(vertex_buffer, vertex_count, B1, vec2(1, 0), c);
    d_mesh_push_vertex(vertex_buffer, vertex_count, B1, vec2(1, 0), c);
    d_mesh_push_vertex(vertex_buffer, vertex_count, A2, vec2(0, 1), c);
    d_mesh_push_vertex(vertex_buffer, vertex_count, B2, vec2(1, 1), c);
}

/** draw functions */
internal void
d_line(Vec2 start, Vec2 end, float32 thickness, Color c)
{
    VertexAtrribute_TexturedColored* vertices     = arena_push_array(d_context->frame_arena, VertexAtrribute_TexturedColored, 6);
    uint64                           vertex_count = 0;

    d_mesh_push_line(vertices, &vertex_count, start, end, thickness, c);

    R_Batch batch;
    batch.key                 = render_key_new(ViewTypeWorld, d_context->active_layer, d_context->active_pass, 0, MeshTypeDynamic, d_context->material_rect);
    batch.element_count       = 1;
    batch.draw_instance_count = vertex_count;
    batch.vertex_buffer       = vertices;
    batch.vertex_buffer_size  = sizeof(VertexAtrribute_TexturedColored) * vertex_count;
    batch.uniform_buffer      = 0;
    r_batch_commit(batch);
}

internal void
d_circle(Vec2 pos, float32 radius, float32 thickness, Color c)
{
    D_ShaderDataCircle* uniform_data = arena_push_struct(d_context->frame_arena, D_ShaderDataCircle);
    uniform_data->model              = transform_quad_aligned(pos, vec2(radius, radius));
    uniform_data->thickness          = thickness;
    uniform_data->color              = color_v4(c);

    R_Batch batch;
    batch.key            = render_key_new(ViewTypeWorld, d_context->active_layer, d_context->active_pass, 0, MeshTypeQuad, d_context->material_circle);
    batch.element_count  = 1;
    batch.uniform_buffer = uniform_data;
    r_batch_commit(batch);
}

internal void
d_rect(Rect r, float32 thickness, Color c)
{
    xassert(thickness >= 0, "rect thickness can't be lower than zero, use zero for filled rects.");
    VertexAtrribute_TexturedColored* vertices     = arena_push_array(d_context->frame_arena, VertexAtrribute_TexturedColored, 6 * 4);
    uint64                           vertex_count = 0;

    if (thickness == 0)
    {
        Bounds b = {.bl = vec2(0, 0), .tr = vec2(1, 1)};
        d_mesh_push_rect(vertices, &vertex_count, r, b, c);
    }
    else
    {
        d_mesh_push_line(vertices, &vertex_count, add_vec2(rect_tl(r), vec2(0, -thickness)), add_vec2(rect_tr(r), vec2(0, -thickness)), thickness, c);
        d_mesh_push_line(vertices, &vertex_count, add_vec2(rect_br(r), vec2(0, thickness)), add_vec2(rect_bl(r), vec2(0, thickness)), thickness, c);
        d_mesh_push_line(vertices, &vertex_count, add_vec2(rect_bl(r), vec2(thickness, 1.5 * thickness)), add_vec2(rect_tl(r), vec2(thickness, 1.5 * -thickness)), thickness, c);
        d_mesh_push_line(vertices, &vertex_count, add_vec2(rect_tr(r), vec2(-thickness, 1.5 * -thickness)), add_vec2(rect_br(r), vec2(-thickness, 1.5 * thickness)), thickness, c);
    }

    R_Batch batch;
    batch.key                 = render_key_new(d_context->active_view, d_context->active_layer, d_context->active_pass, 0, MeshTypeDynamic, d_context->material_rect);
    batch.element_count       = 1;
    batch.draw_instance_count = vertex_count;
    batch.vertex_buffer       = vertices;
    batch.vertex_buffer_size  = sizeof(VertexAtrribute_TexturedColored) * vertex_count;
    batch.uniform_buffer      = 0;
    r_batch_commit(batch);
}

internal void
d_quad(Quad q, float32 thickness, Color c)
{
    xassert(thickness >= 0, "rect thickness can't be lower than zero, use zero for filled rects.");

    Vec2 bl = q.vertices[QuadVertexIndexBottomLeft];
    Vec2 tl = q.vertices[QuadVertexIndexTopLeft];
    Vec2 br = q.vertices[QuadVertexIndexBottomRight];
    Vec2 tr = q.vertices[QuadVertexIndexTopRight];
    d_line(tl, tr, thickness, c);
    d_line(tr, br, thickness, c);
    d_line(br, bl, thickness, c);
    d_line(bl, tl, thickness, c);
}

internal void
d_string(Vec2 pos, String str, float32 size, Color c, Alignment alignment)
{
    GlyphAtlas* atlas         = font_get_atlas(d_context->active_font, size);
    Rect        string_bounds = text_calculate_bounds(atlas, pos, alignment, str, size);
    Vec2        base_offset   = {
                 .x = string_bounds.w * FontAlignmentMultiplierX[alignment],
                 .y = string_bounds.h * FontAlignmentMultiplierY[alignment]};

    VertexAtrribute_TexturedColored* vertices     = arena_push_array(d_context->frame_arena, VertexAtrribute_TexturedColored, str.length * 6);
    uint64                           vertex_count = 0;
    d_mesh_push_string(vertices, &vertex_count, atlas, str, add_vec2(pos, base_offset), size, c);

    R_Batch batch;
    batch.key                 = render_key_new(ViewTypeWorld, d_context->active_layer, d_context->active_pass, atlas->texture, MeshTypeDynamic, d_context->material_text);
    batch.element_count       = 1;
    batch.draw_instance_count = vertex_count;
    batch.vertex_buffer       = vertices;
    batch.vertex_buffer_size  = sizeof(VertexAtrribute_TexturedColored) * vertex_count;
    batch.uniform_buffer      = 0;
    r_batch_commit(batch);
}

internal void
d_sprite_many(SpriteAtlas atlas, D_DrawDataSprite* draw_data, uint32 sprite_count, bool32 sort)
{
    if (sort)
    {
        qsort(draw_data, sprite_count, sizeof(D_DrawDataSprite), d_compare_sprite_draw_data);
    }

    D_ShaderDataSprite* uniform_data = arena_push_array(d_context->frame_arena, D_ShaderDataSprite, sprite_count);
    for (uint64 i = 0; i < sprite_count; i++)
    {
        D_DrawDataSprite* data = &draw_data[i];

        const Sprite* sprite = &atlas.sprites[data->sprite];
        Vec2          flip   = vec2(-2 * ((data->flags & D_DrawFlagsSpriteFlipX) > 0) + 1, -2 * ((data->flags & D_DrawFlagsSpriteFlipY) > 0) + 1);
        Vec2          pivot  = r_sprite_get_pivot(*sprite, draw_data->scale, flip);
        pivot                = mul_vec2_f32(pivot, !(data->flags & D_DrawFlagsSpriteIgnorePivot));
        Vec2 scale           = vec2(sprite->size.w * draw_data->scale.x * flip.x, sprite->size.h * draw_data->scale.y * flip.y);

        uniform_data[i].bounds = sprite->rect.v;
        uniform_data[i].color  = color_v4(ColorWhite);
        uniform_data[i].model  = data->rotation == 0 ? transform_quad_aligned_at_pivot(data->position.xy, scale, pivot)
                                                     : transform_quad_around_pivot(data->position.xy, scale, data->rotation, pivot);
    }

    R_Batch batch;
    batch.key            = render_key_new(d_context->active_view, d_context->active_layer, d_context->active_pass, atlas.texture, MeshTypeQuad, d_context->material_sprite);
    batch.element_count  = sprite_count;
    batch.uniform_buffer = uniform_data;
    r_batch_commit(batch);
}

internal void
d_sprite(SpriteAtlas atlas, SpriteIndex sprite_index, Vec2 pos, Vec2 scale, float32 rotation)
{
    D_DrawDataSprite data;
    data.sprite   = sprite_index;
    data.position = vec3(pos.x, pos.y, 0);
    data.scale    = scale;
    data.rotation = rotation;
    data.flags    = D_DrawFlagsSpriteNone;
    d_sprite_many(atlas, &data, 1, false);
}

/** debug draw functions */
internal void
d_debug_line(Vec2 start, Vec2 end)
{
    d_line(start, end, 1.2 * g_renderer->ppu, ColorRed400);
}

internal void
d_debug_line2(Vec2 start, Vec2 end)
{
    d_line(start, end, 1.2 * g_renderer->ppu, ColorGreen400);
}

internal void
d_debug_rect(Rect r)
{
    d_rect(r, 1.2 * g_renderer->ppu, ColorRed400);
}

internal void
d_debug_rect2(Rect r)
{
    d_rect(r, 1.2 * g_renderer->ppu, ColorGreen400);
}

internal int
d_compare_sprite_draw_data(const void* p, const void* q)
{
    float32 x = (*(const D_DrawDataSprite*)p).position.z;
    float32 y = (*(const D_DrawDataSprite*)q).position.z;
    return (x < y) ? -1 : (x > y) ? 1
                                  : 0;
}
