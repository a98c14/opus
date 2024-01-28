#include "math.h"
#include <base/math.h>

/* Constructors */
internal Vec2
vec2(float32 x, float32 y)
{
    Vec2 result;
    result.x = x;
    result.y = y;
    return result;
}

internal Vec2
vec2_zero()
{
    Vec2 result;
    result.x = 0;
    result.y = 0;
    return result;
}

internal Vec2
vec2_one()
{
    Vec2 result;
    result.x = 1;
    result.y = 1;
    return result;
}

internal Vec2
vec2_up()
{
    Vec2 result;
    result.x = 0;
    result.y = 1;
    return result;
}

internal Vec2
vec2_right()
{
    Vec2 result;
    result.x = 1;
    result.y = 0;
    return result;
}

internal Vec2
heading_vec2(float32 angle)
{
    float32 radian = angle * PI_FLOAT32 / 180.0;
    float32 cosx   = (float32)cosf(radian);
    float32 sinx   = (float32)sinf(radian);

    Vec2 result;
    result.x = cosx;
    result.y = sinx;
    return result;
}

internal Vec2
heading_inverse_vec2(float32 angle)
{
    return mul_vec2_f32(heading_vec2(angle), -1);
}

internal Vec2
direction_vec2(float32 angle, float32 scale)
{
    return mul_vec2_f32(heading_vec2(angle), scale);
}

internal Vec2
direction_inverse_vec2(float32 angle, float32 scale)
{
    return mul_vec2_f32(heading_inverse_vec2(angle), scale);
}

