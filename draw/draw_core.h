#pragma once
#include "../base/base_inc.h"
#include "../gfx/gfx_inc.h"
#include "../text/text_inc.h"

#include "draw_shaders.h"
#include "draw_sprite.h"

/** material uniform data */
typedef struct
{
    Mat4    model;
    Vec4    color;
    float32 thickness;
    Vec3    _;
} D_ShaderDataCircle;

typedef struct
{
    Mat4 model;
    Vec4 color;
} D_ShaderDataTriangle;

typedef struct
{
    Mat4 model;
    Vec4 bounds;
    Vec4 color;
} D_ShaderDataSprite;

typedef struct
{
    Arena* perm_arena;
    Arena* frame_arena;

    /** state */
    PassIndex      active_pass;
    SortLayerIndex active_layer;
    FontFaceIndex  active_font;
    GFX_ViewType   active_view;

    /** materials */
    MaterialIndex material_basic;
    MaterialIndex material_rect;
    MaterialIndex material_circle;
    MaterialIndex material_sprite;
    MaterialIndex material_text;
    MaterialIndex material_triangle;
} D_Context;
global D_Context* d_context;

/** draw data */
typedef uint32 D_DrawFlagsSprite;
enum
{
    D_DrawFlagsSpriteNone        = 0,
    D_DrawFlagsSpriteFlipX       = 1 << 0,
    D_DrawFlagsSpriteFlipY       = 1 << 1,
    D_DrawFlagsSpriteIgnorePivot = 1 << 2,
};

typedef struct
{
    Vec3              position;
    Vec2              scale;
    float32           rotation;
    D_DrawFlagsSprite flags;
    D_SpriteIndex     sprite;
    Color             color;
} D_DrawDataSprite;

internal void d_context_init(Arena* persistent_arena, Arena* frame_arena, String asset_path);

/** batch functions */
internal void d_mesh_push_vertex(GFX_VertexAtrribute_TexturedColored* vertex_buffer, uint32* vertex_count, Vec2 pos, Vec2 tex_coord, Color color);
internal void d_mesh_push_rect(GFX_VertexAtrribute_TexturedColored* vertex_buffer, uint32* vertex_count, Rect rect, Bounds tex_coord, Color color);
internal void d_mesh_push_glyph(GFX_VertexAtrribute_TexturedColored* vertex_buffer, uint32* vertex_count, Glyph glyph, Vec2 pos, float32 size, Color color);
internal void d_mesh_push_string(GFX_VertexAtrribute_TexturedColored* vertex_buffer, uint32* vertex_count, GlyphAtlas* atlas, String str, Vec2 pos, float32 size, Color c);
internal void d_mesh_push_triangle_strip(GFX_VertexAtrribute_TexturedColored* vertex_buffer, uint32* vertex_count, Vec2 pos, Vec2 tex_coord, Color color);
internal void d_mesh_push_line(GFX_VertexAtrribute_TexturedColored* vertex_buffer, uint32* vertex_count, Vec2 start, Vec2 end, float32 thickness, Color c);

/** draw functions */
internal void d_line(Vec2 start, Vec2 end, float32 thickness, Color c);
internal void d_direction(Vec2 start, Vec2 direction, float32 scale, float32 thickness, Color c);
internal void d_triangle(Vec2 pos, Vec2 scale, float32 rotation, Color c);
internal Rect d_rect(Rect r, float32 thickness, Color c);
internal void d_quad(Quad q, float32 thickness, Color c);
internal void d_material_raw(MaterialIndex material, void* shader_data);
internal void d_circle_scaled(Vec2 pos, float32 radius, Vec2 scale, float32 thickness, Color c);
internal void d_circle(Vec2 pos, float32 radius, float32 thickness, Color c);
internal void d_circle_filled(Vec2 pos, float32 radius, Color c);
internal Rect d_string(Rect r, String str, float32 size, Color c, Anchor anchor);
internal Rect d_string_at(Vec2 pos, String str, float32 size, Color c, Alignment alignment);
internal Rect d_string_raw(Vec2 pos, String str, float32 size, Color c, Alignment alignment, MaterialIndex material);
internal void d_sprite_many(D_SpriteAtlas atlas, D_DrawDataSprite* draw_data, uint32 sprite_count, bool32 sort);
internal Rect d_sprite(D_SpriteAtlas* atlas, D_SpriteIndex sprite_index, Rect rect, Vec2 scale, Anchor anchor, Color c);
internal void d_sprite_at(D_SpriteAtlas atlas, D_SpriteIndex sprite_index, Vec2 pos, Vec2 scale, float32 rotation, Color color);
// internal void d_arrow_pro(Vec2 start, Vec2 end, float32 thickness, float32 head_size, Color color);
internal void d_arrow(Vec2 start, Vec2 end, float32 size, Color color);

/** debug draw functions */
internal void d_debug_line(Vec2 start, Vec2 end);
internal void d_debug_line2(Vec2 start, Vec2 end);
internal void d_debug_rect(Rect r);
internal void d_debug_rect2(Rect r);

/** sort */
internal int d_compare_sprite_draw_data(const void* p, const void* q);