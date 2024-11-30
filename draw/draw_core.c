#include "draw_core.h"

internal void
d_context_init(Arena* persistent_arena, Arena* frame_arena, String asset_path)
{
    d_context              = arena_push_struct_zero(persistent_arena, D_Context);
    d_context->perm_arena  = persistent_arena;
    d_context->frame_arena = frame_arena;

    ArenaTemp temp               = scratch_begin(&persistent_arena, 1);
    d_context->material_text     = gfx_material_new(d_shader_opengl_font_vert, d_shader_opengl_font_frag, 0, GFX_DrawTypePackedBuffer);
    d_context->material_sprite   = gfx_material_new(d_shader_opengl_sprite_vert, d_shader_opengl_sprite_frag, sizeof(D_ShaderDataSprite), GFX_DrawTypeInstanced);
    d_context->material_basic    = gfx_material_new(d_shader_opengl_basic_vert, d_shader_opengl_basic_frag, 0, GFX_DrawTypePackedBuffer);
    d_context->material_rect     = gfx_material_new(d_shader_opengl_rect_vert, d_shader_opengl_rect_frag, 0, GFX_DrawTypePackedBuffer);
    d_context->material_circle   = gfx_material_new(d_shader_opengl_circle_vert, d_shader_opengl_circle_frag, sizeof(D_ShaderDataCircle), GFX_DrawTypeInstanced);
    d_context->material_triangle = gfx_material_new(d_shader_opengl_triangle_vert, d_shader_opengl_triangle_frag, sizeof(D_ShaderDataTriangle), GFX_DrawTypeInstanced);
    d_context->active_pass       = 0;
    d_context->active_layer      = 5;

    StringList path = string_list();
    string_list_push(temp.arena, &path, asset_path);
    string_list_pushf(temp.arena, &path, "\\IBMPlexMono-Bold.ttf");
    String font_path       = string_list_join(temp.arena, &path, 0);
    d_context->active_font = font_load(string("ibx_mono"), font_path, GlyphAtlasTypeFreeType);
    scratch_end(temp);
}

internal void
d_mesh_push_vertex(GFX_VertexAtrribute_TexturedColored* vertex_buffer, uint32* vertex_count, Vec2 pos, Vec2 tex_coord, Color color)
{
    vertex_buffer[*vertex_count].pos       = pos;
    vertex_buffer[*vertex_count].tex_coord = tex_coord;
    vertex_buffer[*vertex_count].color     = color_v4(color);
    (*vertex_count)++;
}

internal void
d_mesh_push_triangle_strip(GFX_VertexAtrribute_TexturedColored* vertex_buffer, uint32* vertex_count, Vec2 pos, Vec2 tex_coord, Color color)
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
d_mesh_push_rect(GFX_VertexAtrribute_TexturedColored* vertex_buffer, uint32* vertex_count, Rect rect, Bounds tex_coord, Color color)
{
    d_mesh_push_vertex(vertex_buffer, vertex_count, rect_bl(rect), bounds_bl(tex_coord), color);
    d_mesh_push_vertex(vertex_buffer, vertex_count, rect_tl(rect), bounds_tl(tex_coord), color);
    d_mesh_push_vertex(vertex_buffer, vertex_count, rect_br(rect), bounds_br(tex_coord), color);
    d_mesh_push_vertex(vertex_buffer, vertex_count, rect_br(rect), bounds_br(tex_coord), color);
    d_mesh_push_vertex(vertex_buffer, vertex_count, rect_tl(rect), bounds_tl(tex_coord), color);
    d_mesh_push_vertex(vertex_buffer, vertex_count, rect_tr(rect), bounds_tr(tex_coord), color);
}