internal Vec3
vec3(float32 x, float32 y, float32 z)
{
    Vec3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

internal Vec3
vec3_zero()
{
    Vec3 result;
    result.x = 0;
    result.y = 0;
    result.z = 0;
    return result;
}

internal Vec3
vec3_one()
{
    Vec3 result;
    result.x = 1;
    result.y = 1;
    result.z = 1;
    return result;
}

internal Vec3
vec3_xy_z(Vec2 xy, float32 z)
{
    Vec3 result;
    result.xy = xy;
    result.z  = z;
    return result;
}

internal Vec4
vec4(float32 x, float32 y, float32 z, float32 w)
{
    Vec4 result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return result;
}

internal Rect
rect(float32 x, float32 y, float32 w, float32 h)
{
    Rect result;
    result.x = x;
    result.y = y;
    result.w = w;
    result.h = h;
    return result;
}

internal Rect
rect_from_bl_tr(Vec2 bl, Vec2 tr)
{
    Rect    result;
    float32 w = tr.x - bl.x;
    float32 h = tr.y - bl.y;
    result.x  = bl.x + w / 2.0f;
    result.y  = bl.y + h / 2.0f;
    result.w  = w;
    result.h  = h;
    return result;
}

/* retuns a rect that is positioned at origin with given width and height */
internal Rect
rect_from_wh(float32 w, float32 h)
{
    Rect result = {0};
    result.w    = w;
    result.h    = h;
    return result;
}

internal Circle
circle(Vec2 center, float32 radius)
{
    Circle result;
    result.center = center;
    result.radius = radius;
    return result;
}

internal Range
range(float32 min, float32 max)
{
    Range result;
    result.min = min;
    result.max = max;
    return result;
}

/* Basic Operations */
internal Vec2
add_vec2(Vec2 a, Vec2 b)
{
    Vec2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

internal Vec3
add_vec3(Vec3 a, Vec3 b)
{
    Vec3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

internal Vec2
add_vec2_f32(Vec2 a, float32 b)
{
    Vec2 result;
    result.x = a.x + b;
    result.y = a.y + b;
    return result;
}

internal Vec4
add_vec4(Vec4 a, Vec4 b)
{
    Vec4 result;
#ifdef OPUS_USE_SSE
    result.SSE = _mm_add_ps(a.SSE, b.SSE);
#else
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
#endif
    return result;
}

internal Vec2
sub_vec2(Vec2 a, Vec2 b)
{
    Vec2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

internal Vec3
sub_vec3(Vec3 a, Vec3 b)
{
    Vec3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

internal Vec4
sub_vec4(Vec4 a, Vec4 b)
{
    Vec4 result;
#ifdef OPUS_USE_SSE
    result.SSE = _mm_sub_ps(a.SSE, b.SSE);
#else
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;
#endif
    return result;
}

internal Vec2
mul_vec2_f32(Vec2 a, float32 b)
{
    Vec2 result;
    result.x = a.x * b;
    result.y = a.y * b;
    return result;
}

internal Vec3
mul_vec3_f32(Vec3 a, float32 b)
{
    Vec3 result;
    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;
    return result;
}

internal Vec2
mul_mat2_vec2(Mat2 m, Vec2 v)
{
    Vec2 result;
    result.x = v.elements[0] * m.columns[0].x;
    result.y = v.elements[0] * m.columns[0].y;
    result.x += v.elements[1] * m.columns[1].x;
    result.y += v.elements[1] * m.columns[1].y;
    return result;
}

internal Vec3
mul_mat3_vec3(Mat3 m, Vec3 v)
{
    Vec3 result;
    result.x = v.elements[0] * m.columns[0].x;
    result.y = v.elements[0] * m.columns[0].y;
    result.z = v.elements[0] * m.columns[0].z;

    result.x += v.elements[1] * m.columns[1].x;
    result.y += v.elements[1] * m.columns[1].y;
    result.z += v.elements[1] * m.columns[1].z;

    result.x += v.elements[2] * m.columns[2].x;
    result.y += v.elements[2] * m.columns[2].y;
    result.z += v.elements[2] * m.columns[2].z;
    return result;
}

internal Mat3
mul_mat3(Mat3 a, Mat3 b)
{
    Mat3 result;
    result.columns[0] = mul_mat3_vec3(a, b.columns[0]);
    result.columns[1] = mul_mat3_vec3(a, b.columns[1]);
    result.columns[2] = mul_mat3_vec3(a, b.columns[2]);
    return result;
}

internal Mat4
mul_mat4(Mat4 a, Mat4 b)
{
    Mat4 result;
    result.columns[0] = linear_combine_v4_m4(b.columns[0], a);
    result.columns[1] = linear_combine_v4_m4(b.columns[1], a);
    result.columns[2] = linear_combine_v4_m4(b.columns[2], a);
    result.columns[3] = linear_combine_v4_m4(b.columns[3], a);
    return result;
}

internal Vec4
mul_mat4_vec4(Mat4 m, Vec4 v)
{
    return linear_combine_v4_m4(v, m);
}

internal Vec2
div_vec2_f32(Vec2 a, float32 b)
{
    Vec2 result;
    result.x = a.x / b;
    result.y = a.y / b;
    return result;
}

internal float32
lerp_f32(float32 a, float32 b, float32 t)
{
    return (a * (1 - t)) + (b * t);
}

internal Vec2
lerp_vec2(Vec2 a, Vec2 b, float32 t)
{
    Vec2 result;
    result.x = (a.x * (1 - t)) + (b.x * t);
    result.y = (a.y * (1 - t)) + (b.y * t);
    return result;
}

internal Vec3
lerp_vec3(Vec3 a, Vec3 b, float32 t)
{
    Vec3 result;
    result.x = (a.x * (1 - t)) + (b.x * t);
    result.y = (a.y * (1 - t)) + (b.y * t);
    result.z = (a.z * (1 - t)) + (b.z * t);
    return result;
}

internal Vec4
lerp_vec4(Vec4 a, Vec4 b, float32 t)
{
    Vec4 result;
    result.x = (a.x * (1 - t)) + (b.x * t);
    result.y = (a.y * (1 - t)) + (b.y * t);
    result.z = (a.z * (1 - t)) + (b.z * t);
    result.w = (a.w * (1 - t)) + (b.w * t);
    return result;
}

internal Rect
lerp_rect(Rect a, Rect b, float32 t)
{
    Rect result;
    result.x = (a.x * (1 - t)) + (b.x * t);
    result.y = (a.y * (1 - t)) + (b.y * t);
    result.w = (a.w * (1 - t)) + (b.w * t);
    result.h = (a.h * (1 - t)) + (b.h * t);
    return result;
}

/* Vector Operations */
internal float32
dot_vec2(Vec2 a, Vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

internal float32
dot_vec3(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

internal Vec2
norm_vec2(Vec2 a)
{
    return mul_vec2_f32(a, 1.0f / sqrtf(dot_vec2(a, a)));
}

internal Vec2
norm_vec2_safe(Vec2 a)
{
    if (fabs(a.x) < EPSILON_FLOAT32 && fabs(a.y) < EPSILON_FLOAT32)
        return vec2_zero();

    return mul_vec2_f32(a, 1.0f / sqrtf(dot_vec2(a, a)));
}

internal Vec3
norm_vec3(Vec3 a)
{
    return mul_vec3_f32(a, 1.0f / sqrtf(dot_vec3(a, a)));
}

internal float32
lensqr_vec2(Vec2 a)
{
    return dot_vec2(a, a);
}

internal float32
len_vec2(Vec2 a)
{
    return sqrtf(lensqr_vec2(a));
}

internal float32
distsqr_vec2(Vec2 a, Vec2 b)
{
    return lensqr_vec2(sub_vec2(b, a));
}

internal float32
dist_vec2(Vec2 a, Vec2 b)
{
    return len_vec2(sub_vec2(b, a));
}

internal float32
angle_vec2(Vec2 v)
{
    Vec2    right = vec2_right();
    float32 dot   = dot_vec2(right, v);
    float32 det   = right.x * v.y - right.y * v.x;
    return (float32)atan2(det, dot) * 180.0 / PI_FLOAT32;
}

internal float32
angle_between_vec2(Vec2 a, Vec2 b)
{
    // do we need to normalize the vector?
    float32 dot   = dot_vec2(b, a);
    float32 det   = b.x * a.y - b.y * a.x;
    float32 angle = (float32)atan2(det, dot) * 180.0 / PI_FLOAT32;
    return angle;
}

internal Vec2
heading_to_vec2(Vec2 start, Vec2 end)
{
    return norm_vec2_safe(sub_vec2(end, start));
}

internal Vec2
inverse_heading_to_vec2(Vec2 start, Vec2 end)
{
    return mul_vec2_f32(heading_to_vec2(start, end), -1);
}

internal Vec2
direction_to_vec2(Vec2 start, Vec2 end, float32 scale)
{
    return mul_vec2_f32(heading_to_vec2(start, end), scale);
}

internal Vec2
inverse_direction_to_vec2(Vec2 start, Vec2 end, float32 scale)
{
    return mul_vec2_f32(heading_to_vec2(end, start), -scale);
}

internal Vec2
move_towards_vec2(Vec2 from, Vec2 to, float32 length)
{
    return add_vec2(from, direction_to_vec2(from, to, length));
}

internal Vec2
rotate_vec2(Vec2 v, float angle)
{
    Mat2 rotation = mat2_rotation(angle);
    return mul_mat2_vec2(rotation, v);
}

/* Matrix Operations */
internal Mat2
mat2_identity(void)
{
    Mat2 result    = {0};
    result.m[0][0] = 1;
    result.m[1][1] = 1;
    return result;
}

internal Mat2
mat2_rotation(float angle)
{
    float32 radian = angle * PI_FLOAT32 / 180.0;
    float32 cosx   = (float32)cos(radian);
    float32 sinx   = (float32)sin(radian);

    Mat2 result    = {0};
    result.m[0][0] = cosx;
    result.m[0][1] = sinx;
    result.m[1][0] = -sinx;
    result.m[1][1] = cosx;
    return result;
}

internal Mat2
mat2_transpose(Mat2 m)
{
    Mat2 result    = m;
    result.m[0][1] = m.m[1][0];
    result.m[1][0] = m.m[0][1];
    return result;
}

internal Mat3
mat3_identity(void)
{
    Mat3 result    = {0};
    result.m[0][0] = 1;
    result.m[1][1] = 1;
    result.m[2][2] = 1;
    return result;
}

/* Rotates around Z axis. */
internal Mat3
mat3_rotation(float angle)
{
    float32 radian = angle * PI_FLOAT32 / 180.0;
    float32 cosx   = (float32)cos(radian);
    float32 sinx   = (float32)sin(radian);

    Mat3 result    = {0};
    result.m[0][0] = cosx;
    result.m[0][1] = sinx;
    result.m[1][0] = -sinx;
    result.m[1][1] = cosx;
    result.m[2][2] = 1;
    return result;
}

internal Mat3
mat3_rotate(Mat3 m, float32 angle)
{
    Mat3 rotation = mat3_rotation(angle);
    Mat3 result   = mul_mat3(m, rotation);
    return result;
}

internal Mat3
mat3_transpose(Mat3 m)
{
    Mat3 result    = m;
    result.m[0][1] = m.m[1][0];
    result.m[0][2] = m.m[2][0];
    result.m[1][0] = m.m[0][1];
    result.m[1][2] = m.m[2][1];
    result.m[2][1] = m.m[1][2];
    result.m[2][0] = m.m[0][2];
    return result;
}

internal Mat4
mat4_identity()
{
    Mat4 result    = {0};
    result.m[0][0] = 1;
    result.m[1][1] = 1;
    result.m[2][2] = 1;
    result.m[3][3] = 1;
    return result;
}

internal Mat4
mat4_translation(Vec3 v)
{
    Mat4 result    = mat4_identity();
    result.m[3][0] = v.x;
    result.m[3][1] = v.y;
    result.m[3][2] = v.z;
    return result;
}

internal Mat4
mat4_inverse(Mat4 m)
{
    Mat4 result    = {0};
    result.m[0][0] = 1.0f / m.m[0][0];
    result.m[1][1] = 1.0f / m.m[1][1];
    result.m[2][2] = 1.0f / m.m[2][2];
    result.m[3][3] = 1.0f;

    result.m[3][0] = -m.m[3][0] * result.m[0][0];
    result.m[3][1] = -m.m[3][1] * result.m[1][1];
    result.m[3][2] = -m.m[3][2] * result.m[2][2];
    return result;
}

internal Mat4
mat4_scale(Vec3 v)
{
    Mat4 result    = mat4_identity();
    result.m[0][0] = v.x;
    result.m[1][1] = v.y;
    result.m[2][2] = v.z;
    return result;
}

internal Mat4
mat4_rotation(float32 angle)
{
    float32 radian = angle * PI_FLOAT32 / 180.0;
    float32 cosx   = (float32)cosf(radian);
    float32 sinx   = (float32)sinf(radian);
    Mat4    result = mat4_identity();
    result.m[0][0] = cosx;
    result.m[0][1] = sinx;
    result.m[1][0] = -sinx;
    result.m[1][1] = cosx;
    return result;
}

internal Mat4
mat4_transform(Mat4 translation, Mat4 rotation, Mat4 scale)
{
    Mat4 result;
    result = mul_mat4(translation, rotation);
    result = mul_mat4(result, scale);
    return result;
}

internal Mat4
mat4_ortho(float32 width, float32 height, float32 near_plane, float32 far_plane)
{
    float32 right  = width / 2.0;
    float32 left   = -width / 2.0;
    float32 top    = height / 2.0;
    float32 bottom = -height / 2.0;

    Mat4 result    = {0};
    result.m[0][0] = 2.0 / (right - left);
    result.m[1][1] = 2.0 / (top - bottom);
    result.m[2][2] = -2.0 / (far_plane - near_plane);
    result.m[0][3] = -(right + left) / (right - left);
    result.m[1][3] = -(top + bottom) / (top - bottom);
    result.m[2][3] = -(far_plane + near_plane) / (far_plane - near_plane);
    result.m[3][3] = 1.0;
    return result;
}

internal Vec4
linear_combine_v4_m4(Vec4 v, Mat4 m)
{
    Vec4 result;
#ifdef OPUS_USE_SSE
    result.SSE = _mm_mul_ps(_mm_shuffle_ps(v.SSE, v.SSE, 0x00), m.columns[0].SSE);
    result.SSE = _mm_add_ps(result.SSE, _mm_mul_ps(_mm_shuffle_ps(v.SSE, v.SSE, 0x55), m.columns[1].SSE));
    result.SSE = _mm_add_ps(result.SSE, _mm_mul_ps(_mm_shuffle_ps(v.SSE, v.SSE, 0xaa), m.columns[2].SSE));
    result.SSE = _mm_add_ps(result.SSE, _mm_mul_ps(_mm_shuffle_ps(v.SSE, v.SSE, 0xff), m.columns[3].SSE));
#else
    result.x = v.elements[0] * m.columns[0].x;
    result.y = v.elements[0] * m.columns[0].y;
    result.z = v.elements[0] * m.columns[0].z;
    result.w = v.elements[0] * m.columns[0].w;

    result.x += v.elements[1] * m.columns[1].x;
    result.y += v.elements[1] * m.columns[1].y;
    result.z += v.elements[1] * m.columns[1].z;
    result.w += v.elements[1] * m.columns[1].w;

    result.x += v.elements[2] * m.columns[2].x;
    result.y += v.elements[2] * m.columns[2].y;
    result.z += v.elements[2] * m.columns[2].z;
    result.w += v.elements[2] * m.columns[2].w;

    result.x += v.elements[3] * m.columns[3].x;
    result.y += v.elements[3] * m.columns[3].y;
    result.z += v.elements[3] * m.columns[3].z;
    result.w += v.elements[3] * m.columns[3].w;
#endif
    return result;
}

internal float32
rect_left(Rect rect)
{
    return rect.x - rect.w / 2.0f;
}

internal float32
rect_right(Rect rect)
{
    return rect.x + rect.w / 2.0f;
}

internal float32
rect_bottom(Rect rect)
{
    return rect.y - rect.h / 2.0f;
}

internal float32
rect_top(Rect rect)
{
    return rect.y + rect.h / 2.0f;
}

/* Bottom Left */
internal Vec2
rect_bl(Rect rect)
{
    return vec2(rect.x - rect.w / 2.0f, rect.y - rect.h / 2.0f);
}

/* Bottom Right */
internal Vec2
rect_br(Rect rect)
{
    return vec2(rect.x + rect.w / 2.0f, rect.y - rect.h / 2.0f);
}

/* Top Right */
internal Vec2
rect_tr(Rect rect)
{
    return vec2(rect.x + rect.w / 2.0f, rect.y + rect.h / 2.0f);
}

/* Top Left */
internal Vec2
rect_tl(Rect rect)
{
    return vec2(rect.x - rect.w / 2.0f, rect.y + rect.h / 2.0f);
}

/* Center Left */
internal Vec2
rect_cl(Rect rect)
{
    return vec2(rect.x - rect.w / 2.0f, rect.y);
}

/* Center Right */
internal Vec2
rect_cr(Rect rect)
{
    return vec2(rect.x + rect.w / 2.0f, rect.y);
}

/* Center Bottom */
internal Vec2
rect_cb(Rect rect)
{
    return vec2(rect.x, rect.y - rect.h / 2.0f);
}

/* Center Top */
internal Vec2
rect_ct(Rect rect)
{
    return vec2(rect.x, rect.y + rect.h / 2.0f);
}

internal void
b32_flip(bool32* b)
{
    *b = !(*b);
}

internal Bounds
bounds_shrink(Bounds bounds, Vec2 amount)
{
    Bounds result = bounds;
    result.bl.x += amount.x / 2.0;
    result.bl.y += amount.y / 2.0;
    result.tr.x -= amount.x / 2.0;
    result.tr.y -= amount.y / 2.0;
    return result;
}

internal Bounds
bounds_from_circle(Circle c)
{
    Bounds result;
    result.bl.x = c.center.x - c.radius / 2.0f;
    result.bl.y = c.center.y - c.radius / 2.0f;
    result.tr.x = c.center.x + c.radius / 2.0f;
    result.tr.y = c.center.y + c.radius / 2.0f;
    return result;
}