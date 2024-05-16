#pragma once
#include <base.h>
#include <gfx.h>
#include <text/text_inc.h>

#include "draw_trail.h"

internal Trail*
d_trail_new(Arena* arena)
{
    const uint32 max_trail_capacity = 1024;
    Trail*       result             = arena_push_struct_zero(arena, Trail);
    result->capacity                = max_trail_capacity;
    result->buffer                  = arena_push_array_zero(arena, TrailPoint, max_trail_capacity);
    result->t_lifetime              = 0.4;
    result->width_start             = 2;
    result->width_end               = 2;
    result->color_start             = ColorWhite;
    result->color_end               = ColorWhite;
    return result;
}

internal void
d_trail_reset(Trail* trail)
{
    trail->start = trail->end;
}

internal void
d_trail_push_position(Trail* trail, Vec2 position)
{
    trail->buffer[(trail->end) % trail->capacity].position    = position;
    trail->buffer[(trail->end) % trail->capacity].t_remaining = trail->t_lifetime;
    trail->end++;
}

internal void
d_trail_push_empty(Trail* trail)
{
    trail->buffer[(trail->end) % trail->capacity].t_remaining = 0;
    trail->end++;
}

internal void
d_mesh_generate_trail(VertexAtrribute_TexturedColored* vertex_buffer, uint64* vertex_count, Trail* trail)
{
    if (trail->end - trail->start < 1)
        return;

    for (uint32 i = trail->start; i < trail->end; i++)
    {
        TrailPoint* point      = &trail->buffer[i % trail->capacity];
        TrailPoint* prev_point = &trail->buffer[(i - 1) % trail->capacity];
        if (point->t_remaining <= 0 || prev_point->t_remaining <= 0)
            continue;

        float32 t           = clamp(0, point->t_remaining / trail->t_lifetime, 1);
        float32 width       = lerp_f32(trail->width_end, trail->width_start, t);
        Color   color       = lerp_color(trail->color_end, trail->color_start, t);
        Vec2    end_heading = heading_to_vec2(prev_point->position, point->position);
        Vec2    end_normal  = vec2(-end_heading.y, end_heading.x);

        Vec2 p_right = move_vec2(point->position, end_normal, width);
        Vec2 p_left  = move_vec2(point->position, end_normal, -width);

        if (d_trail_is_segment_endpoint(trail, i - 1))
        {
            d_mesh_push_vertex(vertex_buffer, vertex_count, prev_point->position, vec2_zero(), color);
            d_mesh_push_vertex(vertex_buffer, vertex_count, p_right, vec2_zero(), color);
            d_mesh_push_vertex(vertex_buffer, vertex_count, p_left, vec2_zero(), color);
        }
        else if (d_trail_is_segment_endpoint(trail, i))
        {
            d_mesh_push_triangle_strip(vertex_buffer, vertex_count, point->position, vec2_zero(), color);
        }
        else
        {
            d_mesh_push_triangle_strip(vertex_buffer, vertex_count, p_right, vec2_zero(), color);
            d_mesh_push_triangle_strip(vertex_buffer, vertex_count, p_left, vec2_zero(), color);
        }
    }
}

internal void
d_trail_update(Trail* trail, float32 dt)
{
    for (uint64 i = trail->start; i < trail->end; i++)
    {
        TrailPoint* tp = &trail->buffer[i % trail->capacity];
        tp->t_remaining -= dt;
        if (tp->t_remaining <= 0)
        {
            tp->t_remaining = 0;
            trail->start += i == trail->start;
        }
    }
}

internal void
d_trail_draw(Trail* trail)
{
    // TODO(selim): is the capacity correct?
    VertexAtrribute_TexturedColored* vertices     = arena_push_array(d_context->frame_arena, VertexAtrribute_TexturedColored, (trail->end - trail->start) * 6);
    uint64                           vertex_count = 0;
    d_mesh_generate_trail(vertices, &vertex_count, trail);

    R_Batch batch;
    batch.key                 = render_key_new(d_context->active_view, d_context->active_layer, d_context->active_pass, TEXTURE_INDEX_NULL, MeshTypeDynamic, d_context->material_basic);
    batch.element_count       = 1;
    batch.draw_instance_count = vertex_count;
    batch.vertex_buffer       = vertices;
    batch.vertex_buffer_size  = sizeof(VertexAtrribute_TexturedColored) * vertex_count;
    batch.uniform_buffer      = 0;
    r_batch_commit(batch);
}

internal void
d_trail_set_color(Trail* trail, Color start, Color end)
{
    trail->color_start = start;
    trail->color_end   = end;
}

internal void
d_trail_set_width(Trail* trail, float32 start, float32 end)
{
    trail->width_start = start;
    trail->width_end   = end;
}

internal bool32
d_trail_is_segment_endpoint(Trail* trail, uint32 index)
{
    return index == trail->start ||
           index == trail->end - 1 ||
           trail->buffer[(index - 1) % trail->capacity].t_remaining <= 0 ||
           trail->buffer[(index + 1) % trail->capacity].t_remaining <= 0;
}