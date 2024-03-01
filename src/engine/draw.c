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

    d_state->renderer = renderer;
    d_state->camera   = &renderer->camera;

    d_state->material_basic = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\basic_instanced.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\basic_instanced.frag")),
        sizeof(ShaderDataBasic),
        true);

    d_state->material_line = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\line.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\line.frag")),
        sizeof(ShaderDataLine),
        false);

    d_state->material_basic_texture = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\basic_texture.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\basic_texture.frag")),
        sizeof(ShaderDataBasicTexture),
        false);

    d_state->material_triangle = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\triangle.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\triangle.frag")),
        sizeof(ShaderDataTriangle),
        false);

    d_state->material_rounded_rect = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\rect_rounded.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\rect_rounded.frag")),
        sizeof(ShaderDataRectRounded),
        false);

    d_state->material_boid = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\boid_instanced.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\boid_instanced.frag")),
        sizeof(ShaderDataBoid),
        true);

    d_state->material_circle = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\circle.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\circle.frag")),
        sizeof(ShaderDataCircle),
        false);

    d_state->material_circle_instanced = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\circle_instanced.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\circle_instanced.frag")),
        sizeof(ShaderDataCircle),
        true);

    d_state->material_text = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\text.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\text.frag")),
        sizeof(ShaderDataText),
        true);

    d_state->material_sprite = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\sprite.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\sprite.frag")),
        sizeof(ShaderDataSprite),
        false);

    d_state->material_sprite_border = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\sprite_border.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\sprite_border.frag")),
        sizeof(ShaderDataSpriteBorder),
        false);

    /* Fonts */
    TextureIndex font_texture = texture_new_from_file(renderer, string(ASSET_PATH "\\open_sans.png"), 0, 1);
    d_state->font_open_sans   = glyph_atlas_load(
        arena,
        &FONT_OPEN_SANS_ATLAS_INFO,
        FONT_OPEN_SANS_GLYPHS,
        countof(FONT_OPEN_SANS_GLYPHS),
        font_texture);

    log_debug("initialized draw context");
}

internal void
draw_context_activate_atlas(SpriteAtlas* atlas)
{
    d_state->sprite_atlas = atlas;
}

internal void
draw_line(Vec2 start, Vec2 end, Color color, float32 thickness)
{
    RenderKey      key         = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_line);
    ShaderDataLine shader_data = {.color = color_v4(color)};
    r_draw_single(key, transform_line(start, end, thickness), &shader_data);
}

internal void
draw_heading(Vec2 origin, Vec2 heading, Color color, float32 thickness)
{
    RenderKey      key         = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_line);
    ShaderDataLine shader_data = {.color = color_v4(color)};
    r_draw_single(key, transform_line(origin, add_vec2(origin, mul_vec2_f32(heading, 20)), thickness), &shader_data);
}

internal void
draw_line_fixed(Vec2 position, float32 length, float32 rotation, Color color, float32 thickness)
{
    xassert(length > 0, "line length needs to be larger than 0 for `transform_line_rotated`");
    RenderKey      key         = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_line);
    ShaderDataLine shader_data = {.color = color_v4(color)};
    r_draw_single(key, transform_line_rotated(position, length, rotation, thickness), &shader_data);
}

internal void
draw_arrow(Vec2 position, float32 length, float32 rotation, Color color, float32 thickness)
{
    xassert(length > 0, "line length needs to be larger than 0 for `transform_line_rotated`");
    RenderKey line_key  = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_line);
    RenderKey arrow_key = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->triangle, d_state->material_basic);

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
    RenderKey       key         = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->triangle, d_state->material_basic);
    ShaderDataBasic shader_data = {.color = color_v4(color)};
    r_draw_single(key, transform_quad(position, vec2(size, size), rotation), &shader_data);
}

internal Rect
draw_rect_rotated(Rect rect, float32 rotation, Color color)
{
    RenderKey key = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_rounded_rect);

    Mat4 model = rotation == 0 ? transform_quad_aligned(rect.center, rect.size)
                               : transform_quad(rect.center, rect.size, rotation);

    ShaderDataRectRounded shader_data = {0};
    shader_data.color                 = color_v4(color);
    shader_data.edge_color            = d_color_none;
    shader_data.round                 = vec4(1, 1, 1, 1);
    shader_data.scale                 = rect.size;
    shader_data.softness              = 1;
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
    RenderKey key = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_rounded_rect);

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
    RenderKey              key          = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, texture, g_renderer->quad, d_state->material_basic_texture);
    ShaderDataBasicTexture uniform_data = (ShaderDataBasicTexture){0};
    r_draw_single(key, transform_quad_aligned(pos, scale), &uniform_data);
}

