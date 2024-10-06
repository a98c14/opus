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
glyph_atlas_load(Arena* arena, GlyphAtlasInfo* atlas_info, Glyph* glyphs, uint32 glyph_count, TextureIndex texture)
{
    GlyphAtlas* atlas  = arena_push_struct_zero(arena, GlyphAtlas);
    atlas->glyphs      = glyphs;
    atlas->glyph_count = glyph_count;
    atlas->atlas_info  = *atlas_info;
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
    lines->size.h += line->v.size.y;
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
            i         = 0;
            width     = 0;
            continue;
        }

        width = new_width;
    }
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
text_calculate_transforms(GlyphAtlas* atlas, String str, float32 size_in_pixels, Vec2 position, Alignment alignment, GFX_Batch* batch);
// {
//     // TODO(selim): !!!!!!!!!!!!!!!!!!!!!!
//     //     Rect string_bounds = text_calculate_bounds(atlas, position, alignment, str, size_in_pixels);
//     //     Vec2 base_offset   = {
//     //           .x = string_bounds.w * FontAlignmentMultiplierX[alignment],
//     //           .y = string_bounds.h * FontAlignmentMultiplierY[alignment]};
//     // #if DEBUG_TEXT
//     //     draw_debug_rect(string_bounds);
//     // #endif
//     //     uint32 index = dst_index;
//     //     for (uint32 i = 0; i < str.length; i++)
//     //     {
//     //         Glyph   glyph        = glyph_get(atlas, str.value[i]);
//     //         float32 w            = glyph_width(glyph, size_in_pixels);
//     //         float32 h            = glyph_height(glyph, size_in_pixels);
//     //         Vec2    plane_offset = {.x = size_in_pixels * glyph.plane_bounds.left, .y = size_in_pixels * glyph.plane_bounds.bottom};
//     //         float32 x            = position.x + base_offset.x + plane_offset.x + w / 2.0f;
//     //         float32 y            = position.y + base_offset.y + plane_offset.y + h / 2.0f;

//     //         Mat4 transform = transform_quad_aligned(vec2(x, y), vec2(w, h));
//     //         memcpy(&dst_matrices[index], &transform, sizeof(transform));
//     //         base_offset.x += glyph.advance * size_in_pixels;
//     //         index++;
//     //     }

//     // return string_bounds;
//     return (Rect){0};
// }

internal Rect
text_calculate_glyph_matrices(Arena* frame_arena, GlyphAtlas* atlas, String str, float32 size, Vec2 position, Alignment alignment, float32 max_width, GFX_Batch* batch);
// {
//     // TODO(selim): !!!!!!!!!!!!!!!!!!!!!!
//     //     TextLineList* lines         = text_lines_from_string(frame_arena, atlas, str, size, max_width);
//     //     Rect          string_bounds = rect_at(position, lines->size, alignment);
//     // #if DEBUG_TEXT
//     //     draw_debug_rect(string_bounds);
//     // #endif

//     //     TextLineNode* line_node;
//     //     Rect          lines_rect = string_bounds;
//     //     for_each(line_node, lines->first)
//     //     {
//     //         Rect   line_rect       = rect_cut_top(&lines_rect, line_node->v.size.h);
//     //         Rect   inner_line_rect = rect_from_wh(line_node->v.size.w, line_node->v.size.h);
//     //         Anchor a               = {.child = alignment, .parent = alignment};
//     //         inner_line_rect        = rect_anchor(inner_line_rect, line_rect, a);
//     // #if DEBUG_TEXT
//     //         draw_debug_rect_b(inner_line_rect);
//     // #endif
//     //         Vec2 base_offset = {
//     //             .x = inner_line_rect.x - inner_line_rect.w * 0.5,
//     //             .y = inner_line_rect.y - inner_line_rect.h * 0.5};

//     //         String line_str = line_node->v.str;
//     //         for (uint32 i = 0; i < line_str.length; i++)
//     //         {
//     //             Glyph   glyph = glyph_get(atlas, line_str.value[i]);
//     //             float32 w     = glyph_width(glyph, size);
//     //             float32 h     = glyph_height(glyph, size);
//     //             Vec2    pos   = glyph_position(glyph, size, base_offset);

//     //             Mat4 transform = transform_quad_aligned(pos, vec2(w, h));
//     //             memcpy(&dst_matrices[dst_index], &transform, sizeof(transform));
//     //             dst_index++;
//     //             base_offset.x += glyph.advance * size;
//     //         }
//     //     }

//     //     return string_bounds;
//     return (Rect){0};
// }

/** freetype */
internal void
font_cache_init(Arena* arena)
{
    g_font_cache = arena_push_struct_zero(arena, FontCache);
    int32 error  = FT_Init_FreeType(&g_font_cache->library);
    if (error)
    {
        log_error("Could not initialize freetype.");
        return;
    }
    g_font_cache->arena           = arena;
    g_font_cache->font_face_count = 64;
    g_font_cache->font_faces      = arena_push_array_zero(arena, FontFace, g_font_cache->font_face_count);

    g_font_cache->rasterized_font_cache_capacity = 128;
    g_font_cache->rasterized_font_cache          = arena_push_array_zero(arena, FontCacheList, g_font_cache->rasterized_font_cache_capacity);
}

