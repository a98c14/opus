#pragma once
#include <base.h>
#include <gfx.h>
#include <text/text_inc.h>

#include "draw_core.h"

typedef struct
{
    Vec2    position;
    float32 t_remaining;
} TrailPoint;

typedef struct
{
    /** trail data */
    TrailPoint* buffer;
    uint64      start;
    uint64      end;
    uint32      capacity;

    /** styling */
    float32 t_lifetime;
    float32 width_start;
    float32 width_end;
    Color   color_start;
    Color   color_end;
} Trail;

typedef struct
{
    Vec4 pos;
    Vec4 color;
} TrailVertexData;

internal void   d_mesh_generate_trail(VertexAtrribute_TexturedColored* vertex_buffer, uint64* vertex_count, Trail* trail);
internal Trail* d_trail_new(Arena* arena);
internal void   d_trail_reset(Trail* trail);
internal void   d_trail_push_position(Trail* trail, Vec2 position);
internal void   d_trail_push_empty(Trail* trail);
internal void   d_trail_update(Trail* trail, float32 dt);
internal void   d_trail_draw(Trail* trail);
internal void   d_trail_set_color(Trail* trail, Color start, Color end);
internal void   d_trail_set_width(Trail* trail, float32 start, float32 end);
internal bool32 d_trail_is_segment_endpoint(Trail* trail, uint32 index);