internal void
d_mesh_push_glyph(GFX_VertexAtrribute_TexturedColored* vertex_buffer, uint32* vertex_count, Glyph glyph, Vec2 pos, float32 size, Color color)
{
    float32 w = size * (glyph.plane_bounds.right - glyph.plane_bounds.left);
    float32 h = size * (glyph.plane_bounds.top - glyph.plane_bounds.bottom);

    pos.x = pos.x + glyph.plane_bounds.left * size;
    pos.y = pos.y + glyph.plane_bounds.bottom * size;

    d_mesh_push_rect(vertex_buffer, vertex_count, rect_at(pos, vec2(w, h), AlignmentBottomLeft), glyph.atlas_bounds, color);
}

internal void
d_mesh_push_string(GFX_VertexAtrribute_TexturedColored* vertex_buffer, uint32* vertex_count, GlyphAtlas* atlas, String str, Vec2 pos, float32 size, Color c)
{
    float32 advance_x = 0;
    pos.x             = roundf(pos.x);
    pos.y             = roundf(pos.y);
    for (uint32 i = 0; i < str.length; i++)
    {
        Glyph glyph     = atlas->glyphs[str.value[i] - 32];
        Vec2  glyph_pos = vec2(pos.x + advance_x, pos.y);
        d_mesh_push_glyph(vertex_buffer, vertex_count, glyph, glyph_pos, size, c);
        advance_x += glyph.advance * size;
    }
}

internal void
d_mesh_push_line(GFX_VertexAtrribute_TexturedColored* vertex_buffer, uint32* vertex_count, Vec2 start, Vec2 end, float32 thickness, Color c)
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
    GFX_VertexAtrribute_TexturedColored* vertices     = arena_push_array(d_context->frame_arena, GFX_VertexAtrribute_TexturedColored, 6);
    uint32                               vertex_count = 0;

    d_mesh_push_line(vertices, &vertex_count, start, end, thickness, c);

    GFX_Batch batch;
    batch.key                 = gfx_render_key_new(GFX_ViewTypeWorld, d_context->active_layer, d_context->active_pass, 0, GFX_MeshTypeDynamic, d_context->material_rect);
    batch.element_count       = 1;
    batch.draw_instance_count = vertex_count;
    batch.vertex_buffer       = vertices;
    batch.vertex_buffer_size  = sizeof(GFX_VertexAtrribute_TexturedColored) * vertex_count;
    batch.uniform_buffer      = 0;
    gfx_batch_commit(batch);
}

internal void
d_direction(Vec2 start, Vec2 direction, float32 scale, float32 thickness, Color c)
{
    d_line(start, add_vec2(start, mul_vec2_f32(direction, scale)), thickness, c);
}

internal void
d_triangle(Vec2 pos, Vec2 scale, float32 rotation, Color c)
{
    D_ShaderDataTriangle* uniform_data = arena_push_struct(d_context->frame_arena, D_ShaderDataTriangle);
    uniform_data->model                = transform_quad(pos, vec2(scale.x, scale.y), rotation - 90);
    uniform_data->color                = color_v4(c);

    GFX_Batch batch;
    batch.key            = gfx_render_key_new(GFX_ViewTypeWorld, d_context->active_layer, d_context->active_pass, 0, GFX_MeshTypeTriangle, d_context->material_triangle);
    batch.element_count  = 1;
    batch.uniform_buffer = uniform_data;
    gfx_batch_commit(batch);
}

internal void
d_circle_scaled(Vec2 pos, float32 radius, Vec2 scale, float32 thickness, Color c)
{
    D_ShaderDataCircle* uniform_data = arena_push_struct(d_context->frame_arena, D_ShaderDataCircle);
    uniform_data->model              = transform_quad_aligned(pos, vec2(radius * scale.x, radius * scale.y));
    uniform_data->thickness          = thickness;
    uniform_data->color              = color_v4(c);

    GFX_Batch batch;
    batch.key            = gfx_render_key_new(GFX_ViewTypeWorld, d_context->active_layer, d_context->active_pass, 0, GFX_MeshTypeQuad, d_context->material_circle);
    batch.element_count  = 1;
    batch.uniform_buffer = uniform_data;
    gfx_batch_commit(batch);
}

