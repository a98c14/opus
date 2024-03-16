#include "draw.h"

#ifndef SHADER_PATH
#define SHADER_PATH "..\\src\\shaders"
#endif

#ifndef ASSET_PATH
#define ASSET_PATH "..\\assets"
#endif

internal void
draw_context_init(Arena* arena, Arena* temp_arena, Renderer* renderer, PassIndex default_pass)
{
    d_state                   = arena_push_struct_zero(arena, D_State);
    d_state->persistent_arena = arena;
    d_state->frame_arena      = temp_arena;
    draw_context_push(d_default_node.sort_layer, d_default_node.view, default_pass);
    draw_context_set_font_style(d_default_font_style);

    d_state->renderer = renderer;
    d_state->camera   = &renderer->camera;

    ArenaTemp temp          = scratch_begin(&arena, 1);
    d_state->material_basic = material_new(
        renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\basic_instanced.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\basic_instanced.frag")),
        sizeof(ShaderDataBasic),
        true);
    d_state->material_line = material_new(
        renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\line.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\line.frag")),
        sizeof(ShaderDataLine),
        false);

    d_state->material_basic_texture = material_new(
        renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\basic_texture.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\basic_texture.frag")),
        sizeof(ShaderDataBasicTexture),
        false);

    d_state->material_triangle = material_new(
        renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\triangle.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\triangle.frag")),
        sizeof(ShaderDataTriangle),
        false);

    d_state->material_rounded_rect = material_new(
        renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\rect_rounded.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\rect_rounded.frag")),
        sizeof(ShaderDataRectRounded),
        false);

    d_state->material_boid = material_new(
        renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\boid_instanced.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\boid_instanced.frag")),
        sizeof(ShaderDataBoid),
        true);

    d_state->material_circle = material_new(
        renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\circle.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\circle.frag")),
        sizeof(ShaderDataCircle),
        false);

    d_state->material_circle_instanced = material_new(
        renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\circle_instanced.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\circle_instanced.frag")),
        sizeof(ShaderDataCircle),
        true);

    d_state->material_text = material_new(
        renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\text.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\text.frag")),
        sizeof(ShaderDataText),
        true);

    d_state->material_sprite = material_new(
        renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\sprite.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\sprite.frag")),
        sizeof(ShaderDataSprite),
        false);

    d_state->material_sprite_border = material_new(
        renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\sprite_border.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\sprite_border.frag")),
        sizeof(ShaderDataSpriteBorder),
        false);

    d_state->material_text_free_type = material_new(
        g_renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\freetype_text.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\freetype_text.frag")),
        sizeof(ShaderDataText),
        true);

    d_state->material_text_free_type_sdf = material_new(
        g_renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\freetype_text_sdf.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\freetype_text_sdf.frag")),
        sizeof(ShaderDataText),
        true);

    d_state->material_basic_trail = material_new(
        g_renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\basic_trail.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\basic_trail.frag")),
        sizeof(ShaderDataTrail),
        false);

    String        font_path = string(ASSET_PATH "\\open_sans.ttf");
    String        font_name = string("open_sans");
    FontFaceIndex face      = font_load(font_name, font_path, GlyphAtlasTypeFreeType);
    draw_activate_font(face);
    scratch_end(temp);
    log_debug("initialized draw context");
}

internal void
draw_context_set_font_style(DrawStyleFont style)
{
    d_state->ctx->font_style = style;
}

internal void
draw_context_activate_atlas(SpriteAtlas* atlas)
{
    d_state->sprite_atlas = atlas;
}

internal void
draw_line(Vec2 start, Vec2 end, Color color, float32 thickness)
{
    RenderKey      key         = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_line);
    ShaderDataLine shader_data = {.color = color_v4(color)};
    r_draw_single(key, transform_line(start, end, thickness), &shader_data);
}

internal void
draw_heading(Vec2 origin, Vec2 heading, Color color, float32 thickness)
{
    RenderKey      key         = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_line);
    ShaderDataLine shader_data = {.color = color_v4(color)};
    r_draw_single(key, transform_line(origin, add_vec2(origin, mul_vec2_f32(heading, 20)), thickness), &shader_data);
}

