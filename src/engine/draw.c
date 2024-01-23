#include <core/asserts.h>
#include <core/math.h>
#include <core/strings.h>
#include <engine/color.h>
#include <engine/draw.h>
#include <engine/layout.h>
#include <gfx/base.h>
#include <gfx/sprite.h>
#include <gfx/utils.h>

#ifndef SHADER_PATH
#define SHADER_PATH "..\\src\\shaders"
#endif

#ifndef ASSET_PATH
#define ASSET_PATH "..\\assets"
#endif

internal void
draw_context_initialize(Arena* arena, Arena* temp_arena, Renderer* renderer)
{
    g_draw_context = arena_push_struct_zero(arena, DrawContext);

    g_draw_context->renderer          = renderer;
    g_draw_context->geometry_quad     = geometry_quad_create(renderer);
    g_draw_context->geometry_triangle = geometry_triangle_create(renderer);
    g_draw_context->camera            = &renderer->camera;

    g_draw_context->material_basic = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\basic_instanced.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\basic_instanced.frag")),
        sizeof(ShaderDataBasic),
        true);

    g_draw_context->material_line = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\line.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\line.frag")),
        sizeof(ShaderDataLine),
        false);

    g_draw_context->material_basic_texture = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\basic_texture.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\basic_texture.frag")),
        sizeof(ShaderDataBasicTexture),
        false);

    g_draw_context->material_triangle = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\triangle.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\triangle.frag")),
        sizeof(ShaderDataTriangle),
        false);

    g_draw_context->material_rounded_rect = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\rect_rounded.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\rect_rounded.frag")),
        sizeof(ShaderDataRectRounded),
        false);

    g_draw_context->material_boid = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\boid_instanced.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\boid_instanced.frag")),
        sizeof(ShaderDataBoid),
        true);

    g_draw_context->material_circle = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\circle.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\circle.frag")),
        sizeof(ShaderDataCircle),
        false);

    g_draw_context->material_circle_instanced = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\circle_instanced.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\circle_instanced.frag")),
        sizeof(ShaderDataCircle),
        true);

    g_draw_context->material_text = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\text.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\text.frag")),
        sizeof(ShaderDataText),
        true);

    g_draw_context->material_sprite = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\sprite.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\sprite.frag")),
        sizeof(ShaderDataSprite),
        false);

    /* Fonts */
    TextureIndex font_texture      = texture_new_from_file(renderer, string(ASSET_PATH "\\open_sans.png"), 0, 1);
    g_draw_context->font_open_sans = glyph_atlas_load(
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
    g_draw_context->sprite_atlas = atlas;
}

internal void
draw_line(Vec2 start, Vec2 end, Color color, float32 thickness, SortLayerIndex layer)
{
    DrawBuffer draw_buffer                                      = renderer_buffer_request(g_draw_context->renderer, ViewTypeWorld, layer, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, g_draw_context->geometry_quad, g_draw_context->material_line, 1);
    draw_buffer.model_buffer[0]                                 = transform_line(start, end, thickness);
    ((ShaderDataLine*)draw_buffer.uniform_data_buffer)[0].color = color_to_vec4(color);
}

internal void
draw_line_fixed(Vec2 position, float32 length, float32 rotation, Color color, float32 thickness, SortLayerIndex layer)
{
    xassert(length > 0, "Line length needs to be larger than 0 for `transform_line_rotated`");
    DrawBuffer draw_buffer                                      = renderer_buffer_request(g_draw_context->renderer, ViewTypeWorld, layer, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, g_draw_context->geometry_quad, g_draw_context->material_line, 1);
    draw_buffer.model_buffer[0]                                 = transform_line_rotated(position, length, rotation, thickness);
    ((ShaderDataLine*)draw_buffer.uniform_data_buffer)[0].color = color_to_vec4(color);
}

