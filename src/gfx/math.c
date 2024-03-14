#include "math.h"
#include <base/math.h>
#include <gfx/sprite.h>

internal Mat4
mat4_mvp(Mat4 model, Mat4 view, Mat4 projection)
{
    Mat4 result;
    result = mul_mat4(view, model);
    result = mul_mat4(projection, result);
    return result;
}

/* Transforms */
internal Mat4
transform_quad(Vec2 position, Vec2 scale, float32 rotation)
{
    Mat4 translation_mat = mat4_translation(vec3_xy_z(position, 0));
    Mat4 rotation_mat    = mat4_rotation(rotation);
    Mat4 scale_mat       = mat4_scale(vec3_xy_z(scale, 0));
    return mat4_transform(translation_mat, rotation_mat, scale_mat);
}

internal Mat4
transform_quad_around_pivot(Vec2 position, Vec2 scale, float32 rotation, Vec2 pivot)
{
    Mat4 translation_mat = mat4_translation(vec3_xy_z(position, 0));
    Mat4 rotation_mat    = mat4_rotation(rotation);
    Mat4 scale_mat       = mat4_scale(vec3_xy_z(scale, 0));
    Mat4 pivot_mat       = mat4_translation(vec3_xy_z(pivot, 0));

    Mat4 model = translation_mat;
    model      = mul_mat4(model, rotation_mat);
    model      = mul_mat4(model, pivot_mat);
    model      = mul_mat4(model, scale_mat);
    return model;
}

internal Mat4
transform_quad_aligned(Vec2 position, Vec2 scale)
{
    Mat4 result    = mat4_identity();
    result.m[0][0] = scale.x;
    result.m[1][1] = scale.y;
    result.m[2][2] = 0;
    result.m[3][0] = position.x;
    result.m[3][1] = position.y;
    result.m[3][2] = 0;
    return result;
}

internal Mat4
transform_line(Vec2 start, Vec2 end, float32 thickness)
{
    Vec2    center = lerp_vec2(start, end, 0.5f);
    float32 dist   = dist_vec2(end, start);
    float32 angle  = angle_vec2(sub_vec2(end, start));
    return transform_quad(center, vec2(dist, thickness), angle);
}

internal Mat4
transform_line_rotated(Vec2 position, float32 length, float32 angle, float32 thickness)
{
    // TODO: these values are also calculated in `mat4_rotation`, check if
    // they are optimized away or not.
    float32 radian = angle * PI_FLOAT32 / 180.0;
    float32 cosx   = (float32)cosf(radian) * (length / 2.0f);
    float32 sinx   = (float32)sinf(radian) * (length / 2.0f);
    position.x += cosx;
    position.y += sinx;
    return transform_quad(position, vec2(length, thickness), angle);
}

// TODO(selim): Can we move this to vertex shader?
internal VertexBuffer
gfx_generate_trail_vertices(Arena* arena, Vec2* points, uint32 point_count, float32 trail_width)
{
    if (point_count < 2)
        return (VertexBuffer){0};

    Vec2 start         = points[0];
    Vec2 next          = points[1];
    Vec2 start_heading = heading_to_vec2(start, next);
    Vec2 start_normal  = vec2(-start_heading.y, start_heading.x);
    Vec2 right         = add_vec2(start, mul_vec2_f32(start_normal, trail_width));
    Vec2 left          = add_vec2(start, mul_vec2_f32(start_normal, -trail_width));

    VertexBuffer result;
    result.count = point_count * 2;
    result.v     = arena_push_array_zero(arena, Vec2, result.count);
    result.v[0]  = right;
    result.v[1]  = left;

    for (uint32 i = 2; i < point_count; i++)
    {
        Vec2 start  = points[i - 2];
        Vec2 middle = points[i - 1];
        Vec2 next   = points[i];

        Vec2 heading      = heading_to_vec2(start, middle);
        Vec2 heading_next = heading_to_vec2(next, middle);
        Vec2 normal_end   = vec2(-heading_next.y, heading_next.x);
        Vec2 end_right    = add_vec2(next, mul_vec2_f32(normal_end, trail_width));
        Vec2 end_left     = add_vec2(next, mul_vec2_f32(normal_end, -trail_width));

        Vec2 intersection_right = vec2_intersection_fast(right, heading, end_left, heading_next);
        Vec2 intersection_left  = vec2_intersection_fast(left, heading, end_right, heading_next);

        right                     = intersection_right;
        left                      = intersection_left;
        result.v[(i - 1) * 2]     = right;
        result.v[(i - 1) * 2 + 1] = left;
    }

    Vec2 end                            = points[point_count - 1];
    Vec2 prev                           = points[point_count - 2];
    Vec2 end_heading                    = heading_to_vec2(end, prev);
    Vec2 end_normal                     = vec2(-end_heading.y, end_heading.x);
    Vec2 end_right                      = add_vec2(end, mul_vec2_f32(end_normal, trail_width));
    Vec2 end_left                       = add_vec2(end, mul_vec2_f32(end_normal, -trail_width));
    result.v[(point_count - 1) * 2]     = end_left;
    result.v[(point_count - 1) * 2 + 1] = end_right;
    return result;
}