internal void
draw_line_fixed(Vec2 position, float32 length, float32 rotation, Color color, float32 thickness)
{
    xassert(length > 0, "line length needs to be larger than 0 for `transform_line_rotated`");
    RenderKey      key         = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_line);
    ShaderDataLine shader_data = {.color = color_v4(color)};
    r_draw_single(key, transform_line_rotated(position, length, rotation, thickness), &shader_data);
}

internal void
draw_arrow(Vec2 position, float32 length, float32 rotation, Color color, float32 thickness)
{
    xassert(length > 0, "line length needs to be larger than 0 for `transform_line_rotated`");
    RenderKey line_key  = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_line);
    RenderKey arrow_key = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->triangle, d_state->material_basic);

    ShaderDataLine shader_data = {.color = color_v4(color)};
    r_draw_single(line_key, transform_quad(position, vec2(length, thickness), rotation), &shader_data);

    float32 radian = rotation * PI_FLOAT32 / 180.0;
    float32 cosx   = (float32)cosf(radian) * (length / 2.0f);
    float32 sinx   = (float32)sinf(radian) * (length / 2.0f);
    position.x += cosx;
    position.y += sinx;
    ShaderDataBasic arrow_shader = {.color = color_v4(color)};
    r_draw_single(arrow_key, transform_quad(position, vec2(thickness * 2, thickness * 2), rotation - 90), &arrow_shader);
}

internal void
draw_triangle(Vec2 position, float32 rotation, Color color, float32 size)
{
    RenderKey       key         = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->triangle, d_state->material_basic);
    ShaderDataBasic shader_data = {.color = color_v4(color)};
    r_draw_single(key, transform_quad(position, vec2(size, size), rotation), &shader_data);
}

internal Rect
draw_rect_rotated(Rect rect, float32 rotation, Color color)
{
    RenderKey key = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_rounded_rect);

    Mat4 model = rotation == 0 ? transform_quad_aligned(rect.center, rect.size)
                               : transform_quad(rect.center, rect.size, rotation);

    ShaderDataRectRounded shader_data = {0};
    shader_data.color                 = color_v4(color);
    shader_data.edge_color            = color_v4(color);
    shader_data.round                 = vec4(1, 1, 1, 1);
    shader_data.scale                 = rect.size;
    shader_data.softness              = 4;
    shader_data.edge_thickness        = 0;
    r_draw_single(key, model, &shader_data);
    return rect;
}

internal Rect
draw_rect(Rect rect, Color color)
{
    return draw_rect_rotated(rect, 0, color);
}

internal Rect
draw_rect_outline(Rect rect, Color color, float32 thickness)
{
    RenderKey key = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_rounded_rect);

    Mat4 model = transform_quad_aligned(rect.center, rect.size);

    ShaderDataRectRounded shader_data = {0};
    shader_data.color                 = d_color_none;
    shader_data.edge_color            = color_v4(color);
    shader_data.round                 = vec4(1, 1, 1, 1);
    shader_data.scale                 = rect.size;
    shader_data.softness              = 2;
    shader_data.edge_thickness        = thickness;
    r_draw_single(key, model, &shader_data);
    return rect;
}

internal void
draw_texture_aligned(Vec2 pos, Vec2 scale, TextureIndex texture)
{
    RenderKey              key          = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, texture, g_renderer->quad, d_state->material_basic_texture);
    ShaderDataBasicTexture uniform_data = (ShaderDataBasicTexture){0};
    r_draw_single(key, transform_quad_aligned(pos, scale), &uniform_data);
}

internal void
draw_bounds(float32 left, float32 right, float32 bottom, float32 top, Color color, float32 thickness)
{
    RenderKey key   = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_rounded_rect);
    Rect      r     = rect_from_xy_wh((right + left) / 2, (top + bottom) / 2, right - left, top - bottom);
    Mat4      model = transform_quad_aligned(r.center, r.size);

    ShaderDataRectRounded shader_data = {0};
    shader_data.color                 = d_color_none;
    shader_data.round                 = vec4(1, 1, 1, 1);
    shader_data.scale                 = r.size;
    shader_data.softness              = 1;
    shader_data.edge_color            = color_v4(color);
    shader_data.edge_thickness        = thickness;
    r_draw_single(key, model, &shader_data);
}

