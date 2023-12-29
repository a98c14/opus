#pragma once
#include <core/math.h>
#include <core/asserts.h>


internal Mat4
mat4_mvp(Mat4 model, Mat4 view, Mat4 projection);

/* Transforms */
internal Mat4
transform_quad(Vec2 position, Vec2 scale, float32 rotation);

/* If the quad is axis aligned (no rotation) this can be used
instead of `transform_quad` for better performance */
internal Mat4
transform_quad_aligned(Vec3 position, Vec2 scale);

internal Mat4
transform_line(Vec2 start, Vec2 end, float32 thickness);

internal Mat4
transform_line_rotated(Vec2 position, float32 length, float32 angle, float32 thickness);