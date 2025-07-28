#include "text.h"

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

internal Rect
text_calculate_bounds(FontFaceIndex font_face, Vec2 position, Alignment alignment, String str, float32 size)
{
    Vec2   string_size = vec2_zero();
    uint64 processed   = 0;
    while (processed < str.length)
    {
        UnicodeDecode utf_char = utf8_decode(str.value + processed, 4);
        Glyph         glyph    = font_get_glyph(font_face, size, utf_char.codepoint);
        string_size.x += glyph.advance.x * size;
        string_size.y = max(glyph.plane_bounds.top * size, string_size.y);
        processed += utf_char.inc;
    };

    float32 x = string_size.x * AlignmentMultiplierX[alignment];
    float32 y = size * AlignmentMultiplierY[alignment];
    return (Rect){.x = x + position.x, .y = y + position.y, .w = string_size.x, .h = string_size.y};
}

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
    g_font_cache->arena              = arena;
    g_font_cache->font_face_count    = 1; // reserve first slot for null font face
    g_font_cache->font_face_capacity = 512;
    g_font_cache->font_faces         = arena_push_array_zero(arena, FontFace, g_font_cache->font_face_capacity);

    g_font_cache->rasterized_font_cache_capacity = 512;
    g_font_cache->rasterized_font_cache          = arena_push_array_zero(arena, FontCacheList, g_font_cache->rasterized_font_cache_capacity);

    g_font_cache->glyph_cache_capacity = 4096;
    g_font_cache->glyph_cache          = arena_push_array_zero(arena, GlyphCacheList, g_font_cache->glyph_cache_capacity);
}

internal FontFaceIndex
font_load(String font_name, String font_path, GlyphAtlasType atlas_type)
{
    if (g_font_cache == 0)
    {
        log_error("Font cache hasn't been initialized, skipping font load. Font Name: %s", font_name.value);
        return 0;
    }

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

internal Glyph
font_get_glyph(FontFaceIndex font_face_index, float32 pixel_size, uint64 codepoint)
{
    uint32 font_size = font_pixel_to_font_size(pixel_size);
    uint64 params[]  = {font_face_index, font_size, codepoint};
    uint64 hash      = hash_array_uint64(params, array_count(params));

    GlyphCacheList* cache = &g_font_cache->glyph_cache[hash % g_font_cache->glyph_cache_capacity];
    GlyphCacheNode* node;
    for_each(node, cache->first)
    {
        if (node->hash == hash)
        {
            return node->glyph;
        }
    }

    GlyphAtlas* atlas = font_get_atlas(font_face_index, pixel_size);

    FontFace* font_face  = &g_font_cache->font_faces[font_face_index];
    FT_Int32  load_flags = atlas->type == GlyphAtlasTypeFreeType ? FT_LOAD_RENDER : FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);
    if (FT_Load_Char(font_face->freetype_face, (FT_ULong)codepoint, load_flags))
    {
        fprintf(stderr, "ERROR: could not load glyph of a character with code %lld\n", codepoint);
        exit(1);
    }

    FT_GlyphSlot freetype_glyph = font_face->freetype_face->glyph;
    if (FT_Render_Glyph(freetype_glyph, FT_RENDER_MODE_NORMAL))
    {
        fprintf(stderr, "ERROR: could not render glyph of a character with code %lld\n", codepoint);
        exit(1);
    }

    Glyph glyph               = {0};
    glyph.advance.x           = (freetype_glyph->advance.x >> 6) / (float32)pixel_size;
    glyph.advance.y           = (freetype_glyph->advance.y >> 6) / (float32)pixel_size;
    glyph.plane_bounds.left   = (freetype_glyph->bitmap_left) / (float32)pixel_size;
    glyph.plane_bounds.bottom = (freetype_glyph->bitmap_top - (int32)freetype_glyph->bitmap.rows) / (float32)pixel_size;
    glyph.plane_bounds.right  = (freetype_glyph->bitmap_left + (int32)freetype_glyph->bitmap.width) / (float32)pixel_size;
    glyph.plane_bounds.top    = (freetype_glyph->bitmap_top) / (float32)pixel_size;

    RectPackerAddResult rect_add_result = rect_packer_add(atlas->packer, (float32)freetype_glyph->bitmap.width, (float32)freetype_glyph->bitmap.rows);
    glyph.atlas_bounds                  = gfx_rect_to_texture_bounds(rect_add_result.rect, atlas->atlas_info.width, atlas->atlas_info.height);

    gfx_texture_write_rect(atlas->texture, rect_add_result.rect, freetype_glyph->bitmap.buffer);

    node        = arena_push_struct_zero(g_font_cache->arena, GlyphCacheNode);
    node->glyph = glyph;
    node->hash  = hash;
    queue_push(cache->first, cache->last, node);

    return glyph;
}

internal GlyphAtlas*
font_get_atlas(FontFaceIndex font_face_index, float32 pixel_size)
{
    xassert_m(pixel_size > 0, "font size needs to be larger than 0");
    FontFace* font_face = &g_font_cache->font_faces[font_face_index];
    xassert_m(font_face, "could not find given font face");

    uint32 font_size = (uint32)(pixel_size);
    uint32 size      = font_face->atlas_type == GlyphAtlasTypeFreeType ? font_size : 32;
    uint64 params[]  = {font_face_index, size};
    uint64 hash      = hash_array_uint64(params, array_count(params));

    FontCacheList* font_bucket = &g_font_cache->rasterized_font_cache[hash % g_font_cache->rasterized_font_cache_capacity];

    FontCacheNode* node;
    for_each(node, font_bucket->first)
    {
        if (node->hash == hash)
        {
            return node->v.atlas;
        }
    }

    node                    = arena_push_struct_zero(g_font_cache->arena, FontCacheNode);
    node->v.font_face_index = font_face_index;
    node->v.atlas           = font_atlas_new(font_face_index, pixel_size);
    node->v.size            = size;
    node->hash              = hash;

    queue_push(font_bucket->first, font_bucket->last, node);

    return node->v.atlas;
}

internal GlyphAtlas*
font_atlas_new(FontFaceIndex font_face_index, float32 pixel_size)
{
    FontFace* font_face = &g_font_cache->font_faces[font_face_index];
    uint32    font_size = (uint32)(pixel_size);
    uint32    size      = font_face->atlas_type == GlyphAtlasTypeFreeType ? font_size : 32;

    GlyphAtlas* atlas      = arena_push_struct_zero(g_font_cache->arena, GlyphAtlas);
    atlas->type            = font_face->atlas_type;
    atlas->atlas_info.size = size;

    uint32 error = FT_Set_Pixel_Sizes(font_face->freetype_face, 0, size);
    if (error)
    {
        fprintf(stderr, "ERROR: Could not set pixel size to %u\n", size);
        return 0;
    }

    // uint32 x_padding         = 4;
    atlas->atlas_info.width  = 2048;
    atlas->atlas_info.height = 2048;
    atlas->texture           = gfx_texture_new(atlas->atlas_info.width, atlas->atlas_info.height, 1, GL_LINEAR, NULL);
    atlas->packer            = rect_packer_new(g_font_cache->arena, rect_from_bl_tr(vec2_zero(), vec2((float32)atlas->atlas_info.width, (float32)atlas->atlas_info.height)));

    return atlas;
}

internal uint32
font_pixel_to_font_size(float32 pixel_size)
{
    return (uint32)(pixel_size);
}