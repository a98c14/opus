#pragma once

#include <base.h>
#include <engine/color.h>
#include <gfx.h>
#include <physics/intersection.h>

#include "color.h"
#include "layout.h"
#include "text.h"

typedef struct
{
    float32 thickness;
    Vec4    outline_color;
    float32 outline_thickness;
    float32 softness;
} DrawStyleFont;

global read_only DrawStyleFont d_default_font_style = {
    0.55,
    {.r = 0, .b = 0, .g = 0, .a = 0},
    0.3,
    30
};

typedef struct DrawContextNode DrawContextNode;
struct DrawContextNode
{
    /** render info */
    ViewType       view;
    SortLayerIndex sort_layer;
    PassIndex      pass;

    /** styling */
    FontFaceIndex font_face;
    DrawStyleFont font_style;

    DrawContextNode* next;
};

global read_only DrawContextNode d_default_node = {
    .sort_layer = 12,
    .view       = ViewTypeScreen,
};

/** default settings */
global read_only float32 d_default_text_baseline          = 1;
global read_only float32 d_default_text_thickness         = 0.52;
global read_only float32 d_default_text_softness          = 20;
global read_only float32 d_default_text_outline_thickness = 0.2;
global read_only Vec4    d_color_none                     = {.r = 0, .b = 0, .g = 0, .a = 0};
global read_only Vec4    d_color_none_white               = {.r = 1, .b = 1, .g = 1, .a = 0};
global read_only Vec4    d_color_black                    = {.r = 0, .b = 0, .g = 0, .a = 1};
global read_only Vec4    d_color_white                    = {.r = 1, .b = 1, .g = 1, .a = 1};

typedef struct
{
    Renderer* renderer;
    Camera*   camera;
    Arena*    persistent_arena;
    Arena*    frame_arena;

    /* materials */
    MaterialIndex material_basic_trail;
    MaterialIndex material_text;
    MaterialIndex material_text_free_type;
    MaterialIndex material_text_free_type_sdf;
    MaterialIndex material_basic;
    MaterialIndex material_line;
    MaterialIndex material_basic_texture;
    MaterialIndex material_triangle;
    MaterialIndex material_rounded_rect;
    MaterialIndex material_quad;
    MaterialIndex material_circle;
    MaterialIndex material_boid;
    MaterialIndex material_sprite;
    MaterialIndex material_sprite_border;
    MaterialIndex material_circle_instanced;

    /** draw state context */
    DrawContextNode* free_nodes;
    DrawContextNode* ctx;

    /** atlas */
    SpriteAtlas* sprite_atlas;
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
} ShaderDataTrail;

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
    int32 sprite_index;
    int32 texture_layer_index;
    Vec2  size;
    Vec4  color;
    int32 protection;
    Vec3  _;
} ShaderDataSpriteBorder;

typedef struct
{
    Vec4 color;
} ShaderDataBoid;

internal void draw_context_init(Arena* arena, Arena* temp_arena, Renderer* renderer, PassIndex default_pass);
internal void draw_context_activate_atlas(SpriteAtlas* atlas);

/** context push */
internal void draw_activate_font(FontFaceIndex font_face);
internal void draw_context_set_font_style(DrawStyleFont style);
internal void draw_context_push(SortLayerIndex sort_layer, ViewType view_type, PassIndex pass);
internal void draw_context_pop();
#define draw_scope(sort_layer, view_type, pass) defer_loop(draw_context_push(sort_layer, view_type, pass), draw_context_pop())

/* draw_functions */
internal void draw_line(Vec2 start, Vec2 end, Color color, float32 thickness);
internal void draw_heading(Vec2 origin, Vec2 heading, Color color, float32 thickness);
internal void draw_line_fixed(Vec2 position, float32 length, float32 rotation, Color color, float32 thickness);
internal void draw_arrow(Vec2 position, float32 length, float32 rotation, Color color, float32 thickness);
internal void draw_triangle(Vec2 position, float32 rotation, Color color, float32 size);

internal Rect draw_rect_rotated(Rect rect, float32 rotation, Color color);
internal Rect draw_rect(Rect rect, Color color);
internal Rect draw_rect_outline(Rect rect, Color color, float32 thickness);
internal void draw_texture_aligned(Vec2 pos, Vec2 scale, TextureIndex texture);
internal void draw_bounds(float32 left, float32 right, float32 bottom, float32 top, Color color, float32 thickness);

internal Rect draw_text_at_internal(String str, Vec2 pos, Alignment alignment, float32 size, Color color);
internal Rect draw_text_at(String str, Vec2 pos, Alignment alignment, float32 size, Color color);
internal Rect draw_text(String str, Rect rect, Anchor anchor, float32 size, Color color);
internal void draw_circle(Vec2 pos, float32 radius, Color color);
internal void draw_circle_filled(Vec2 pos, float32 radius, Color color);
internal void draw_circle_partially_filled(Vec2 position, float32 rotation, float32 radius, Color color, float32 min_angle, float32 max_angle);

/* sprite */
internal void draw_sprite_colored(Vec2 position, float32 scale, float32 rotation, SpriteIndex sprite, Vec2 flip, Color color, float32 alpha);
internal void draw_sprite_colored_ignore_pivot(Vec2 position, float32 scale, SpriteIndex sprite, Vec2 flip, Color color, float32 alpha);
internal void draw_sprite(Vec2 position, float32 scale, float32 rotation, SpriteIndex sprite, Vec2 flip);

/** trail */
internal void draw_trail(Vec2* points, uint32 point_count, Color color);

/** extra draw functions */
internal Rect draw_debug_rect(Rect rect);
internal Rect draw_debug_rect_b(Rect rect);
internal Rect draw_sprite_rect(Rect rect, SpriteIndex sprite, Anchor anchor);
internal Rect draw_sprite_rect_colored(Rect rect, SpriteIndex sprite, Anchor anchor, Color color, float32 alpha);
internal Rect draw_sprite_rect_cut(Rect* rect, CutSide side, SpriteIndex sprite, Anchor anchor);
internal Rect draw_sprite_rect_cut_colored(Rect* rect, CutSide side, SpriteIndex sprite, Anchor anchor, Color color, float32 alpha);
internal Rect draw_sprite_rect_flipped(Rect* rect, CutSide side, SpriteIndex sprite, Anchor anchor);
internal Rect sprite_rect(SpriteIndex sprite);
internal Rect sprite_rect_with_pivot(SpriteIndex sprite, Vec2 position, Vec2 flip, float32 scale_multiplier);

/** TEMP(selim): will be moved to renderer */
typedef struct
{
    Vec3           position;
    float32        sort_order;
    float32        rotation;
    Vec2           scale;
    SpriteIndex    sprite;
    SortLayerIndex layer;
    Color          color;
} SpriteRenderRequest;

typedef struct
{
    int32                count;
    SpriteRenderRequest* arr;
} SpriteRenderRequestBuffer;
internal void render_sprites_sorted(Arena* frame_arena, PassIndex pass, SpriteRenderRequest* requests, uint64 count, int32* layer_entity_counts);
internal int  qsort_compare_render_requests_descending(const void* p, const void* q);

internal VertexBuffer* draw_util_generate_trail_vertices_fast(Arena* arena, Vec2* points, uint32 point_count, uint32 start_index, float32 trail_width);