internal void
draw_bounds(float32 left, float32 right, float32 bottom, float32 top, Color color, float32 thickness)
{
    RenderKey key   = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_rounded_rect);
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
draw_text_at_internal(String str, Vec2 pos, Alignment alignment, float32 size, Color color, Color outline_color)
{
    if (str.length <= 0)
        return rect_from_wh(0, 0);

    pos.y += d_default_text_baseline;
    ShaderDataText shader_data    = {0};
    shader_data.color             = color_v4(color);
    shader_data.outline_color     = color_v4(outline_color);
    shader_data.thickness         = d_default_text_thickness;
    shader_data.softness          = d_default_text_softness;
    shader_data.outline_thickness = d_default_text_outline_thickness;

    RenderKey       key                = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, d_state->font_open_sans->texture, g_renderer->quad, d_state->material_text);
    R_Batch*        batch              = r_batch_from_key(key, str.length);
    Rect            bounds             = text_calculate_transforms(d_state->font_open_sans, str, size, pos, alignment, batch->model_buffer, 0);
    ShaderDataText* shader_data_buffer = (ShaderDataText*)batch->uniform_buffer;
    for (uint32 i = 0; i < str.length; i++)
    {
        Glyph glyph              = glyph_get(d_state->font_open_sans, str.value[i]);
        shader_data.glyph_bounds = glyph.atlas_bounds.v;
        memcpy(&shader_data_buffer[i], &shader_data, sizeof(ShaderDataText));
    }
    return bounds;
}

internal Rect
draw_text_at(String str, Vec2 pos, Alignment alignment, float32 size, Color color)
{
    return draw_text_at_internal(str, pos, alignment, size, color, ColorBlack);
}

internal Rect
draw_text(String str, Rect rect, Anchor anchor, float32 size, Color color)
{
    if (str.length <= 0)
        return (Rect){0};

    Vec2 position = rect_get(rect, anchor.parent);
    position.y += d_default_text_baseline;
    RenderKey key = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, d_state->font_open_sans->texture, g_renderer->quad, d_state->material_text);

    R_Batch* batch  = r_batch_from_key(key, str.length);
    Rect     result = text_calculate_glyph_matrices(d_state->frame_arena, d_state->font_open_sans, str, size, position, anchor.child, rect.w, batch->model_buffer, 0);

    ShaderDataText shader_data    = {0};
    shader_data.color             = color_v4(color);
    shader_data.outline_color     = d_color_black;
    shader_data.thickness         = d_default_text_thickness;
    shader_data.softness          = d_default_text_softness;
    shader_data.outline_thickness = d_default_text_outline_thickness;

    ShaderDataText* shader_data_buffer = (ShaderDataText*)batch->uniform_buffer;
    for (uint32 i = 0; i < str.length; i++)
    {
        Glyph glyph              = glyph_get(d_state->font_open_sans, str.value[i]);
        shader_data.glyph_bounds = glyph.atlas_bounds.v;
        memcpy(&shader_data_buffer[i], &shader_data, sizeof(ShaderDataText));
    }

    return result;
}

internal void
draw_circle(Vec2 pos, float32 radius, Color color)
{
    RenderKey key = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_circle_instanced);

    ShaderDataCircle shader_data = {0};
    shader_data.color            = color_v4(color);
    shader_data.slice_ratio      = 1;
    r_draw_single(key, transform_quad_aligned(pos, vec2(radius, radius)), &shader_data);
}

internal void
draw_circle_filled(Vec2 pos, float32 radius, Color color)
{
    RenderKey key = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_circle_instanced);

    ShaderDataCircle shader_data = {0};
    shader_data.color            = color_v4(color);
    shader_data.fill_ratio       = 1.1;
    shader_data.slice_ratio      = 1.1;
    r_draw_single(key, transform_quad_aligned(pos, vec2(radius, radius)), &shader_data);
}

internal void
draw_circle_partially_filled(Vec2 position, float32 rotation, float32 radius, Color color, float32 min_angle, float32 max_angle)
{
    RenderKey key = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, TEXTURE_INDEX_NULL, g_renderer->quad, d_state->material_circle_instanced);

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

    RenderKey key         = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, d_state->sprite_atlas->texture, g_renderer->quad, d_state->material_sprite);
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

    RenderKey key   = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, d_state->sprite_atlas->texture, g_renderer->quad, d_state->material_sprite_border);
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

    RenderKey key = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, d_state->sprite_atlas->texture, g_renderer->quad, d_state->material_sprite);

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
draw_context_push(SortLayerIndex sort_layer, ViewType view_type, PassIndex pass)
{
    DrawContextNode* node = d_state->free_nodes;
    if (!node)
        node = arena_push_struct_zero(d_state->persistent_arena, DrawContextNode);

    node->sort_layer = sort_layer;
    node->view       = view_type;
    node->pass       = pass;
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