internal Rect
draw_text_at_internal(String str, Vec2 pos, Alignment alignment, float32 size, Color color)
{
    if (str.length <= 0)
        return rect_from_wh(0, 0);

    pos.y += d_default_text_baseline;
    ShaderDataText shader_data    = {0};
    shader_data.color             = color_v4(color);
    shader_data.outline_color     = d_state->ctx->font_style.outline_color;
    shader_data.thickness         = d_state->ctx->font_style.thickness;
    shader_data.softness          = d_state->ctx->font_style.softness;
    shader_data.outline_thickness = d_state->ctx->font_style.outline_thickness;

    GlyphAtlas*     atlas              = font_get_atlas(d_state->ctx->font_face, size);
    MaterialIndex   material           = atlas->type == GlyphAtlasTypeFreeType ? d_state->material_text_free_type : d_state->material_text_free_type_sdf;
    RenderKey       key                = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, atlas->texture, g_renderer->quad, material);
    R_Batch*        batch              = r_batch_from_key(key, str.length);
    Rect            bounds             = text_calculate_transforms(atlas, str, size, pos, alignment, batch->model_buffer, 0);
    ShaderDataText* shader_data_buffer = (ShaderDataText*)batch->uniform_buffer;
    for (uint32 i = 0; i < str.length; i++)
    {
        Glyph glyph              = glyph_get(atlas, str.value[i]);
        shader_data.glyph_bounds = glyph.atlas_bounds.v;
        memcpy(&shader_data_buffer[i], &shader_data, sizeof(ShaderDataText));
    }
    return bounds;
}

internal Rect
draw_text_at(String str, Vec2 pos, Alignment alignment, float32 size, Color color)
{
    return draw_text_at_internal(str, pos, alignment, size, color);
}

internal Rect
draw_text(String str, Rect rect, Anchor anchor, float32 size, Color color)
{
    if (str.length <= 0)
        return (Rect){0};

    Vec2 position = rect_get(rect, anchor.parent);
    position.y += d_default_text_baseline;
    GlyphAtlas*   atlas    = font_get_atlas(d_state->ctx->font_face, size);
    MaterialIndex material = atlas->type == GlyphAtlasTypeFreeType ? d_state->material_text_free_type : d_state->material_text_free_type_sdf;
    RenderKey     key      = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, atlas->texture, g_renderer->quad, material);
    R_Batch*      batch    = r_batch_from_key(key, str.length);
    Rect          result   = text_calculate_glyph_matrices(d_state->frame_arena, atlas, str, size, position, anchor.child, rect.w, batch->model_buffer, 0);

    ShaderDataText shader_data    = {0};
    shader_data.color             = color_v4(color);
    shader_data.outline_color     = d_state->ctx->font_style.outline_color;
    shader_data.thickness         = d_state->ctx->font_style.thickness;
    shader_data.softness          = d_state->ctx->font_style.softness;
    shader_data.outline_thickness = d_state->ctx->font_style.outline_thickness;

    ShaderDataText* shader_data_buffer = (ShaderDataText*)batch->uniform_buffer;
    for (uint32 i = 0; i < str.length; i++)
    {
        Glyph glyph              = glyph_get(atlas, str.value[i]);
        shader_data.glyph_bounds = glyph.atlas_bounds.v;
        memcpy(&shader_data_buffer[i], &shader_data, sizeof(ShaderDataText));
    }

    return result;
}

internal void
draw_circle(Vec2 pos, float32 radius, Color color)
{
    RenderKey key = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_circle_instanced);

    ShaderDataCircle shader_data = {0};
    shader_data.color            = color_v4(color);
    shader_data.fill_ratio       = 0;
    shader_data.slice_ratio      = 1;
    r_draw_single(key, transform_quad_aligned(pos, vec2(radius, radius)), &shader_data);
}

