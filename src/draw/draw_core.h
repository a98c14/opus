#pragma once
#include <base.h>
#include <gfx.h>
#include <text/text_inc.h>

#include "draw_shaders.h"

typedef struct
{
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
    float32 thickness;
} TempCircleShaderData;

internal void d_context_init(Arena* persistent_arena, String asset_path);

/** batch functions */
internal void d_batch_push_vertex_pos_tex_color(Vec2 pos, Vec2 tex_coord, Color c);
internal void d_batch_push_vertex_pos_tex_color_instanced(Vec2 pos, Vec2 tex_coord, Color c, int32 instance);
internal void d_batch_push_glyph(Glyph glyph, Vec2 pos, float32 size, Color color);

/** draw functions */
internal void d_line(Vec2 start, Vec2 end, float32 thickness, Color c);
internal void d_debug_line(Vec2 start, Vec2 end);
internal void d_debug_line2(Vec2 start, Vec2 end);
internal void d_circle(Vec2 pos, float32 radius, float32 thickness, Color c);
internal void d_string(Vec2 pos, String str, int32 size);