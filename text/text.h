#pragma once
// NOTE(selim): `FT_FREETYPE_H` uses `internal` as variable name so this is a workaround
// for that. Needs a better solution
#undef internal
#include "ft2build.h"
#include FT_FREETYPE_H
#define internal static
#include <string.h>

#include "../base/base_inc.h"
#include "../gfx/gfx_inc.h"

#define DEBUG_TEXT 0

// clang-format off
//                                                        Center,  Bottom,   Top,  Right,   Left,  BottomLeft,  BottomRight,  TopLeft,  TopRight
const float32 FontAlignmentMultiplierY[AlignmentCount] = {  -0.375,  -0.5, -0.25, -0.375, -0.375,        -0.5,         -0.5,    -0.25,     -0.25};
// clang-format on

typedef uint32 AtlasIndex;

typedef struct
{
    uint32     unicode;
    Vec2       advance;
    Bounds     plane_bounds;
    Bounds     atlas_bounds;
    AtlasIndex atlas_index;
} Glyph;

typedef struct
{
    uint32  width;
    uint32  height;
    uint16  distance_range;
    float32 line_height;
    float32 ascender;
    float32 descender;
    float32 underline_y;
    float32 underline_thickness;
} GlyphAtlasInfo;

typedef struct
{
    uint32         index;
    uint32         glyph_count;
    GlyphAtlasInfo atlas_info;
    TextureIndex   texture;
    RectPacker*    packer;
    bool32         is_full;
    bool32         is_initialized;
} GlyphAtlas;

typedef struct
{
    String str;
    Vec2   size;
} TextLine;

typedef struct TextLineNode TextLineNode;
struct TextLineNode
{
    TextLine v;

    TextLineNode* next;
};

typedef struct
{
    Vec2          size;
    uint32        count;
    TextLineNode* first;
    TextLineNode* last;
} TextLineList;

typedef uint32 FontFaceIndex;
typedef struct
{
    uint64  hash;
    String  name;
    FT_Face freetype_face;
} FontFace;

typedef struct
{
    uint32        size;
    GlyphAtlas*   atlas;
    FontFaceIndex font_face_index;
} RasterizedFont;

typedef struct GlyphCacheNode GlyphCacheNode;
struct GlyphCacheNode
{
    Glyph           glyph;
    uint64          hash;
    GlyphCacheNode* next;
    uint64          codepoint;
};

typedef struct
{
    uint32          count;
    GlyphCacheNode* first;
    GlyphCacheNode* last;
} GlyphCacheList;

typedef struct
{
    Arena*     arena;
    FT_Library library;

    uint32    font_face_capacity;
    uint32    font_face_count;
    FontFace* font_faces;

    uint32      atlas_count;
    GlyphAtlas* glyph_atlases;

    uint64          glyph_cache_capacity;
    GlyphCacheList* glyph_cache;
} FontCache;
FontCache* g_font_cache;

internal float32 glyph_width(Glyph glyph, float32 size);
internal float32 glyph_height(Glyph glyph, float32 size);
internal Vec2    glyph_position(Glyph glyph, float32 size, Vec2 base_offset);

internal void text_line_add(Arena* frame_arena, TextLineList* lines, String str, Vec2 size);

internal Vec2 text_calculate_size(FontFaceIndex font_face, String str, float32 size);
internal Rect text_calculate_rect(FontFaceIndex font_face, Vec2 position, Alignment alignment, String str, float32 size);
internal Rect text_calculate_transforms(GlyphAtlas* atlas, String str, float32 size_in_pixels, Vec2 position, Alignment alignment, GFX_Batch* batch);
internal Rect text_calculate_glyph_matrices(Arena* frame_arena, GlyphAtlas* atlas, String str, float32 size, Vec2 position, Alignment alignment, float32 max_width, GFX_Batch* batch);

internal void          font_cache_init(Arena* arena);
internal FontFaceIndex font_load(String font_name, String font_path);
internal Glyph         font_get_glyph(FontFaceIndex font_face_index, float32 pixel_size, uint64 codepoint);
internal GlyphAtlas*   font_get_available_atlas();
internal void          font_atlas_init(GlyphAtlas* atlas, AtlasIndex atlas_index);
internal uint32        font_pixel_to_font_size(float32 pixel_size);
internal TextureIndex  font_get_atlas_texture(AtlasIndex atlas_index);