internal void
draw_circle_filled(Vec2 pos, float32 radius, Color color)
{
    RenderKey key = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_circle_instanced);

    ShaderDataCircle shader_data = {0};
    shader_data.color            = color_v4(color);
    shader_data.fill_ratio       = 1.1;
    shader_data.slice_ratio      = 1.1;
    r_draw_single(key, transform_quad_aligned(pos, vec2(radius, radius)), &shader_data);
}

internal void
draw_circle_partially_filled(Vec2 position, float32 rotation, float32 radius, Color color, float32 min_angle, float32 max_angle)
{
    RenderKey key = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_circle_instanced);

    float32 percentage = (max_angle - min_angle) / 360.0f;
    rotation += (max_angle + min_angle) / 2;

    ShaderDataCircle shader_data = {0};
    shader_data.color            = color_v4(color);
    shader_data.fill_ratio       = 1;
    shader_data.slice_ratio      = percentage;
    r_draw_single(key, transform_quad(position, vec2(radius, radius), rotation), &shader_data);
}

internal void
draw_sprite_colored(Vec2 position, float32 scale, float32 rotation, SpriteIndex sprite, Vec2 flip, Color color, float32 alpha)
{
    xassert(d_state->sprite_atlas, "`d_state->sprite_atlas` is null. Please activate atlas by calling `draw_context_activate_sprite_atlas` before calling sprite draw functions.");

    RenderKey key         = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, d_state->sprite_atlas->texture, g_renderer->quad, d_state->material_sprite);
    Sprite    sprite_data = d_state->sprite_atlas->sprites[sprite];

    Vec2 pivot = sprite_get_pivot(sprite_data, vec2(scale, scale), flip);

    Mat4 model = transform_quad_around_pivot(position, mul_vec2_f32(vec2(sprite_data.size.w * flip.x, sprite_data.size.h * flip.y), scale), rotation, pivot);

    ShaderDataSprite shader_data    = {0};
    shader_data.sprite_index        = sprite;
    shader_data.texture_layer_index = d_state->sprite_atlas->sprite_texture_indices[sprite];
    shader_data.alpha               = alpha;
    shader_data.color               = color_v4(color);

    r_draw_single(key, model, &shader_data);
}

internal void
draw_sprite_edges(Rect rect, SpriteIndex sprite, uint32 protection)
{
    xassert(d_state->sprite_atlas, "`d_state->sprite_atlas` is null. Please activate atlas by calling `draw_context_activate_sprite_atlas` before calling sprite draw functions.");

    RenderKey key   = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, d_state->sprite_atlas->texture, g_renderer->quad, d_state->material_sprite_border);
    Mat4      model = transform_quad_aligned(rect.center, rect.size);

    ShaderDataSpriteBorder shader_data = {0};
    shader_data.sprite_index           = sprite;
    shader_data.texture_layer_index    = d_state->sprite_atlas->sprite_texture_indices[sprite];
    shader_data.size                   = rect.size;
    shader_data.color                  = d_color_none;
    shader_data.protection             = protection;

    r_draw_single(key, model, &shader_data);
}

internal void
draw_sprite_colored_ignore_pivot(Vec2 position, float32 scale, SpriteIndex sprite, Vec2 flip, Color color, float32 alpha)
{
    xassert(d_state->sprite_atlas, "`d_state->sprite_atlas` is null. Please activate atlas by calling `draw_context_activate_sprite_atlas` before calling sprite draw functions.");

    RenderKey key = render_key_new_default(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, d_state->sprite_atlas->texture, g_renderer->quad, d_state->material_sprite);

    Sprite sprite_data = d_state->sprite_atlas->sprites[sprite];
    Mat4   model       = transform_quad_aligned(position, mul_vec2_f32(vec2(sprite_data.size.w * flip.x, sprite_data.size.h * flip.y), scale));

    ShaderDataSprite shader_data    = {0};
    shader_data.sprite_index        = sprite;
    shader_data.texture_layer_index = d_state->sprite_atlas->sprite_texture_indices[sprite];
    shader_data.alpha               = alpha;
    shader_data.color               = color_v4(color);

    r_draw_single(key, model, &shader_data);
}

internal void
draw_sprite(Vec2 position, float32 scale, float32 rotation, SpriteIndex sprite, Vec2 flip)
{
    draw_sprite_colored(position, scale, rotation, sprite, flip, ColorInvisible, 1);
}

