#pragma once

#include <base.h>
#include <gfx.h>
#include <string.h>

#include "layout.h"

#define DEBUG_TEXT 0

// Center,  Bottom,   Top,  Right,  Left,  BottomLeft,  BottomRight,  TopLeft,  TopRight
const float32 FontAlignmentMultiplierX[AlignmentCount] = {0, 0, 0, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5};
const float32 FontAlignmentMultiplierY[AlignmentCount] = {0, -0.5, 0.5, 0, 0, 0.5, 0.5, 0.5, 0.5};

typedef struct
{
    uint32  unicode;
    float32 advance;
    Bounds  plane_bounds;
    Bounds  atlas_bounds;
} Glyph;

typedef struct
{
    uint16  width;
    uint16  height;
    uint16  size;
    uint16  distance_range;
    float32 line_height;
    float32 ascender;
    float32 descender;
    float32 underline_y;
    float32 underline_thickness;
} GlyphAtlasInfo;

typedef struct
{
    uint32                glyph_count;
    const Glyph*          glyphs;
    const GlyphAtlasInfo* atlas_info;
    TextureIndex          texture;
} GlyphAtlas;

typedef struct
{
    Rect text_bounds;

    uint32 glyph_count;
    Rect*  glyph_rects;
} TextGlyphPositionResult;

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

internal GlyphAtlas* glyph_atlas_load(Arena* arena, const GlyphAtlasInfo* atlas_info, const Glyph* glyphs, uint32 glyph_count, TextureIndex texture);

internal Glyph   glyph_get(GlyphAtlas* atlas, char c);
internal float32 glyph_width(Glyph glyph, float32 size);
internal float32 glyph_height(Glyph glyph, float32 size);
internal Vec2    glyph_position(Glyph glyph, float32 size, Vec2 base_offset);

internal void          text_line_add(Arena* frame_arena, TextLineList* lines, String str, Vec2 size);
internal TextLineList* text_lines_from_string(Arena* frame_arena, GlyphAtlas* atlas, String str, float32 size, float32 max_width);

internal Rect text_calculate_bounds(GlyphAtlas* atlas, Vec2 position, Alignment alignment, String str, float32 size);
internal Rect text_calculate_transforms(GlyphAtlas* atlas, String str, float32 size_in_pixels, Vec2 position, Alignment alignment, Mat4* dst_matrices, uint32 dst_index);
internal Rect text_calculate_glyph_matrices(Arena* frame_arena, GlyphAtlas* atlas, String str, float32 size, Vec2 position, Alignment alignment, float32 max_width, Mat4* dst_matrices, uint32 dst_index);