internal void
d_circle(Vec2 pos, float32 radius, float32 thickness, Color c)
{
    d_circle_scaled(pos, radius, vec2_one(), thickness, c);
}

internal Rect
d_rect(Rect r, float32 thickness, Color c)
{
    xassert(thickness >= 0, "rect thickness can't be lower than zero, use zero for filled rects.");
    GFX_VertexAtrribute_TexturedColored* vertices     = arena_push_array(d_context->frame_arena, GFX_VertexAtrribute_TexturedColored, 6 * 4);
    uint32                               vertex_count = 0;

    if (thickness == 0)
    {
        Bounds b = {.bl = vec2(0, 0), .tr = vec2(1, 1)};
        d_mesh_push_rect(vertices, &vertex_count, r, b, c);
    }
    else
    {
        d_mesh_push_line(vertices, &vertex_count, add_vec2(rect_tl(r), vec2(0, -thickness)), add_vec2(rect_tr(r), vec2(0, -thickness)), thickness, c);
        d_mesh_push_line(vertices, &vertex_count, add_vec2(rect_br(r), vec2(0, thickness)), add_vec2(rect_bl(r), vec2(0, thickness)), thickness, c);
        d_mesh_push_line(vertices, &vertex_count, add_vec2(rect_bl(r), vec2(thickness, 1.5f * thickness)), add_vec2(rect_tl(r), vec2(thickness, 1.5f * -thickness)), thickness, c);
        d_mesh_push_line(vertices, &vertex_count, add_vec2(rect_tr(r), vec2(-thickness, 1.5f * -thickness)), add_vec2(rect_br(r), vec2(-thickness, 1.5f * thickness)), thickness, c);
    }

    GFX_Batch batch;
    batch.key                 = gfx_render_key_new(d_context->active_view, d_context->active_layer, d_context->active_pass, 0, GFX_MeshTypeDynamic, d_context->material_rect);
    batch.element_count       = 1;
    batch.draw_instance_count = vertex_count;
    batch.vertex_buffer       = vertices;
    batch.vertex_buffer_size  = sizeof(GFX_VertexAtrribute_TexturedColored) * vertex_count;
    batch.uniform_buffer      = 0;
    gfx_batch_commit(batch);
    return r;
}

internal void
d_material_raw(MaterialIndex material, void* shader_data)
{
    uint64 size         = gfx_material_uniform_data_size(material);
    void*  uniform_data = arena_push_zero(d_context->frame_arena, size);
    if (shader_data)
    {
        memory_copy(uniform_data, shader_data, size);
    }

    GFX_Batch batch;
    batch.key            = gfx_render_key_new(GFX_ViewTypeWorld, d_context->active_layer, d_context->active_pass, 0, GFX_MeshTypeQuad, material);
    batch.element_count  = 1;
    batch.uniform_buffer = uniform_data;
    gfx_batch_commit(batch);
}

internal void
d_quad(Quad q, float32 thickness, Color c)
{
    xassert(thickness >= 0, "quad thickness can't be lower than zero, use zero for filled rects.");
    GFX_VertexAtrribute_TexturedColored* vertices     = arena_push_array(d_context->frame_arena, GFX_VertexAtrribute_TexturedColored, 6 * 6);
    uint32                               vertex_count = 0;

    Vec2 bl = q.vertices[QuadVertexIndexBottomLeft];
    Vec2 tl = q.vertices[QuadVertexIndexTopLeft];
    Vec2 br = q.vertices[QuadVertexIndexBottomRight];
    Vec2 tr = q.vertices[QuadVertexIndexTopRight];
    d_mesh_push_line(vertices, &vertex_count, tl, tr, thickness, c);
    d_mesh_push_line(vertices, &vertex_count, tr, br, thickness, c);
    d_mesh_push_line(vertices, &vertex_count, br, bl, thickness, c);
    d_mesh_push_line(vertices, &vertex_count, bl, tl, thickness, c);

    // draw normals
    Vec2 nh = q.normals[QuadNormalIndexHorizontal];
    Vec2 nv = q.normals[QuadNormalIndexVertical];
    d_mesh_push_line(vertices, &vertex_count, lerp_vec2(tl, tr, 0.5), add_vec2(lerp_vec2(tl, tr, 0.5), mul_vec2_f32(nv, px(30))), thickness, ColorSlate400);
    d_mesh_push_line(vertices, &vertex_count, lerp_vec2(tl, bl, 0.5), add_vec2(lerp_vec2(tl, bl, 0.5), mul_vec2_f32(nh, px(30))), thickness, ColorSlate400);

    GFX_Batch batch;
    batch.key                 = gfx_render_key_new(d_context->active_view, d_context->active_layer, d_context->active_pass, 0, GFX_MeshTypeDynamic, d_context->material_rect);
    batch.element_count       = 1;
    batch.draw_instance_count = vertex_count;
    batch.vertex_buffer       = vertices;
    batch.vertex_buffer_size  = sizeof(GFX_VertexAtrribute_TexturedColored) * vertex_count;
    batch.uniform_buffer      = 0;
    gfx_batch_commit(batch);
}