/** context push */
internal void
draw_activate_font(FontFaceIndex font_face)
{
    xassert(d_state->ctx, "draw_context is null");
    d_state->ctx->font_face = font_face;
}

internal void
draw_context_push(SortLayerIndex sort_layer, ViewType view_type, PassIndex pass)
{
    DrawContextNode* node = d_state->free_nodes;
    stack_pop(d_state->free_nodes);
    if (!node)
        node = arena_push_struct_zero(d_state->persistent_arena, DrawContextNode);

    node->sort_layer = sort_layer;
    node->view       = view_type;
    node->pass       = pass;
    node->font_face  = d_state->ctx ? d_state->ctx->font_face : 0;
    node->font_style = d_state->ctx ? d_state->ctx->font_style : (DrawStyleFont){0};
    stack_push(d_state->ctx, node);
}

internal void
draw_context_pop()
{
    xassert(d_state->ctx, "draw_context is already null, can't pop!");
    DrawContextNode* node = d_state->ctx;
    stack_pop(d_state->ctx);
    stack_push(d_state->free_nodes, node);
}

/** extra draw functions */
internal Rect
draw_debug_rect(Rect rect)
{
    return draw_rect_outline(rect, ColorRed500, 4);
}

internal Rect
draw_debug_rect_b(Rect rect)
{
    return draw_rect_outline(rect, ColorGreen500, 4);
}

internal Rect
draw_sprite_rect_colored(Rect rect, SpriteIndex sprite, Anchor anchor, Color color, float32 alpha)
{
    Rect r      = sprite_rect(sprite);
    Rect result = rect_anchor(r, rect, anchor);
    draw_sprite_colored_ignore_pivot(result.center, 1, sprite, vec2(1, 1), color, alpha);
    return result;
}

internal Rect
draw_sprite_rect(Rect rect, SpriteIndex sprite, Anchor anchor)
{
    return draw_sprite_rect_colored(rect, sprite, anchor, ColorInvisible, 1);
}

internal Rect
draw_sprite_rect_cut_colored(Rect* rect, CutSide side, SpriteIndex sprite, Anchor anchor, Color color, float32 alpha)
{
    Rect r         = sprite_rect(sprite);
    Rect container = rect_cut_r(rect, r, side);
    Rect result    = rect_anchor(r, container, anchor);
    draw_sprite_colored_ignore_pivot(result.center, 1, sprite, vec2(1, 1), color, alpha);
    return result;
}

internal Rect
draw_sprite_rect_cut(Rect* rect, CutSide side, SpriteIndex sprite, Anchor anchor)
{
    return draw_sprite_rect_cut_colored(rect, side, sprite, anchor, ColorInvisible, 1);
}

internal Rect
draw_sprite_rect_flipped(Rect* rect, CutSide side, SpriteIndex sprite, Anchor anchor)
{
    Rect r         = sprite_rect(sprite);
    Rect container = rect_cut_r(rect, r, side);
    Rect result    = rect_anchor(r, container, anchor);
    draw_sprite_colored_ignore_pivot(result.center, 1, sprite, vec2(-1, 1), ColorInvisible, 1);
    return container;
}

internal Rect
sprite_rect(SpriteIndex sprite)
{
    const Sprite* s = &d_state->sprite_atlas->sprites[sprite];
    // NOTE(selim): -2 is removed because by default all our sprites have 1 px padding on each side
    return rect_from_wh(s->size.w - 2, s->size.h - 2);
}

internal Rect
sprite_rect_with_pivot(SpriteIndex sprite, Vec2 position, Vec2 flip, float32 scale_multiplier)
{
    Sprite sprite_data   = d_state->sprite_atlas->sprites[sprite];
    Vec2   pivot         = sprite_get_pivot(sprite_data, vec2(scale_multiplier, scale_multiplier), flip);
    Vec2   scale         = mul_vec2_f32(vec2(sprite_data.size.w * flip.x, sprite_data.size.h * flip.y), scale_multiplier);
    Vec2   rect_position = add_vec2(position, pivot);
    return rect_at(rect_position, fabs_vec2(scale), AlignmentCenter);
}

