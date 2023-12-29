#pragma once

#include <core/defines.h>
#include <core/math.h>
#include <core/file.h>
#include <gfx/base.h>
#include <gfx/primitives.h>
#include <gfx/math.h>
#include <gfx/utils.h>
#include <fonts/fonts.h>
#include "color.h"
#include "style.h"
#include "text.h"
#include "layout.h"

typedef struct
{
    Renderer* renderer;
    Camera* camera;

    /* Geometries */
    GeometryIndex geometry_quad;
    GeometryIndex geometry_triangle;

    /* Materials */
    MaterialIndex material_text;
    MaterialIndex material_basic;
    MaterialIndex material_line;
    MaterialIndex material_basic_texture;
    MaterialIndex material_triangle;
    MaterialIndex material_rounded_rect;
    MaterialIndex material_quad;
    MaterialIndex material_circle;
    MaterialIndex material_boid;

    MaterialIndex material_circle_instanced;

    /* Font */
    GlyphAtlas* font_open_sans;
} DrawContext;

/* Shader Data */
typedef struct 
{
    Vec4 glyph_bounds;
    Vec4 color;
    Vec4 outline_color;
    /** Softness of the glyph edges. Recommended value: 30 */
    float32 softness;
    /** Thickness of the glyph. Recommended value: 0.5, Values outside of the range 0.4 and 0.9 are not really usable */
    float32 thickness;
    /** Outline thickness. Should be between 0 and 0.5 */
    float32 outline_thickness;
    float32 _;
} ShaderDataText;

typedef struct 
{
    Vec4 color;
} ShaderDataLine;

typedef struct 
{
    Vec4 color;
} ShaderDataTriangle;

typedef struct 
{
    Vec4 color;
} ShaderDataBasic;

typedef struct 
{
    Vec4 color;
} ShaderDataBasicTexture;

typedef struct
{
    Vec4 color;
    Vec4 edge_color;
    Vec4 round;
    Vec2 scale;
    float32 softness;
    float32 edge_thickness;
} ShaderDataRectRounded;

typedef struct 
{
    Vec4 color;
    float32 fill_ratio;
    // determines how much of the circle will be drawn, 0.5 gives half circle
    float32 slice_ratio;
    Vec2 _;
} ShaderDataCircle;

typedef struct 
{
    Vec4 color;
} ShaderDataBoid;

internal DrawContext*
draw_context_new(Arena* arena, Renderer* renderer);

internal void
draw_line(DrawContext* dc, Vec2 start, Vec2 end, Color color, float32 thickness);

internal void
draw_line_fixed(DrawContext* dc, Vec2 position, float32 length, float32 rotation, Color color, float32 thickness);

internal void
draw_arrow(DrawContext* dc, Vec2 position, float32 length, float32 angle, Color color, float32 thickness);

internal void
draw_debug_line(DrawContext* dc, Vec2 start, Vec2 end, Color color);

internal void
draw_texture_aligned(DrawContext* dc, Vec3 pos, Vec2 scale, TextureIndex texture);

internal void
draw_bounds(DrawContext* dc, float32 left, float32 right, float32 bottom, float32 top, Color color, float32 thickness);

/** Draws given string to screen and returns the bounding box for the while string */
internal Rect
draw_text(DrawContext* dc, Vec2 pos, String str, Alignment alignment, StyleText style);

internal void
draw_circle(DrawContext* dc, Vec2 position, float32 radius, Color color);

internal void
draw_circle_filled(DrawContext* dc, Circle circle, Color color);

internal void
draw_circle_partially_filled(DrawContext* dc, Vec2 position, float32 rotation, float32 radius, Color color, float32 min_angle, float32 max_angle);

internal void
draw_boid(DrawContext* dc, Vec2 position, float32 rotation, float32 size, Color color);

internal void
draw_triangle(DrawContext* dc, Vec2 position, float32 rotation, Color color, float32 size, SortLayerIndex sort_index);

internal Rect
draw_rect(DrawContext* dc, Rect rect, float32 rotation, SortLayerIndex sort_index, StyleRect style);