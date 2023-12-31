#include <core/asserts.h>
#include <core/math.h>
#include <core/strings.h>
#include <engine/color.h>
#include <engine/draw.h>
#include <gfx/base.h>
#include <gfx/sprite.h>
#include <gfx/utils.h>

#ifndef SHADER_PATH
#define SHADER_PATH "..\\src\\shaders"
#endif

#ifndef ASSET_PATH
#define ASSET_PATH "..\\assets"
#endif

internal DrawContext*
draw_context_new(Arena* arena, Arena* temp_arena, Renderer* renderer)
{
    DrawContext* draw_context = arena_push_struct_zero(arena, DrawContext);

    draw_context->renderer          = renderer;
    draw_context->geometry_quad     = geometry_quad_create(renderer);
    draw_context->geometry_triangle = geometry_triangle_create(renderer);

    draw_context->material_basic = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\basic_instanced.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\basic_instanced.frag")),
        sizeof(ShaderDataBasic),
        true);

    draw_context->material_line = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\line.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\line.frag")),
        sizeof(ShaderDataLine),
        false);

    draw_context->material_basic_texture = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\basic_texture.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\basic_texture.frag")),
        sizeof(ShaderDataBasicTexture),
        false);

    draw_context->material_triangle = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\triangle.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\triangle.frag")),
        sizeof(ShaderDataTriangle),
        false);

    draw_context->material_rounded_rect = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\rect_rounded.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\rect_rounded.frag")),
        sizeof(ShaderDataRectRounded),
        false);

    draw_context->material_boid = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\boid_instanced.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\boid_instanced.frag")),
        sizeof(ShaderDataBoid),
        true);

    draw_context->material_circle = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\circle.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\circle.frag")),
        sizeof(ShaderDataCircle),
        false);

    draw_context->material_circle_instanced = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\circle_instanced.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\circle_instanced.frag")),
        sizeof(ShaderDataCircle),
        true);

    draw_context->material_text = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\text.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\text.frag")),
        sizeof(ShaderDataText),
        true);

    draw_context->material_sprite = material_new(
        renderer,
        file_read_all_as_string(arena, string(SHADER_PATH "\\sprite.vert")),
        file_read_all_as_string(arena, string(SHADER_PATH "\\sprite.frag")),
        sizeof(ShaderDataSprite),
        false);

    /* Fonts */
    TextureIndex font_texture    = texture_new_from_file(renderer, string(ASSET_PATH "\\open_sans.png"), 0, 1);
    draw_context->font_open_sans = glyph_atlas_load(
        arena,
        &FONT_OPEN_SANS_ATLAS_INFO,
        FONT_OPEN_SANS_GLYPHS,
        countof(FONT_OPEN_SANS_GLYPHS),
        font_texture);

    log_debug("initialized draw context");
    return draw_context;
}

internal void
draw_context_activate_atlas(DrawContext* context, SpriteAtlas* atlas)
{
    context->sprite_atlas = atlas;
}

internal void
draw_line(DrawContext* dc, Vec2 start, Vec2 end, Color color, float32 thickness)
{
    DrawBuffer draw_buffer                                      = renderer_buffer_request(dc->renderer, ViewTypeWorld, SORT_LAYER_INDEX_DEFAULT, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, dc->geometry_quad, dc->material_line, 1);
    draw_buffer.model_buffer[0]                                 = transform_line(start, end, thickness);
    ((ShaderDataLine*)draw_buffer.uniform_data_buffer)[0].color = color_to_vec4(color);
}

internal void
draw_line_fixed(DrawContext* dc, Vec2 position, float32 length, float32 rotation, Color color, float32 thickness)
{
    xassert(length > 0, "Line length needs to be larger than 0 for `transform_line_rotated`");
    DrawBuffer draw_buffer                                      = renderer_buffer_request(dc->renderer, ViewTypeWorld, SORT_LAYER_INDEX_DEFAULT, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, dc->geometry_quad, dc->material_line, 1);
    draw_buffer.model_buffer[0]                                 = transform_line_rotated(position, length, rotation, thickness);
    ((ShaderDataLine*)draw_buffer.uniform_data_buffer)[0].color = color_to_vec4(color);
}