/** TODO(selim): This needs to be a generic solution. */
internal void
render_sprites_sorted(Arena* frame_arena, PassIndex pass, SpriteRenderRequest* requests, uint64 count, int32* layer_entity_counts)
{
    if (count <= 0)
        return;

    SpriteRenderRequestBuffer* request_buffers = arena_push_array_zero(frame_arena, SpriteRenderRequestBuffer, SORTING_LAYER_CAPACITY);
    for (int32 i = 0; i < SORTING_LAYER_CAPACITY; i++)
    {
        request_buffers[i].arr   = arena_push_array(frame_arena, SpriteRenderRequest, layer_entity_counts[i]);
        request_buffers[i].count = 0;
    }

    for (uint32 i = 0; i < count; i++)
    {
        SpriteRenderRequest        request = requests[i];
        SpriteRenderRequestBuffer* buffer  = &request_buffers[request.layer];
        buffer->arr[buffer->count]         = request;
        buffer->count += 1;
    }

    for (int32 i = 0; i < SORTING_LAYER_CAPACITY; i++)
    {
        SpriteRenderRequestBuffer* buffer = &request_buffers[i];
        if (buffer->count == 0)
            continue;

        qsort(buffer->arr, buffer->count, sizeof(SpriteRenderRequest), qsort_compare_render_requests_descending);

        RenderKey key   = render_key_new_default(ViewTypeWorld, i, pass, d_state->sprite_atlas->texture, g_renderer->quad, d_state->material_sprite);
        R_Batch*  batch = r_batch_from_key(key, buffer->count);

        Mat4*             model_buffer   = batch->model_buffer;
        ShaderDataSprite* uniform_buffer = (ShaderDataSprite*)batch->uniform_buffer;
        for (int32 i = 0; i < buffer->count; i++)
        {
            SpriteRenderRequest request = buffer->arr[i];

            Sprite sprite = d_state->sprite_atlas->sprites[request.sprite];
            Vec2   pivot  = sprite_get_pivot(sprite, request.scale, vec2_one());
            Vec2   scale  = vec2(request.scale.x * sprite.size.w, request.scale.y * sprite.size.h);
            Vec3   pos    = request.position;
            pos.y += request.position.z * 0.7;

            model_buffer[i] = transform_quad_around_pivot(pos.xy, scale, request.rotation, pivot);

            uniform_buffer[i].sprite_index        = request.sprite;
            uniform_buffer[i].texture_layer_index = d_state->sprite_atlas->sprite_texture_indices[request.sprite];
            uniform_buffer[i].alpha               = 1;
            uniform_buffer[i].color               = color_v4(request.color);
        }
    }
}

internal int
qsort_compare_render_requests_descending(const void* p, const void* q)
{
    float32 x = (*(const SpriteRenderRequest*)p).sort_order;
    float32 y = (*(const SpriteRenderRequest*)q).sort_order;
    return (x < y)   ? 1
           : (x > y) ? -1
                     : 0;
}

internal Trail*
trail_new(Arena* arena, uint32 point_count)
{
    const uint32 max_trail_capacity = 1024;
    Trail*       result             = arena_push_struct_zero(arena, Trail);
    result->total_capacity          = max_trail_capacity;
    result->buffer                  = arena_push_array_zero(arena, Vec2, max_trail_capacity);
    result->current_capacity        = point_count;
    return result;
}

internal void
trail_push_position(Trail* trail, Vec2 position)
{
    trail->count                      = min(trail->count + 1, trail->current_capacity);
    trail->buffer[trail->start_index] = position; // TODO(selim): do we need to mod here?
    trail->start_index                = (trail->start_index + 1) % trail->current_capacity;
}