internal Rect
d_string(Rect r, String str, float32 size, Color c, Anchor anchor)
{
    Vec2 pos = rect_get(r, anchor.parent);
    return d_string_at(pos, str, size, c, anchor.child);
}

internal Rect
d_string_at(Vec2 pos, String str, float32 size, Color c, Alignment alignment)
{
    return d_string_raw(pos, str, size, c, alignment, d_context->material_text);
}

internal Rect
d_string_raw(Vec2 pos, String str, float32 size, Color c, Alignment alignment, MaterialIndex material)
{

    GlyphAtlas* atlas         = font_get_atlas(d_context->active_font, size);
    Rect        string_bounds = text_calculate_bounds(atlas, pos, alignment, str, size);
    Vec2        base_offset   = {
                 .x = string_bounds.w * FontAlignmentMultiplierX[alignment],
                 .y = string_bounds.h * FontAlignmentMultiplierY[alignment]};

    GFX_VertexAtrribute_TexturedColored* vertices     = arena_push_array(d_context->frame_arena, GFX_VertexAtrribute_TexturedColored, str.length * 6);
    uint32                               vertex_count = 0;
    d_mesh_push_string(vertices, &vertex_count, atlas, str, add_vec2(pos, base_offset), size, c);

    GFX_Batch batch;
    batch.key                 = gfx_render_key_new(d_context->active_view, d_context->active_layer, d_context->active_pass, atlas->texture, GFX_MeshTypeDynamic, material);
    batch.element_count       = 1;
    batch.draw_instance_count = vertex_count;
    batch.vertex_buffer       = vertices;
    batch.vertex_buffer_size  = sizeof(GFX_VertexAtrribute_TexturedColored) * vertex_count;
    batch.uniform_buffer      = 0;
    gfx_batch_commit(batch);
    return string_bounds;
}

internal void
d_sprite_many(D_SpriteAtlas atlas, D_DrawDataSprite* draw_data, uint32 sprite_count, bool32 sort)
{
    if (sort)
    {
        qsort(draw_data, sprite_count, sizeof(D_DrawDataSprite), d_compare_sprite_draw_data);
    }

    D_ShaderDataSprite* uniform_data = arena_push_array(d_context->frame_arena, D_ShaderDataSprite, sprite_count);
    for (uint64 i = 0; i < sprite_count; i++)
    {
        D_DrawDataSprite* data = &draw_data[i];

        const D_Sprite* sprite = &atlas.sprites[data->sprite];
        Vec2            flip   = vec2(-2.0f * ((data->flags & D_DrawFlagsSpriteFlipX) > 0) + 1, -2.0f * ((data->flags & D_DrawFlagsSpriteFlipY) > 0) + 1);
        Vec2            pivot  = d_sprite_pivot(*sprite, draw_data->scale, flip);
        pivot                  = mul_vec2_f32(pivot, !(data->flags & D_DrawFlagsSpriteIgnorePivot));
        Vec2 scale             = vec2(sprite->size.w * draw_data->scale.x * flip.x, sprite->size.h * draw_data->scale.y * flip.y);

        uniform_data[i].bounds = sprite->rect.v;
        uniform_data[i].color  = color_v4(data->color);
        uniform_data[i].model  = data->rotation == 0 ? transform_quad_aligned_at_pivot(data->position.xy, scale, pivot)
                                                     : transform_quad_around_pivot(data->position.xy, scale, data->rotation, pivot);
    }

    GFX_Batch batch;
    batch.key            = gfx_render_key_new(d_context->active_view, d_context->active_layer, d_context->active_pass, atlas.texture, GFX_MeshTypeQuad, d_context->material_sprite);
    batch.element_count  = sprite_count;
    batch.uniform_buffer = uniform_data;
    gfx_batch_commit(batch);
}

