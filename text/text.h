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
//                                                        Center,  Bottom,   Top,  Right,  Left,  BottomLeft,  BottomRight,  TopLeft,  TopRight
const float32 FontAlignmentMultiplierX[AlignmentCount] = {  -0.5,    -0.5,  -0.5,     -1,     0,           0,           -1,        0,        -1};
const float32 FontAlignmentMultiplierY[AlignmentCount] = {  -0.5,       0,    -1,   -0.5,  -0.5,           0,            0,       -1,        -1};
// clang-format on

typedef enum
{
    GlyphAtlasTypeSDF,
    GlyphAtlasTypeFreeType
} GlyphAtlasType;

typedef struct
{
    uint32 unicode;
    Vec2   advance;
    Bounds plane_bounds;
    Bounds atlas_bounds;
} Glyph;

typedef struct
{
    uint32  width;
    uint32  height;
    uint32  size;
    uint16  distance_range;
    float32 line_height;
    float32 ascender;
    float32 descender;
    float32 underline_y;
    float32 underline_thickness;
} GlyphAtlasInfo;

typedef struct
{
    // Glyph*         glyphs;

    uint32         glyph_count;
    GlyphAtlasType type;
    GlyphAtlasInfo atlas_info;
    TextureIndex   texture;
    RectPacker*    packer;
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
    GlyphAtlasType atlas_type;
    uint64         hash;
    String         name;
    FT_Face        freetype_face;
} FontFace;

typedef struct
{
    uint32        size;
    GlyphAtlas*   atlas;
    FontFaceIndex font_face_index;
} RasterizedFont;

typedef struct FontCacheNode FontCacheNode;
struct FontCacheNode
{
    uint64         hash;
    RasterizedFont v;

    FontCacheNode* next;
};

typedef struct
{
    uint32         count;
    FontCacheNode* first;
    FontCacheNode* last;
} FontCacheList;

typedef struct GlyphCacheNode GlyphCacheNode;
struct GlyphCacheNode
{
    Glyph           glyph;
    uint64          hash;
    GlyphCacheNode* next;
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

    uint64         rasterized_font_cache_capacity;
    FontCacheList* rasterized_font_cache;

    uint64          glyph_cache_capacity;
    GlyphCacheList* glyph_cache;
} FontCache;
FontCache* g_font_cache;

internal float32 glyph_width(Glyph glyph, float32 size);
internal float32 glyph_height(Glyph glyph, float32 size);
internal Vec2    glyph_position(Glyph glyph, float32 size, Vec2 base_offset);

internal void text_line_add(Arena* frame_arena, TextLineList* lines, String str, Vec2 size);

internal Rect text_calculate_bounds(FontFaceIndex font_face, Vec2 position, Alignment alignment, String str, float32 size);
internal Rect text_calculate_transforms(GlyphAtlas* atlas, String str, float32 size_in_pixels, Vec2 position, Alignment alignment, GFX_Batch* batch);
internal Rect text_calculate_glyph_matrices(Arena* frame_arena, GlyphAtlas* atlas, String str, float32 size, Vec2 position, Alignment alignment, float32 max_width, GFX_Batch* batch);

internal void          font_cache_init(Arena* arena);
internal FontFaceIndex font_load(String font_name, String font_path, GlyphAtlasType atlas_type);
internal GlyphAtlas*   font_get_atlas(FontFaceIndex font_face_index, float32 pixel_size);
internal GlyphAtlas*   font_atlas_new(FontFaceIndex font_face_index, float32 pixel_size);
internal Glyph         font_get_glyph(FontFaceIndex font_face_index, float32 pixel_size, uint64 codepoint);
internal uint32        font_pixel_to_font_size(float32 pixel_size);