internal void
trail_draw(Trail* trail)
{
    ArenaTemp     temp        = scratch_begin(0, 0);
    VertexBuffer* vertex_data = draw_util_generate_trail_vertices_fast(temp.arena, trail->buffer, trail->count, trail->start_index, trail->width_start, trail->width_end);

    RenderKey    key                = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, d_state->sprite_atlas->texture, g_renderer->geometry_empty, d_state->material_basic_trail, RenderTypeTrail);
    R_BatchNode* batch_node         = arena_push_struct_zero(g_renderer->frame_arena, R_BatchNode);
    batch_node->v.key               = key;
    batch_node->v.element_count     = vertex_data->count;
    batch_node->v.uniform_data_size = sizeof(TrailVertexData) * vertex_data->count;

    TrailVertexData* vertices = arena_push_array(temp.arena, TrailVertexData, vertex_data->count);
    for (uint32 i = 0; i < vertex_data->count; i++)
    {
        vertices[i].pos   = vec4(vertex_data->v[i].x, vertex_data->v[i].y, 0, 1);
        vertices[i].color = color_v4(lerp_color(trail->color_end, trail->color_start, (float32)i / vertex_data->count));
    }
    batch_node->v.uniform_buffer = arena_push(g_renderer->frame_arena, sizeof(TrailVertexData) * vertex_data->count);
    memcpy((uint8*)batch_node->v.uniform_buffer, vertices, batch_node->v.uniform_data_size);

    Mat4 model                 = transform_quad(vec2(0, 0), vec2_one(), 0);
    batch_node->v.model_buffer = arena_push_array(g_renderer->frame_arena, Mat4, 1);
    memcpy(batch_node->v.model_buffer, &model, sizeof(Mat4) * 1);

    r_batch_commit(batch_node);

    scratch_end(temp);
}

internal void
trail_set_color(Trail* trail, Color start, Color end)
{
    trail->color_start = start;
    trail->color_end   = end;
}

internal void
trail_set_width(Trail* trail, float32 start, float32 end)
{
    trail->width_start = start;
    trail->width_end   = end;
}

// TODO(selim): Can we move this to vertex shader?
internal VertexBuffer*
draw_util_generate_trail_vertices_fast(Arena* arena, Vec2* points, uint32 point_count, uint32 start_index, float32 start_width, float32 end_width)
{
    // TODO(selim): generated vertices needs to be drawn with `GL_TRIANGLE_STRIP`
    // we need to be able to define it in the vertex buffer or tell that to the renderer somehow
    VertexBuffer* result = vertex_buffer_new(arena);
    if (point_count < 2)
        return result;

    Vec2 start   = points[(start_index) % point_count];
    Vec2 next    = points[(start_index + 1) % point_count];
    Vec2 heading = heading_to_vec2(start, next);
    Vec2 normal  = vec2(-heading.y, heading.x);

    vertex_buffer_push(result, move_vec2(start, normal, start_width));
    vertex_buffer_push(result, move_vec2(start, normal, -start_width));
    for (uint32 i = 1; i < point_count; i++)
    {
        float32 width = lerp_f32(end_width, start_width, (float32)i / point_count);
        int32   index = (start_index + i);

        Vec2 prev        = points[(index - 1) % point_count];
        Vec2 end         = points[index % point_count];
        Vec2 end_heading = heading_to_vec2(prev, end);
        Vec2 end_normal  = vec2(-end_heading.y, end_heading.x);
        vertex_buffer_push(result, move_vec2(end, end_normal, width));
        vertex_buffer_push(result, move_vec2(end, end_normal, -width));
    }
    return result;
}