internal void
d_sprite_at(D_SpriteAtlas atlas, D_SpriteIndex sprite_index, Vec2 pos, Vec2 scale, float32 rotation, Color color)
{
    D_DrawDataSprite data;
    data.sprite   = sprite_index;
    data.position = vec3(pos.x, pos.y, 0);
    data.scale    = scale;
    data.color    = color;
    data.rotation = rotation;
    data.flags    = D_DrawFlagsSpriteNone;
    d_sprite_many(atlas, &data, 1, false);
}

internal Rect
d_sprite(D_SpriteAtlas* atlas, D_SpriteIndex sprite_index, Rect rect, Vec2 scale, Anchor anchor, Color c)
{
    const D_Sprite* sprite = &atlas->sprites[sprite_index];

    Rect sprite_bounds = rect_from_wh(sprite->size.w * scale.x, sprite->size.h * scale.y);
    Rect final         = rect_anchor(sprite_bounds, rect, anchor);

    D_ShaderDataSprite* uniform_data = arena_push_array(d_context->frame_arena, D_ShaderDataSprite, 1);
    uniform_data[0].bounds           = sprite->rect.v;
    uniform_data[0].color            = color_v4(c);
    uniform_data[0].model            = transform_quad_aligned(final.center, final.size);

    GFX_Batch batch;
    batch.key            = gfx_render_key_new(d_context->active_view, d_context->active_layer, d_context->active_pass, atlas->texture, GFX_MeshTypeQuad, d_context->material_sprite);
    batch.element_count  = 1;
    batch.uniform_buffer = uniform_data;
    gfx_batch_commit(batch);

    return final;
}

internal void
d_arrow_pro(Vec2 start, Vec2 end, float32 thickness, float32 head_size, Color color)
{
    float32 rotation    = angle_vec2(sub_vec2(end, start));
    Vec2    head_offset = rotate_vec2(vec2(0, -head_size / 2.0f), rotation - 90);

    d_line(start, add_vec2(end, head_offset), thickness, color);
    d_triangle(add_vec2(end, head_offset), vec2(head_size, head_size), rotation, color);
}

internal void
d_arrow(Vec2 start, Vec2 end, float32 size, Color color)
{
    d_arrow_pro(start, end, size, size * 8, color);
}

/** debug draw functions */
internal void
d_debug_line(Vec2 start, Vec2 end)
{
    d_line(start, end, px(1.2f), ColorRed400);
}

internal void
d_debug_line2(Vec2 start, Vec2 end)
{
    d_line(start, end, px(1.2f), ColorGreen400);
}

internal void
d_debug_rect(Rect r)
{
    d_rect(r, px(1.2f), ColorRed400);
}

internal void
d_debug_rect2(Rect r)
{
    d_rect(r, px(1.2f), ColorGreen400);
}

internal int
d_compare_sprite_draw_data(const void* p, const void* q)
{
    float32 x = (*(const D_DrawDataSprite*)p).position.z;
    float32 y = (*(const D_DrawDataSprite*)q).position.z;
    return (x < y) ? -1 : (x > y) ? 1
                                  : 0;
}
