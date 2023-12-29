#pragma once

#include <string.h>

#include <core/defines.h>
#include <core/math.h>
#include <core/memory.h>
#include <core/strings.h>
#include <gfx/base.h>
#include <gfx/math.h>
#include "layout.h"

                                                                // Center,  Bottom,   Top,  Right,  Left,  BottomLeft,  BottomRight,  TopLeft,  TopRight
const float32 FontAlignmentMultiplierX[AlignmentCount] = {   -0.5,    -0.5,  -0.5,   -1.0,   0.0,         0.0,         -1.0,      0.0,      -1.0 };
const float32 FontAlignmentMultiplierY[AlignmentCount] = {   -0.5,     0.0,  -1.0,   -0.5,  -0.5,         0.0,          0.0,     -1.0,      -1.0 };

typedef struct
{
    uint32 unicode;
    float32 advance;
    Bounds plane_bounds;
    Bounds atlas_bounds;
} Glyph;

typedef struct
{
    uint16 width;
    uint16 height;
    uint16 size;
    uint16 distance_range;
    float32 line_height;
    float32 ascender;
    float32 descender;
    float32 underline_y;
    float32 underline_thickness;
} GlyphAtlasInfo;

typedef struct
{
    uint32 glyph_count;
    const Glyph* glyphs;
    const GlyphAtlasInfo* atlas_info;
    TextureIndex texture;
} GlyphAtlas;

internal Glyph
glyph_get(GlyphAtlas* atlas, char c);

internal float32
glyph_width(Glyph glyph, float32 size);

internal float32
glyph_height(Glyph glyph, float32 size);

internal GlyphAtlas*
glyph_atlas_load(Arena* arena, const GlyphAtlasInfo* atlas_info, const Glyph* glyphs, uint32 glyph_count, TextureIndex texture);

internal Rect
text_calculate_bounds(GlyphAtlas* atlas, Vec2 position, Alignment alignment, String str, float32 size);

internal Rect
text_calculate_transforms(GlyphAtlas* atlas, String str, float32 size_in_pixels, Vec2 position, Alignment alignment, Mat4* dst_matrices, uint32 dst_index);