internal VertexBuffer*
draw_util_generate_trail_vertices_slow(Arena* arena, Vec2* points, uint32 point_count, uint32 start_index, float32 trail_width)
{
    VertexBuffer* result = vertex_buffer_new(arena);
    if (point_count < 2)
        return result;

    Vec2 start   = points[(start_index) % point_count];
    Vec2 next    = points[(start_index + 1) % point_count];
    Vec2 heading = heading_to_vec2(start, next);
    Vec2 normal  = vec2(-heading.y, heading.x);

    vertex_buffer_push_strip(result, move_vec2(start, normal, trail_width));
    vertex_buffer_push_strip(result, move_vec2(start, normal, -trail_width));
    for (uint32 i = 1; i < point_count; i++)
    {
        int32 index = (start_index + i);

        Vec2 prev        = points[(index - 1) % point_count];
        Vec2 end         = points[(index) % point_count];
        Vec2 end_heading = heading_to_vec2(prev, end);
        Vec2 end_normal  = vec2(-end_heading.y, end_heading.x);
        vertex_buffer_push_strip(result, move_vec2(end, end_normal, trail_width));
        vertex_buffer_push_strip(result, move_vec2(end, end_normal, -trail_width));
    }

    // for (uint32 i = 0; i < point_count; i++)
    // {
    //     draw_circle_filled(points[i], 4, ColorWhite);
    // }

    // for (uint32 i = 2; i < point_count; i++)
    // {
    //     float32 width = trail_width;
    //     int32   index = i + start_index;

    //     Vec2 start  = points[(index - 2) % point_count];
    //     Vec2 middle = points[(index - 1) % point_count];
    //     Vec2 next   = points[(index) % point_count];

    //     Vec2 heading      = heading_to_vec2(start, middle);
    //     Vec2 heading_next = heading_to_vec2(next, middle);
    //     if (lensqr_vec2(heading) <= 0 || lensqr_vec2(heading_next) <= 0)
    //         continue;

    //     Vec2 normal_end = vec2(-heading_next.y, heading_next.x);

    //     Vec2 right     = result->v[result->count - 2];
    //     Vec2 left      = result->v[result->count - 1];
    //     Vec2 end_right = add_vec2(next, mul_vec2_f32(normal_end, width));
    //     Vec2 end_left  = add_vec2(next, mul_vec2_f32(normal_end, -width));

    //     const float32 joint_break_threshold = 0;
    //     float32       dot                   = dot_vec2(heading, heading_next);

    //     Rect info_rect = rect_at(add_vec2(middle, vec2(0, 10)), vec2(80, 40), AlignmentBottom);
    //     draw_text(string_pushf(d_state->frame_arena, "%.2f", dot), rect_cut_top(&info_rect, 10), ANCHOR_C_C, 7, ColorWhite);
    //     if (dot > joint_break_threshold)
    //     {
    //         Vec2 normal_start        = vec2(-heading.y, heading.x);
    //         result.v[result.count++] = add_vec2(middle, mul_vec2_f32(normal_start, width));
    //         result.v[result.count++] = add_vec2(middle, mul_vec2_f32(normal_start, -width));
    //         result.v[result.count++] = add_vec2(middle, mul_vec2_f32(normal_end, -width));
    //         result.v[result.count++] = add_vec2(middle, mul_vec2_f32(normal_end, width));
    //         // result.v[result.count++] = add_vec2(middle, mul_vec2_f32(normal_start, -width));
    //         // result.v[result.count++] = end_left;
    //         // result.v[result.count++] = end_right;
    //     }
    //     else
    //     {
    //         draw_heading(right, heading, ColorYellow600, 2);
    //         draw_heading(left, heading, ColorYellow600, 2);
    //         draw_heading(end_right, heading_next, ColorGreen400, 2);
    //         draw_heading(end_left, heading_next, ColorGreen400, 2);
    //         Vec2 intersection_right  = vec2_intersection_fast(right, heading, end_left, heading_next);
    //         Vec2 intersection_left   = vec2_intersection_fast(left, heading, end_right, heading_next);
    //         result.v[result.count++] = intersection_right;
    //         result.v[result.count++] = intersection_left;
    //     }
    // }

    // Vec2 right       = result.v[result.count - 2];
    // Vec2 left        = result.v[result.count - 1];

    // Vec2 prev        = points[((int32)start_index - 2) % point_count];
    // Vec2 end_heading = heading_to_vec2(end, prev);
    // Vec2 end_normal  = vec2(-end_heading.y, end_heading.x);
    // Vec2 end_right   = add_vec2(end, mul_vec2_f32(end_normal, trail_width));
    // Vec2 end_left    = add_vec2(end, mul_vec2_f32(end_normal, -trail_width));

    // result.v[result.count++] = end_left;
    // result.v[result.count++] = end_left;
    // result.v[result.count++] = left;
    // result.v[result.count++] = end_right;

    // for (uint32 i = 0; i < result.count; i++)
    // {
    //     Color c = i % 2 == 0 ? ColorWhite : ColorRed600;
    //     draw_circle_filled(result.v[i], 4, c);
    // }

    return result;
}