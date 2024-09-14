#pragma once
#include <base/asserts.h>
#include <base/math.h>
#include <base/memory.h>

typedef union
{
    struct
    {
        Vec2 bl;
        Vec2 tl;
        Vec2 br;
        Vec2 tr;
    };
    Vec2 v[4];
} R_Quad;

internal Mat4 mat4_mvp(Mat4 model, Mat4 view, Mat4 projection);

/* Transforms */
internal Mat4 transform_quad(Vec2 position, Vec2 scale, float32 rotation);
internal Mat4 transform_quad_around_pivot(Vec2 position, Vec2 scale, float32 rotation, Vec2 pivot);

/* If the quad is axis aligned (no rotation) this can be used
instead of `transform_quad` for better performance */
internal Mat4 transform_quad_aligned(Vec2 position, Vec2 scale);
internal Mat4 transform_quad_aligned_at_pivot(Vec2 position, Vec2 scale, Vec2 pivot);
internal Mat4 transform_line(Vec2 start, Vec2 end, float32 thickness);
internal Mat4 transform_line_rotated(Vec2 position, float32 length, float32 angle, float32 thickness);