internal void
draw_arrow(Vec2 position, float32 length, float32 angle, Color color, float32 thickness, SortLayerIndex layer)
{
    xassert(length > 0, "Line length needs to be larger than 0 for `transform_line_rotated`");
    DrawBuffer draw_buffer = renderer_buffer_request(g_draw_context->renderer, ViewTypeWorld, layer, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, g_draw_context->geometry_quad, g_draw_context->material_line, 1);

    float32 radian = angle * PI_FLOAT32 / 180.0;
    float32 cosx   = (float32)cosf(radian) * (length / 2.0f);
    float32 sinx   = (float32)sinf(radian) * (length / 2.0f);
    position.x += cosx;
    position.y += sinx;
    Mat4 line = transform_quad(position, vec2(length, thickness), angle);
    position.x += cosx;
    position.y += sinx;
    Mat4 arrow                                                        = transform_quad(position, vec2(thickness * 2, thickness * 2), angle - 90);
    draw_buffer.model_buffer[0]                                       = line;
    ((ShaderDataLine*)draw_buffer.uniform_data_buffer)[0].color       = color_to_vec4(color);
    DrawBuffer draw_buffer_arrow                                      = renderer_buffer_request(g_draw_context->renderer, ViewTypeWorld, layer, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, g_draw_context->geometry_triangle, g_draw_context->material_basic, 1);
    draw_buffer_arrow.model_buffer[0]                                 = arrow;
    ((ShaderDataLine*)draw_buffer_arrow.uniform_data_buffer)[0].color = color_to_vec4(color);
}

internal void
draw_triangle(Vec2 position, float32 rotation, Color color, float32 size, SortLayerIndex sort_index)
{
    DrawBuffer draw_buffer                                       = renderer_buffer_request(g_draw_context->renderer, ViewTypeWorld, sort_index, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, g_draw_context->geometry_triangle, g_draw_context->material_basic, 1);
    draw_buffer.model_buffer[0]                                  = transform_quad(position, vec2(size, size), rotation);
    ((ShaderDataBasic*)draw_buffer.uniform_data_buffer)[0].color = color_to_vec4(color);
}

internal Rect
draw_rect(Rect rect, float32 rotation, SortLayerIndex sort_index, ViewType view_type, StyleRect style)
{
    DrawBuffer draw_buffer = renderer_buffer_request(g_draw_context->renderer, view_type, sort_index, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, g_draw_context->geometry_quad, g_draw_context->material_rounded_rect, 1);
    // transform_quad_aligned is much faster so if there is no need for rotation, use aligned
    if (rotation == 0)
        draw_buffer.model_buffer[0] = transform_quad_aligned(vec3_xy_z(rect.center, 0), rect.size);
    else
        draw_buffer.model_buffer[0] = transform_quad(rect.center, rect.size, rotation);

    ShaderDataRectRounded* uniform_buffer = (ShaderDataRectRounded*)draw_buffer.uniform_data_buffer;
    uniform_buffer[0].color               = style.color;
    uniform_buffer[0].edge_color          = style.border_color;
    uniform_buffer[0].round               = style.border_radius.v;
    uniform_buffer[0].scale               = rect.size;
    uniform_buffer[0].softness            = style.softness;
    uniform_buffer[0].edge_thickness      = style.border_thickness;

    return rect;
}

internal void
draw_debug_line(Vec2 start, Vec2 end, Color color, SortLayerIndex layer)
{
    draw_line(start, end, color, 4 * g_draw_context->renderer->ppu, layer);
}

internal void
draw_texture_aligned(Vec3 pos, Vec2 scale, TextureIndex texture, SortLayerIndex layer)
{
    DrawBuffer             draw_buffer  = renderer_buffer_request(g_draw_context->renderer, ViewTypeWorld, layer, FRAME_BUFFER_INDEX_DEFAULT, texture, g_draw_context->geometry_quad, g_draw_context->material_basic_texture, 1);
    ShaderDataBasicTexture uniform_data = (ShaderDataBasicTexture){0};
    draw_buffer_insert(&draw_buffer, transform_quad_aligned(pos, scale), &uniform_data);
}

internal void
draw_bounds(float32 left, float32 right, float32 bottom, float32 top, Color color, float32 thickness, SortLayerIndex layer)
{
    DrawBuffer draw_buffer      = renderer_buffer_request(g_draw_context->renderer, ViewTypeWorld, layer, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, g_draw_context->geometry_quad, g_draw_context->material_line, 4);
    draw_buffer.model_buffer[0] = transform_line(vec2(left, top), vec2(left, bottom), thickness);
    draw_buffer.model_buffer[1] = transform_line(vec2(left, bottom), vec2(right, bottom), thickness);
    draw_buffer.model_buffer[2] = transform_line(vec2(right, bottom), vec2(right, top), thickness);
    draw_buffer.model_buffer[3] = transform_line(vec2(right, top), vec2(left, top), thickness);

    Vec4            color_vec4     = color_to_vec4(color);
    ShaderDataLine* uniform_buffer = (ShaderDataLine*)draw_buffer.uniform_data_buffer;
    uniform_buffer[0].color        = color_vec4;
    uniform_buffer[1].color        = color_vec4;
    uniform_buffer[2].color        = color_vec4;
    uniform_buffer[3].color        = color_vec4;
}

