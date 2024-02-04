#include "text.h"

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

internal Vec2
glyph_position(Glyph glyph, float32 size, Vec2 base_offset)
{
    Vec2 result;
    Vec2 plane_offset = {.x = size * glyph.plane_bounds.left, .y = size * glyph.plane_bounds.bottom};
    result.x          = base_offset.x + plane_offset.x + glyph_width(glyph, size) / 2.0f;
    result.y          = base_offset.y + plane_offset.y + glyph_height(glyph, size) / 2.0f;
    return result;
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

internal void
text_line_add(Arena* frame_arena, TextLineList* lines, String str, Vec2 size)
{
    TextLineNode* line = arena_push_struct_zero(frame_arena, TextLineNode);
    line->v.size       = size;
    line->v.str        = str;
    queue_push(lines->first, lines->last, line);
    lines->size.w = max(lines->size.w, line->v.size.x);
    lines->size.h += max(lines->size.h, line->v.size.y);
    lines->count++;
}

internal TextLineList*
text_lines_from_string(Arena* frame_arena, GlyphAtlas* atlas, String str, float32 size, float32 max_width)
{
    int32   space_index    = -1;
    float32 width          = 0;
    float32 width_at_space = 0;

    TextLineList* lines = arena_push_struct_zero(frame_arena, TextLineList);

    Glyph  glyph;
    String remaining = str;
    for (uint32 i = 0; i < remaining.length; i++)
    {
        glyph = glyph_get(atlas, remaining.value[i]);

        if (char_is_space(remaining.value[i]))
        {
            // NOTE(selim): glyph width is added to cover the last letter of the string
            width_at_space = width;
            space_index    = i;
        }

        float32 new_width = width + glyph.advance * size;
        if (new_width > max_width && space_index > 0)
        {
            text_line_add(frame_arena, lines, string_substr(remaining, 0, space_index + 1), vec2(width_at_space, size));

            remaining = string_skip(remaining, space_index + 1);
            i         = -1;
            width     = 0;
            continue;
        }

        width = new_width;
    };
    text_line_add(frame_arena, lines, remaining, vec2(width, size));
    return lines;
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
#if DEBUG_TEXT
    draw_debug_rect(string_bounds);
#endif
    uint32 index = dst_index;
    for (uint32 i = 0; i < str.length; i++)
    {
        Glyph   glyph        = glyph_get(atlas, str.value[i]);
        float32 w            = glyph_width(glyph, size_in_pixels);
        float32 h            = glyph_height(glyph, size_in_pixels);
        Vec2    plane_offset = {.x = size_in_pixels * glyph.plane_bounds.left, .y = size_in_pixels * glyph.plane_bounds.bottom};
        float32 x            = position.x + base_offset.x + plane_offset.x + w / 2.0f;
        float32 y            = position.y + base_offset.y + plane_offset.y + h / 2.0f;

        Mat4 transform = transform_quad_aligned(vec2(x, y), vec2(w, h));
        memcpy(&dst_matrices[index], &transform, sizeof(transform));
        base_offset.x += glyph.advance * size_in_pixels;
        index++;
    }

    return string_bounds;
}

internal Rect
text_calculate_glyph_matrices(Arena* frame_arena, GlyphAtlas* atlas, String str, float32 size, Vec2 position, Alignment alignment, float32 max_width, Mat4* dst_matrices, uint32 dst_index)
{
    TextLineList* lines         = text_lines_from_string(frame_arena, atlas, str, size, max_width);
    Rect          string_bounds = rect_at(position, lines->size, alignment);
#if DEBUG_TEXT
    draw_debug_rect(string_bounds);
#endif

    TextLineNode* line_node;
    for_each(line_node, lines->first)
    {
        Rect   line_rect       = rect_cut_top(&string_bounds, line_node->v.size.h);
        Rect   inner_line_rect = rect_from_wh(line_node->v.size.w, line_node->v.size.h);
        Anchor a               = {.child = alignment, .parent = alignment};
        inner_line_rect        = rect_anchor(inner_line_rect, line_rect, a);
#if DEBUG_TEXT
        draw_debug_rect_b(inner_line_rect);
#endif

        Vec2 base_offset = {
            .x = inner_line_rect.x - inner_line_rect.w * 0.5,
            .y = inner_line_rect.y - inner_line_rect.h * 0.5};

        String line_str = line_node->v.str;
        for (uint32 i = 0; i < line_str.length; i++)
        {
            Glyph   glyph = glyph_get(atlas, line_str.value[i]);
            float32 w     = glyph_width(glyph, size);
            float32 h     = glyph_height(glyph, size);
            Vec2    pos   = glyph_position(glyph, size, base_offset);

            Mat4 transform = transform_quad_aligned(pos, vec2(w, h));
            memcpy(&dst_matrices[dst_index], &transform, sizeof(transform));
            dst_index++;
            base_offset.x += glyph.advance * size;
        }
    }

    return string_bounds;
}