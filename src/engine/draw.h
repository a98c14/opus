#pragma once

#include <base/asserts.h>
#include <base/defines.h>
#include <base/file.h>
#include <base/math.h>
#include <base/strings.h>
#include <engine/color.h>
#include <engine/draw.h>
#include <engine/layout.h>
#include <fonts/fonts.h>
#include <gfx/base.h>
#include <gfx/math.h>
#include <gfx/primitives.h>
#include <gfx/sprite.h>
#include <gfx/utils.h>

#include "color.h"
#include "layout.h"
#include "style.h"
#include "text.h"

typedef struct DrawContextNode DrawContextNode;
struct DrawContextNode
{
    ViewType       view;
    SortLayerIndex sort_layer;

    DrawContextNode* next;
};

global read_only DrawContextNode d_default_node = {
    .sort_layer = 12,
    .view       = ViewTypeScreen,
};

typedef struct
{
    Renderer* renderer;
    Camera*   camera;
    Arena*    persistent_arena;

    /* geometries */
    GeometryIndex geometry_quad;
    GeometryIndex geometry_triangle;

    /* materials */
    MaterialIndex material_text;
    MaterialIndex material_basic;
    MaterialIndex material_line;
    MaterialIndex material_basic_texture;
    MaterialIndex material_triangle;
    MaterialIndex material_rounded_rect;
    MaterialIndex material_quad;
    MaterialIndex material_circle;
    MaterialIndex material_boid;
    MaterialIndex material_sprite;
    MaterialIndex material_circle_instanced;

    /** draw state context */
    DrawContextNode* free_nodes;
    DrawContextNode* ctx;

    /** atlas */
    GlyphAtlas*  font_open_sans;
    SpriteAtlas* sprite_atlas;

    /** default styles */
    StyleRect style_debug_rect;
    StyleRect style_default_rect;
    StyleText style_default_text;
} D_State;
global D_State* d_state;

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
    Vec4    color;
    Vec4    edge_color;
    Vec4    round;
    Vec2    scale;
    float32 softness;
    float32 edge_thickness;
} ShaderDataRectRounded;

typedef struct
{
    Vec4    color;
    float32 fill_ratio;
    // determines how much of the circle will be drawn, 0.5 gives half circle
    float32 slice_ratio;
    Vec2    _;
} ShaderDataCircle;

typedef struct
{
    int32   sprite_index;
    int32   texture_layer_index;
    float32 alpha;
    float32 color_strength;
    Vec4    color;
} ShaderDataSprite;

typedef struct
{
    Vec4 color;
} ShaderDataBoid;

internal void draw_context_initialize(Arena* arena, Arena* temp_arena, Renderer* renderer);
internal void draw_context_activate_atlas(SpriteAtlas* atlas);

/* Draw Functions */
internal void draw_line(Vec2 start, Vec2 end, Color color, float32 thickness, SortLayerIndex layer);
internal void draw_line_2(Vec2 start, Vec2 end, Color color, float32 thickness, FrameBufferIndex frame_buffer_index, SortLayerIndex layer);
internal void draw_line_fixed(Vec2 position, float32 length, float32 rotation, Color color, float32 thickness, SortLayerIndex layer);
internal void draw_arrow(Vec2 position, float32 length, float32 angle, Color color, float32 thickness, SortLayerIndex layer);
internal void draw_debug_line(Vec2 start, Vec2 end, Color color, SortLayerIndex layer);
internal void draw_texture_aligned(Vec3 pos, Vec2 scale, TextureIndex texture, SortLayerIndex layer);
internal void draw_bounds(float32 left, float32 right, float32 bottom, float32 top, Color color, float32 thickness, SortLayerIndex layer);

// Draws given string to screen and returns the bounding box for the while string
internal Rect draw_text_at(Vec2 pos, String str, Alignment alignment, StyleText style, ViewType view_type, SortLayerIndex layer);
internal Rect draw_text(Rect rect, String str, Anchor anchor, StyleText style, ViewType view_type, SortLayerIndex layer);
internal void draw_circle(Vec2 position, float32 radius, Color color, SortLayerIndex layer);
internal void draw_circle_filled(Circle circle, Color color, SortLayerIndex layer);
internal void draw_circle_partially_filled(Vec2 position, float32 rotation, float32 radius, Color color, float32 min_angle, float32 max_angle, SortLayerIndex layer);
// TODO(selim): why is this here?
internal void draw_boid(Vec2 position, float32 rotation, float32 size, Color color, SortLayerIndex layer);
internal void draw_triangle(Vec2 position, float32 rotation, Color color, float32 size, SortLayerIndex sort_index);
internal Rect draw_rect_simple(Rect rect, float32 rotation, SortLayerIndex sort_index, ViewType view_type, Vec4 color);
internal Rect draw_rect_rotated(Rect rect, float32 rotation, SortLayerIndex sort_index, ViewType view_type, StyleRect style);
internal Rect draw_rect_internal(Rect rect, SortLayerIndex sort_index, ViewType view_type, StyleRect style);

/* sprite */
internal void draw_sprite_colored(Vec2 position, float32 scale, float32 rotation, SpriteIndex sprite, Vec2 flip, ViewType view_type, SortLayerIndex layer, Color color);
internal void draw_sprite_colored_ignore_pivot(Vec2 position, float32 scale, SpriteIndex sprite, Vec2 flip, ViewType view_type, SortLayerIndex layer, Color color, float32 alpha);
internal void draw_sprite(Vec2 position, float32 scale, float32 rotation, SpriteIndex sprite, Vec2 flip, ViewType view_type, SortLayerIndex layer);

/** Utility */
// TODO(selim): Move these to renderer (make renderer global as well)
internal float32 screen_top();
internal float32 screen_left();
internal float32 screen_right();
internal float32 screen_bottom();
internal float32 screen_height();
internal float32 screen_width();
internal Rect    screen_rect();

/** context push */
internal void draw_context_push(SortLayerIndex sort_layer, ViewType view_type);
internal void draw_context_pop();
#define draw_scope(sort_layer, view_type) defer_loop(draw_context_push(sort_layer, view_type), draw_context_pop())

/** extra draw functions */
internal Rect draw_rect(Rect rect, Color color);
internal Rect draw_debug_rect(Rect rect);
internal Rect draw_debug_rect_screen(Rect rect);
internal Rect draw_sprite_rect(Rect* rect, CutSide side, SpriteIndex sprite, Anchor anchor);
internal Rect draw_sprite_rect_colored(Rect* rect, CutSide side, SpriteIndex sprite, Anchor anchor, Color color, float32 alpha);
internal Rect draw_sprite_rect_flipped(Rect* rect, CutSide side, SpriteIndex sprite, Anchor anchor);
internal Rect get_sprite_rect(SpriteIndex sprite);
internal Rect draw_text_screen(Rect rect, String str, Anchor anchor, float32 font_size);