internal Rect
draw_text_at(Vec2 pos, String str, Alignment alignment, StyleText style, ViewType view_type, SortLayerIndex layer)
{
    if (str.length <= 0)
        return rect_from_wh(0, 0);

    pos.y += style.base_line;
    ShaderDataText shader_data         = {0};
    shader_data.color                  = style.color;
    shader_data.outline_color          = style.outline_color;
    shader_data.thickness              = style.thickness;
    shader_data.softness               = style.softness;
    shader_data.outline_thickness      = style.outline_thickness;
    DrawBuffer      db                 = renderer_buffer_request(g_draw_context->renderer, view_type, layer, FRAME_BUFFER_INDEX_DEFAULT, g_draw_context->font_open_sans->texture, g_draw_context->geometry_quad, g_draw_context->material_text, str.length);
    Rect            bounds             = text_calculate_transforms(g_draw_context->font_open_sans, str, style.font_size, pos, alignment, db.model_buffer, 0);
    ShaderDataText* shader_data_buffer = (ShaderDataText*)db.uniform_data_buffer;
    for (int i = 0; i < str.length; i++)
    {
        Glyph glyph              = glyph_get(g_draw_context->font_open_sans, str.value[i]);
        shader_data.glyph_bounds = glyph.atlas_bounds.v;
        memcpy(&shader_data_buffer[i], &shader_data, sizeof(ShaderDataText));
    }
    return bounds;
}

internal void
draw_text(Rect rect, String str, Anchor anchor, StyleText style, ViewType view_type, SortLayerIndex layer)
{
    if (str.length <= 0)
        return;

    Vec2 position = rect_get(rect, anchor.parent);
    position.y += style.base_line;
    DrawBuffer db = renderer_buffer_request(g_draw_context->renderer, view_type, layer, FRAME_BUFFER_INDEX_DEFAULT, g_draw_context->font_open_sans->texture, g_draw_context->geometry_quad, g_draw_context->material_text, str.length);
    text_calculate_transforms_v2(g_draw_context->font_open_sans, str, style.font_size, position, anchor.child, rect.w, db.model_buffer, 0);

    ShaderDataText shader_data    = {0};
    shader_data.color             = style.color;
    shader_data.outline_color     = style.outline_color;
    shader_data.thickness         = style.thickness;
    shader_data.softness          = style.softness;
    shader_data.outline_thickness = style.outline_thickness;

    ShaderDataText* shader_data_buffer = (ShaderDataText*)db.uniform_data_buffer;
    for (int i = 0; i < str.length; i++)
    {
        Glyph glyph              = glyph_get(g_draw_context->font_open_sans, str.value[i]);
        shader_data.glyph_bounds = glyph.atlas_bounds.v;
        memcpy(&shader_data_buffer[i], &shader_data, sizeof(ShaderDataText));
    }
}

internal void
draw_circle(Vec2 position, float32 radius, Color color, SortLayerIndex layer)
{
    DrawBuffer draw_buffer         = renderer_buffer_request(g_draw_context->renderer, ViewTypeWorld, layer, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, g_draw_context->geometry_quad, g_draw_context->material_circle_instanced, 1);
    draw_buffer.model_buffer[0]    = transform_quad(position, vec2(radius, radius), 0);
    ShaderDataCircle* model_buffer = (ShaderDataCircle*)draw_buffer.uniform_data_buffer;
    model_buffer[0].color          = color_to_vec4(color);
    model_buffer[0].slice_ratio    = 1;
}