internal void
draw_arrow(DrawContext* dc, Vec2 position, float32 length, float32 angle, Color color, float32 thickness)
{
    xassert(length > 0, "Line length needs to be larger than 0 for `transform_line_rotated`");
    DrawBuffer draw_buffer = renderer_buffer_request(dc->renderer, ViewTypeWorld, SORT_LAYER_INDEX_DEFAULT, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, dc->geometry_quad, dc->material_line, 1);

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
    DrawBuffer draw_buffer_arrow                                      = renderer_buffer_request(dc->renderer, ViewTypeWorld, SORT_LAYER_INDEX_DEFAULT - 1, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, dc->geometry_triangle, dc->material_basic, 1);
    draw_buffer_arrow.model_buffer[0]                                 = arrow;
    ((ShaderDataLine*)draw_buffer_arrow.uniform_data_buffer)[0].color = color_to_vec4(color);
}

internal void
draw_triangle(DrawContext* dc, Vec2 position, float32 rotation, Color color, float32 size, SortLayerIndex sort_index)
{
    DrawBuffer draw_buffer                                       = renderer_buffer_request(dc->renderer, ViewTypeWorld, sort_index, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, dc->geometry_triangle, dc->material_basic, 1);
    draw_buffer.model_buffer[0]                                  = transform_quad(position, vec2(size, size), rotation);
    ((ShaderDataBasic*)draw_buffer.uniform_data_buffer)[0].color = color_to_vec4(color);
}

