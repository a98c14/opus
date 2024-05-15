#pragma once
#include <base.h>
#include <gfx.h>
#include <text/text_inc.h>

#include "draw_shaders.h"

typedef struct
{
    Arena* frame_arena;

    /** state */
    PassIndex      active_pass;
    SortLayerIndex active_layer;
    FontFaceIndex  active_font;

    /** materials */
    MaterialIndex material_basic;
    MaterialIndex material_rect;
    MaterialIndex material_circle;
    MaterialIndex material_sprite;
    MaterialIndex material_text;
} D_Context;
D_Context* d_context;

typedef struct
{
    Mat4    model;
    Vec4    color;
    float32 thickness;
    Vec3    _;
} D_ShaderDataCircle;

internal void d_context_init(Arena* persistent_arena, Arena* frame_arena, String asset_path);

/** batch functions */
internal void d_mesh_push_vertex(VertexAtrribute_TexturedColored* vertex_buffer, uint64* vertex_count, Vec2 pos, Vec2 tex_coord, Color color);
internal void d_mesh_push_quad(VertexAtrribute_TexturedColored* vertex_buffer, uint64* vertex_count, Rect rect, Bounds tex_coord, Color color);
internal void d_mesh_push_glyph(VertexAtrribute_TexturedColored* vertex_buffer, uint64* vertex_count, Glyph glyph, Vec2 pos, float32 size, Color color);
internal void d_mesh_push_string(VertexAtrribute_TexturedColored* vertex_buffer, uint64* vertex_count, GlyphAtlas* atlas, String str, Vec2 pos, float32 size, Color c);

/** draw functions */
internal void d_line(Vec2 start, Vec2 end, float32 thickness, Color c);
internal void d_debug_line(Vec2 start, Vec2 end);
internal void d_debug_line2(Vec2 start, Vec2 end);
internal void d_circle(Vec2 pos, float32 radius, float32 thickness, Color c);
internal void d_string(Vec2 pos, String str, int32 size, Color c);