internal void
draw_circle_filled(Circle circle, Color color, SortLayerIndex layer)
{
    DrawBuffer draw_buffer         = renderer_buffer_request(g_draw_context->renderer, ViewTypeWorld, layer, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, g_draw_context->geometry_quad, g_draw_context->material_circle_instanced, 1);
    draw_buffer.model_buffer[0]    = transform_quad(circle.center, vec2(circle.radius, circle.radius), 0);
    ShaderDataCircle* model_buffer = (ShaderDataCircle*)draw_buffer.uniform_data_buffer;
    model_buffer[0].color          = color_to_vec4(color);
    model_buffer[0].fill_ratio     = 1;
    model_buffer[0].slice_ratio    = 1;
}

internal void
draw_circle_partially_filled(Vec2 position, float32 rotation, float32 radius, Color color, float32 min_angle, float32 max_angle, SortLayerIndex layer)
{
    float32 percentage = (max_angle - min_angle) / 360.0f;
    rotation += (max_angle + min_angle) / 2;
    DrawBuffer draw_buffer         = renderer_buffer_request(g_draw_context->renderer, ViewTypeWorld, layer, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, g_draw_context->geometry_quad, g_draw_context->material_circle_instanced, 1);
    draw_buffer.model_buffer[0]    = transform_quad(position, vec2(radius, radius), rotation);
    ShaderDataCircle* model_buffer = (ShaderDataCircle*)draw_buffer.uniform_data_buffer;
    model_buffer[0].color          = color_to_vec4(color);
    model_buffer[0].fill_ratio     = 1;
    model_buffer[0].slice_ratio    = percentage;
}

internal void
draw_boid(Vec2 position, float32 rotation, float32 size, Color color, SortLayerIndex layer)
{
    DrawBuffer draw_buffer      = renderer_buffer_request(g_draw_context->renderer, ViewTypeWorld, layer, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, g_draw_context->geometry_quad, g_draw_context->material_boid, 1);
    draw_buffer.model_buffer[0] = transform_quad(position, vec2(size, size * 1.4), 90 + rotation);

    ((ShaderDataBoid*)draw_buffer.uniform_data_buffer)[0].color = color_to_vec4(color);
}

internal void
draw_sprite_colored(Vec2 position, float32 scale, float32 rotation, SpriteIndex sprite, Vec2 flip, ViewType view_type, SortLayerIndex layer, Color color)
{
    xassert(g_draw_context->sprite_atlas, "`g_draw_context->sprite_atlas` is null. Please activate atlas by calling `draw_context_activate_sprite_atlas` before calling sprite draw functions.");
    DrawBuffer draw_buffer = renderer_buffer_request(g_draw_context->renderer, view_type, layer, FRAME_BUFFER_INDEX_DEFAULT, g_draw_context->sprite_atlas->texture, g_draw_context->geometry_quad, g_draw_context->material_sprite, 1);
    Sprite     sprite_data = g_draw_context->sprite_atlas->sprites[sprite];

    Vec2 pivot                     = sprite_get_pivot(sprite_data, vec2(scale, scale), vec2_one());
    draw_buffer.model_buffer[0]    = transform_quad_around_pivot(position, mul_vec2_f32(vec2(sprite_data.size.w * flip.x, sprite_data.size.h * flip.y), scale), rotation, pivot);
    ShaderDataSprite* model_buffer = (ShaderDataSprite*)draw_buffer.uniform_data_buffer;

    model_buffer[0].sprite_index        = sprite;
    model_buffer[0].texture_layer_index = g_draw_context->sprite_atlas->sprite_texture_indices[sprite];
    model_buffer[0].alpha               = 1;
    model_buffer[0].color               = color_to_vec4(color);
}

internal void
draw_sprite(Vec2 position, float32 scale, float32 rotation, SpriteIndex sprite, Vec2 flip, ViewType view_type, SortLayerIndex layer)
{
    draw_sprite_colored(position, scale, rotation, sprite, flip, view_type, layer, ColorInvisible);
}

internal float32
screen_top()
{
    return g_draw_context->camera->world_height / 2.0f;
}

internal float32
screen_left()
{
    return -g_draw_context->camera->world_width / 2.0f;
}

internal float32
screen_right()
{
    return g_draw_context->camera->world_width / 2.0f;
}

internal float32
screen_bottom()
{
    return -g_draw_context->camera->world_height / 2.0f;
}

internal float32
screen_height()
{
    return g_draw_context->camera->world_height;
}

internal float32
screen_width()
{
    return g_draw_context->camera->world_width;
}