internal Rect
draw_rect(DrawContext* dc, Rect rect, float32 rotation, SortLayerIndex sort_index, StyleRect style)
{
    DrawBuffer draw_buffer = renderer_buffer_request(dc->renderer, ViewTypeWorld, sort_index, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, dc->geometry_quad, dc->material_rounded_rect, 1);
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
draw_debug_line(DrawContext* dc, Vec2 start, Vec2 end, Color color)
{
    draw_line(dc, start, end, color, 4 * dc->renderer->ppu);
}

internal void
draw_texture_aligned(DrawContext* dc, Vec3 pos, Vec2 scale, TextureIndex texture)
{
    DrawBuffer             draw_buffer  = renderer_buffer_request(dc->renderer, ViewTypeWorld, SORT_LAYER_INDEX_DEFAULT, FRAME_BUFFER_INDEX_DEFAULT, texture, dc->geometry_quad, dc->material_basic_texture, 1);
    ShaderDataBasicTexture uniform_data = (ShaderDataBasicTexture){0};
    draw_buffer_insert(&draw_buffer, transform_quad_aligned(pos, scale), &uniform_data);
}

internal void
draw_bounds(DrawContext* dc, float32 left, float32 right, float32 bottom, float32 top, Color color, float32 thickness)
{
    DrawBuffer draw_buffer      = renderer_buffer_request(dc->renderer, ViewTypeWorld, SORT_LAYER_INDEX_DEFAULT, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, dc->geometry_quad, dc->material_line, 4);
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
draw_text(DrawContext* dc, Vec2 pos, String str, Alignment alignment, StyleText style)
{
    pos.y += style.base_line;
    ShaderDataText shader_data         = {0};
    shader_data.color                  = style.color;
    shader_data.outline_color          = style.outline_color;
    shader_data.thickness              = style.thickness;
    shader_data.softness               = style.softness;
    shader_data.outline_thickness      = style.outline_thickness;
    DrawBuffer      db                 = renderer_buffer_request(dc->renderer, ViewTypeWorld, SORT_LAYER_INDEX_DEFAULT, FRAME_BUFFER_INDEX_DEFAULT, dc->font_open_sans->texture, dc->geometry_quad, dc->material_text, str.length);
    Rect            bounds             = text_calculate_transforms(dc->font_open_sans, str, style.font_size, pos, alignment, db.model_buffer, 0);
    ShaderDataText* shader_data_buffer = (ShaderDataText*)db.uniform_data_buffer;
    for (int i = 0; i < str.length; i++)
    {
        Glyph glyph              = glyph_get(dc->font_open_sans, str.value[i]);
        shader_data.glyph_bounds = glyph.atlas_bounds.v;
        memcpy(&shader_data_buffer[i], &shader_data, sizeof(ShaderDataText));
    }
    return bounds;
}

internal void
draw_circle(DrawContext* dc, Vec2 position, float32 radius, Color color)
{
    DrawBuffer draw_buffer         = renderer_buffer_request(dc->renderer, ViewTypeWorld, SORT_LAYER_INDEX_DEFAULT, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, dc->geometry_quad, dc->material_circle_instanced, 1);
    draw_buffer.model_buffer[0]    = transform_quad(position, vec2(radius, radius), 0);
    ShaderDataCircle* model_buffer = (ShaderDataCircle*)draw_buffer.uniform_data_buffer;
    model_buffer[0].color          = color_to_vec4(color);
    model_buffer[0].slice_ratio    = 1;
}

internal void
draw_circle_filled(DrawContext* dc, Circle circle, Color color)
{
    DrawBuffer draw_buffer         = renderer_buffer_request(dc->renderer, ViewTypeWorld, SORT_LAYER_INDEX_DEFAULT + 2, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, dc->geometry_quad, dc->material_circle_instanced, 1);
    draw_buffer.model_buffer[0]    = transform_quad(circle.center, vec2(circle.radius, circle.radius), 0);
    ShaderDataCircle* model_buffer = (ShaderDataCircle*)draw_buffer.uniform_data_buffer;
    model_buffer[0].color          = color_to_vec4(color);
    model_buffer[0].fill_ratio     = 1;
    model_buffer[0].slice_ratio    = 1;
}

internal void
draw_circle_partially_filled(DrawContext* dc, Vec2 position, float32 rotation, float32 radius, Color color, float32 min_angle, float32 max_angle)
{
    float32 percentage = (max_angle - min_angle) / 360.0f;
    rotation += (max_angle + min_angle) / 2;
    DrawBuffer draw_buffer         = renderer_buffer_request(dc->renderer, ViewTypeWorld, SORT_LAYER_INDEX_DEFAULT, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, dc->geometry_quad, dc->material_circle_instanced, 1);
    draw_buffer.model_buffer[0]    = transform_quad(position, vec2(radius, radius), rotation);
    ShaderDataCircle* model_buffer = (ShaderDataCircle*)draw_buffer.uniform_data_buffer;
    model_buffer[0].color          = color_to_vec4(color);
    model_buffer[0].fill_ratio     = 1;
    model_buffer[0].slice_ratio    = percentage;
}

internal void
draw_boid(DrawContext* dc, Vec2 position, float32 rotation, float32 size, Color color)
{
    DrawBuffer draw_buffer      = renderer_buffer_request(dc->renderer, ViewTypeWorld, SORT_LAYER_INDEX_DEFAULT, FRAME_BUFFER_INDEX_DEFAULT, TEXTURE_INDEX_NULL, dc->geometry_quad, dc->material_boid, 1);
    draw_buffer.model_buffer[0] = transform_quad(position, vec2(size, size * 1.4), 90 + rotation);

    ((ShaderDataBoid*)draw_buffer.uniform_data_buffer)[0].color = color_to_vec4(color);
}

internal void
draw_sprite(DrawContext* dc, Vec2 position, float32 scale, SpriteIndex sprite)
{
    xassert(dc->sprite_atlas, "`dc->sprite_atlas` is null. Please activate atlas by calling `draw_context_activate_sprite_atlas` before calling sprite draw functions.");
    DrawBuffer draw_buffer = renderer_buffer_request(dc->renderer, ViewTypeWorld, SORT_LAYER_INDEX_DEFAULT, FRAME_BUFFER_INDEX_DEFAULT, dc->sprite_atlas->texture, dc->geometry_quad, dc->material_sprite, 1);
    Sprite     sprite_data = dc->sprite_atlas->sprites[sprite];

    Vec2 pivot                     = sprite_get_pivot(sprite_data, vec2(scale, scale), vec2_one());
    draw_buffer.model_buffer[0]    = transform_quad_around_pivot(position, mul_vec2_f32(vec2(sprite_data.size.w, sprite_data.size.h), scale), 0, pivot);
    ShaderDataSprite* model_buffer = (ShaderDataSprite*)draw_buffer.uniform_data_buffer;

    model_buffer[0].sprite_index        = sprite;
    model_buffer[0].texture_layer_index = dc->sprite_atlas->sprite_texture_indices[sprite];
    model_buffer[0].alpha               = 1;
    model_buffer[0].color               = color_to_vec4(ColorInvisible);
}