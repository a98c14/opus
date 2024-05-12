#pragma once
#include <base.h>
#include <gfx.h>

#include "draw_shaders.h"

typedef struct
{
    /** state */
    PassIndex      active_pass;
    SortLayerIndex active_layer;

    /** materials */
    MaterialIndex material_basic;
    MaterialIndex material_sprite;
    MaterialIndex material_text;
} D_Context;
D_Context* d_context;

internal void d_context_init(Arena* persistent_arena);

/** draw functions */
internal void d_line(Vec2 start, Vec2 end, float32 thickness, Color c);