internal FontFaceIndex
font_load(String font_name, String font_path, GlyphAtlasType atlas_type)
{
    FontFace face  = {0};
    int32    error = FT_New_Face(g_font_cache->library, font_path.value, 0, &face.freetype_face);
    if (error == FT_Err_Unknown_File_Format)
    {
        log_error("Could not load font face, unknown format.");
    }
    else if (error)
    {
        log_error("Could not load font face.");
    }

    // TODO(selim): Check if font face is already loaded
    FontFaceIndex face_index = g_font_cache->font_face_count;
    face.name                = font_name;
    face.hash                = hash_string(font_name);
    face.atlas_type          = atlas_type;

    g_font_cache->font_faces[g_font_cache->font_face_count] = face;
    g_font_cache->font_face_count++;
    return face_index;
}

internal GlyphAtlas*
font_get_atlas(FontFaceIndex font_face_index, float32 pixel_size)
{
    xassert(pixel_size > 0, "font size needs to be larger than 0");
    FontFace* font_face = &g_font_cache->font_faces[font_face_index];
    xassert(font_face, "could not find given font face");

    uint16         font_size   = (uint16)px(ceilf(pixel_size));
    uint16         size        = font_face->atlas_type == GlyphAtlasTypeFreeType ? font_size : 32;
    uint64         params[]    = {font_face_index, size};
    uint64         hash        = hash_array_uint64(params, array_count(params));
    FontCacheList* font_bucket = &g_font_cache->rasterized_font_cache[hash % g_font_cache->rasterized_font_cache_capacity];
    FontCacheNode* node;
    for_each(node, font_bucket->first)
    {
        if (node->hash == hash)
        {
            return node->v.atlas;
        }
    }

    GlyphAtlas* atlas      = arena_push_struct_zero(g_font_cache->arena, GlyphAtlas);
    atlas->glyph_count     = 128;
    atlas->glyphs          = arena_push_array_zero(g_font_cache->arena, Glyph, atlas->glyph_count);
    atlas->type            = font_face->atlas_type;
    atlas->atlas_info.size = size;

    uint32 error = FT_Set_Pixel_Sizes(font_face->freetype_face, 0, size);
    if (error)
    {
        fprintf(stderr, "ERROR: Could not set pixel size to %u\n", size);
        return 0;
    }

    FT_Int32 calc_flags = atlas->type == GlyphAtlasTypeFreeType ? FT_LOAD_DEFAULT : FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);

    // TODO(selim): Load non-ASCII characters as well
    for (int i = 32; i < 128; ++i)
    {
        if (FT_Load_Char(font_face->freetype_face, i, calc_flags))
        {
            fprintf(stderr, "ERROR: could not load glyph of a character with code %d\n", i);
            exit(1);
        }

        atlas->atlas_info.width += font_face->freetype_face->glyph->bitmap.width + 2;
        if (atlas->atlas_info.height < font_face->freetype_face->glyph->bitmap.rows)
        {
            atlas->atlas_info.height = font_face->freetype_face->glyph->bitmap.rows;
        }
    }

    atlas->texture      = gfx_texture_new(atlas->atlas_info.width, atlas->atlas_info.height, 1, GL_LINEAR, NULL);
    FT_Int32 load_flags = atlas->type == GlyphAtlasTypeFreeType ? FT_LOAD_RENDER : FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);

    int32 x = 0;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (uint32 i = 32; i < 128; ++i)
    {
        if (FT_Load_Char(font_face->freetype_face, i, load_flags))
        {
            fprintf(stderr, "ERROR: could not load glyph of a character with code %d\n", i);
            exit(1);
        }

        FT_GlyphSlot glyph = font_face->freetype_face->glyph;
        if (FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL))
        {
            fprintf(stderr, "ERROR: could not render glyph of a character with code %d\n", i);
            exit(1);
        }

        atlas->glyphs[i - 32].advance             = (glyph->advance.x >> 6) / (float32)size;
        atlas->glyphs[i - 32].advance_y           = (glyph->advance.y >> 6) / (float32)size;
        atlas->glyphs[i - 32].plane_bounds.left   = (glyph->bitmap_left) / (float32)size;
        atlas->glyphs[i - 32].plane_bounds.bottom = (glyph->bitmap_top - (int32)glyph->bitmap.rows) / (float32)size;
        atlas->glyphs[i - 32].plane_bounds.right  = (glyph->bitmap_left + (int32)glyph->bitmap.width) / (float32)size;
        atlas->glyphs[i - 32].plane_bounds.top    = (glyph->bitmap_top) / (float32)size;

        atlas->glyphs[i - 32].atlas_bounds.left   = x / (float32)atlas->atlas_info.width;
        atlas->glyphs[i - 32].atlas_bounds.bottom = (glyph->bitmap.rows) / (float32)atlas->atlas_info.height;
        atlas->glyphs[i - 32].atlas_bounds.right  = (x + glyph->bitmap.width) / (float32)atlas->atlas_info.width;
        atlas->glyphs[i - 32].atlas_bounds.top    = 0;

        glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, glyph->bitmap.width, glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer);
        x += glyph->bitmap.width + 2;
    }

    node                    = arena_push_struct_zero(g_font_cache->arena, FontCacheNode);
    node->v.font_face_index = font_face_index;
    node->v.atlas           = atlas;
    node->v.size            = size;
    node->hash              = hash;
    queue_push(font_bucket->first, font_bucket->last, node);
    return node->v.atlas;
}