#include "text.h"
#include <engine/text.h>

internal Glyph
glyph_get(GlyphAtlas* atlas, char c)
{
    return atlas->glyphs[(int)c - 32];
}

internal float32
glyph_width(Glyph glyph, float32 size)
{
    return size * (glyph.plane_bounds.right - glyph.plane_bounds.left);
}

internal float32
glyph_height(Glyph glyph, float32 size)
{
    return size * (glyph.plane_bounds.top - glyph.plane_bounds.bottom);
}

internal GlyphAtlas*
glyph_atlas_load(Arena* arena, const GlyphAtlasInfo* atlas_info, const Glyph* glyphs, uint32 glyph_count, TextureIndex texture)
{
    GlyphAtlas* atlas  = arena_push_struct_zero(arena, GlyphAtlas);
    atlas->glyphs      = glyphs;
    atlas->glyph_count = glyph_count;
    atlas->atlas_info  = atlas_info;
    atlas->texture     = texture;
    return atlas;
}

internal Rect
text_calculate_bounds_deprecated(GlyphAtlas* atlas, Vec2 position, Alignment alignment, String str, float32 size)
{
    Vec2  string_size = vec2_zero();
    Glyph glyph;
    for (uint32 i = 0; i < str.length; i++)
    {
        glyph = glyph_get(atlas, str.value[i]);
        string_size.x += glyph.advance * size;
        float32 h     = glyph_height(glyph, size);
        string_size.y = max(h, string_size.y);
    };

    float32 x = string_size.x * AlignmentMultiplierX[alignment];
    float32 y = size * AlignmentMultiplierY[alignment];
    return (Rect){.x = x + position.x, .y = y + position.y, .w = string_size.x, .h = size};
}

// TODO(selim): Go over each of these functions at some point. Currently we calculate glyph bounds multiple times for each string.
internal Rect
text_calculate_bounds(GlyphAtlas* atlas, Vec2 position, Alignment alignment, String str, float32 size)
{
    Vec2  string_size = vec2_zero();
    Glyph glyph;
    for (uint32 i = 0; i < str.length; i++)
    {
        glyph = glyph_get(atlas, str.value[i]);
        string_size.x += glyph.advance * size;
        string_size.y = max(glyph.plane_bounds.top * size, string_size.y);
    };

    float32 x = string_size.x * AlignmentMultiplierX[alignment];
    float32 y = size * AlignmentMultiplierY[alignment];
    return (Rect){.x = x + position.x, .y = y + position.y, .w = string_size.x, .h = string_size.y};
}

internal Rect
text_calculate_transforms(GlyphAtlas* atlas, String str, float32 size_in_pixels, Vec2 position, Alignment alignment, Mat4* dst_matrices, uint32 dst_index)
{
    Rect string_bounds = text_calculate_bounds(atlas, position, alignment, str, size_in_pixels);
    Vec2 base_offset   = {
          .x = string_bounds.w * FontAlignmentMultiplierX[alignment],
          .y = string_bounds.h * FontAlignmentMultiplierY[alignment]};

    uint32 index = dst_index;
    for (uint32 i = 0; i < str.length; i++)
    {
        Glyph   glyph        = glyph_get(atlas, str.value[i]);
        float32 w            = glyph_width(glyph, size_in_pixels);
        float32 h            = glyph_height(glyph, size_in_pixels);
        Vec2    plane_offset = {.x = size_in_pixels * glyph.plane_bounds.left, .y = size_in_pixels * glyph.plane_bounds.bottom};
        float32 x            = position.x + base_offset.x + plane_offset.x + w / 2.0f;
        float32 y            = position.y + base_offset.y + plane_offset.y + h / 2.0f;

        Mat4 transform = transform_quad_aligned(vec3(x, y, 0), vec2(w, h));
        memcpy(&dst_matrices[index], &transform, sizeof(transform));
        base_offset.x += glyph.advance * size_in_pixels;
        index++;
    }

    return string_bounds;
}

internal Rect
text_calculate_transforms_v2(GlyphAtlas* atlas, String str, float32 size_in_pixels, Vec2 position, Alignment alignment, float32 max_width, Mat4* dst_matrices, uint32 dst_index)
{
    Rect string_bounds       = text_calculate_bounds(atlas, position, alignment, str, size_in_pixels);
    Vec2 initial_base_offset = {
        .x = string_bounds.w * FontAlignmentMultiplierX[alignment],
        .y = string_bounds.h * FontAlignmentMultiplierY[alignment]};
    Vec2 base_offset = initial_base_offset;

    uint32  current_line = 0;
    float32 line_height  = size_in_pixels;
    int32   space_index  = -1;
    for (uint32 i = 0; i < str.length; i++)
    {
        Glyph   glyph        = glyph_get(atlas, str.value[i]);
        float32 w            = glyph_width(glyph, size_in_pixels);
        float32 h            = glyph_height(glyph, size_in_pixels);
        Vec2    plane_offset = {.x = size_in_pixels * glyph.plane_bounds.left, .y = size_in_pixels * glyph.plane_bounds.bottom};
        float32 x            = position.x + base_offset.x + plane_offset.x + w / 2.0f;
        float32 y            = position.y + base_offset.y + plane_offset.y + h / 2.0f - current_line * line_height;

        if (str.value[i] == ' ')
            space_index = i;

        Mat4 transform = transform_quad_aligned(vec3(x, y, 0), vec2(w, h));
        memcpy(&dst_matrices[dst_index + i], &transform, sizeof(transform));
        base_offset.x += glyph.advance * size_in_pixels;

        if (base_offset.x + w > max_width && space_index > 0)
        {
            base_offset.x = initial_base_offset.x;
            current_line++;
            i           = space_index;
            space_index = -1;
        }
    }

    return string_bounds;
}