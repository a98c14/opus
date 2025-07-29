#include "base_math.h"

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
vec2_zero(void)
{
    Vec2 result;
    result.x = 0;
    result.y = 0;
    return result;
}

internal Vec2
vec2_one(void)
{
    Vec2 result;
    result.x = 1;
    result.y = 1;
    return result;
}

internal Vec2
vec2_up(void)
{
    Vec2 result;
    result.x = 0;
    result.y = 1;
    return result;
}

internal Vec2
vec2_right(void)
{
    Vec2 result;
    result.x = 1;
    result.y = 0;
    return result;
}

internal Vec2
heading_vec2(float32 angle)
{
    float32 radian = angle * ANGLE_TO_RAD_FLOAT32;
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
vec3_zero(void)
{
    Vec3 result;
    result.x = 0;
    result.y = 0;
    result.z = 0;
    return result;
}

internal Vec3
vec3_one(void)
{
    Vec3 result;
    result.x = 1;
    result.y = 1;
    result.z = 1;
    return result;
}

internal Vec3
vec3_x_y(float32 x, float32 y)
{
    Vec3 result;
    result.x = x;
    result.y = y;
    result.z = 0;
    return result;
}

internal Vec3
vec3_xy(Vec2 xy)
{
    Vec3 result;
    result.xy = xy;
    result.z  = 0;
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

internal Vec4
vec4_xxxx(float32 x)
{
    Vec4 result;
    result.x = x;
    result.y = x;
    result.z = x;
    result.w = x;
    return result;
}

internal IVec2
ivec2(int32 x, int32 y)
{
    IVec2 result;
    result.x = x;
    result.y = y;
    return result;
}

internal IVec2
ivec2_xy(Vec2 xy)
{
    IVec2 result;
    result.x = (int32)xy.x;
    result.y = (int32)xy.y;
    return result;
}

internal Rect
rect_from_xy_wh(float32 x, float32 y, float32 w, float32 h)
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
    float32 w = (float32)fabs(tr.x - bl.x);
    float32 h = (float32)fabs(tr.y - bl.y);
    result.x  = min(bl.x, tr.x) + w / 2.0f;
    result.y  = min(bl.y, tr.y) + h / 2.0f;
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

internal Rect
rect_flip_x(Rect rect)
{
    Rect result = rect;
    result.w *= -1;
    return result;
}

internal Rect
rect_flip_y(Rect rect)
{
    Rect result = rect;
    result.h *= -1;
    return result;
}

internal Rect
rect_scale_xy(Rect rect, float32 x_scale, float32 y_scale)
{
    Rect result = rect;
    result.x *= x_scale;
    result.w *= x_scale;
    result.y *= y_scale;
    result.h *= y_scale;
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

internal IRange
irange(int32 min, int32 max)
{
    IRange result;
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

internal Vec2
mul_vec2(Vec2 a, Vec2 b)
{
    Vec2 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
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

internal float32
inverse_lerp_f32(float32 a, float32 b, float32 value)
{
    return (value - a) / (b - a);
}

internal Vec2
inverse_lerp_vec2(Vec2 a, Vec2 b, Vec2 value)
{
    Vec2 result;
    result.x = inverse_lerp_f32(a.x, b.x, value.x);
    result.y = inverse_lerp_f32(a.y, b.y, value.y);
    return result;
}

internal float32
remap_f32(float32 imin, float32 imax, float32 omin, float32 omax, float32 v)
{
    float32 t = inverse_lerp_f32(imin, imax, v);
    return lerp_f32(omin, omax, t);
}

internal Vec2
remap_vec2(Vec2 imin, Vec2 imax, Vec2 omin, Vec2 omax, Vec2 v)
{
    Vec2 t = inverse_lerp_vec2(imin, imax, v);

    Vec2 result;
    result.x = lerp_f32(omin.x, omax.y, t.x);
    result.y = lerp_f32(omin.y, omax.y, t.y);
    return result;
}

internal float32
exp_decay_f32_(float32 a, float32 b, float32 decay, float32 dt)
{
    return b + (a - b) * expf(-decay * dt);
}

internal Vec2
exp_decay_vec2_(Vec2 a, Vec2 b, float32 decay, float32 dt)
{
    Vec2 result;
    result.x = exp_decay_f32_(a.x, b.x, decay, dt);
    result.y = exp_decay_f32_(a.y, b.y, decay, dt);
    return result;
}

internal Vec3
exp_decay_vec3_(Vec3 a, Vec3 b, float32 decay, float32 dt)
{
    Vec3 result;
    result.x = exp_decay_f32_(a.x, b.x, decay, dt);
    result.y = exp_decay_f32_(a.y, b.y, decay, dt);
    result.z = exp_decay_f32_(a.z, b.z, decay, dt);
    return result;
}

internal float32
exp_decay_f32(float32 a, float32 b, float32 dt)
{
    return exp_decay_f32_(a, b, _default_decay, dt);
}

internal Vec2
exp_decay_vec2(Vec2 a, Vec2 b, float32 dt)
{
    return exp_decay_vec2_(a, b, _default_decay, dt);
}

internal Vec3
exp_decay_vec3(Vec3 a, Vec3 b, float32 dt)
{
    return exp_decay_vec3_(a, b, _default_decay, dt);
}

/* Vector Operations */
internal float32
dot_vec2(Vec2 a, Vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

internal float32
det_vec2(Vec2 a, Vec2 b)
{
    return a.x * b.y - a.y * b.x;
}

internal float32
dot_vec3(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

internal Vec2
fabs_vec2(Vec2 a)
{
    Vec2 result;
    result.x = (float32)fabs(a.x);
    result.y = (float32)fabs(a.y);
    return result;
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

internal Vec2
clamp_vec2_length(float32 min, Vec2 a, float32 max)
{
    return mul_vec2_f32(norm_vec2_safe(a), clamp(min, len_vec2(a), max));
}

internal Vec2
clamp_vec2(Vec2 a, Rect r)
{
    Vec2 result;
    result.x = clamp(rect_left(r), a.x, rect_right(r));
    result.y = clamp(rect_bottom(r), a.y, rect_top(r));
    return result;
}

internal float32
lensqr_vec2(Vec2 a)
{
    return dot_vec2(a, a);
}

internal float32
lensqr_vec3(Vec3 a)
{
    return dot_vec3(a, a);
}

internal float32
len_vec2(Vec2 a)
{
    return sqrtf(lensqr_vec2(a));
}

internal float32
len_vec3(Vec3 a)
{
    return sqrtf(lensqr_vec3(a));
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
    return (float32)atan2(det, dot) * RAD_TO_ANGLE_FLOAT32;
}

internal float32
angle_between_vec2(Vec2 a, Vec2 b)
{
    // do we need to normalize the vector?
    float32 dot   = dot_vec2(b, a);
    float32 det   = b.x * a.y - b.y * a.x;
    float32 angle = (float32)atan2(det, dot) * RAD_TO_ANGLE_FLOAT32;
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
move_vec2(Vec2 from, Vec2 heading, float32 length)
{
    return add_vec2(from, mul_vec2_f32(heading, length));
}

internal Vec2
rotate_vec2(Vec2 v, float angle)
{
    Mat2 rotation = mat2_rotation(angle);
    return mul_mat2_vec2(rotation, v);
}

internal Vec2
rotate90_vec2(Vec2 v)
{
    return vec2(-v.y, v.x);
}

internal Vec2
rotate90i_vec2(Vec2 v)
{
    return vec2(v.y, -v.x);
}

/** Quad */
internal Quad
quad_at(Vec2 pos, Vec2 size, float32 rotation)
{
    return quad_from_rect(rect_from_xy_wh(pos.x, pos.y, size.x, size.h), rotation);
}

internal Quad
quad_from_rect(Rect r, float32 rotation)
{
    Mat2 rotation_matrix = mat2_rotation(rotation);
    Quad result;
    result.vertices[QuadVertexIndexBottomLeft]  = add_vec2(r.center, mul_mat2_vec2(rotation_matrix, vec2(-r.w / 2.0f, -r.h / 2.0f)));
    result.vertices[QuadVertexIndexTopRight]    = add_vec2(r.center, mul_mat2_vec2(rotation_matrix, vec2(r.w / 2.0f, r.h / 2.0f)));
    result.vertices[QuadVertexIndexTopLeft]     = add_vec2(r.center, mul_mat2_vec2(rotation_matrix, vec2(-r.w / 2.0f, r.h / 2.0f)));
    result.vertices[QuadVertexIndexBottomRight] = add_vec2(r.center, mul_mat2_vec2(rotation_matrix, vec2(r.w / 2.0f, -r.h / 2.0f)));

    result.normals[QuadNormalIndexHorizontal] = rotate90_vec2(heading_to_vec2(result.vertices[QuadVertexIndexBottomLeft], result.vertices[QuadVertexIndexTopLeft]));
    result.normals[QuadNormalIndexVertical]   = rotate90_vec2(heading_to_vec2(result.vertices[QuadVertexIndexTopLeft], result.vertices[QuadVertexIndexTopRight]));
    return result;
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
    float32 radian = angle * ANGLE_TO_RAD_FLOAT32;
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

internal Mat4
mat4_transpose(Mat4 m)
{
    Mat4 result;
#ifdef OPUS_USE_SSE
    result = m;
    _MM_TRANSPOSE4_PS(result.columns[0].SSE, result.columns[1].SSE, result.columns[2].SSE, result.columns[3].SSE);
#else
    result.m[0][0] = m.m[0][0];
    result.m[0][1] = m.m[1][0];
    result.m[0][2] = m.m[2][0];
    result.m[0][3] = m.m[3][0];
    result.m[1][0] = m.m[0][1];
    result.m[1][1] = m.m[1][1];
    result.m[1][2] = m.m[2][1];
    result.m[1][3] = m.m[3][1];
    result.m[2][0] = m.m[0][2];
    result.m[2][1] = m.m[1][2];
    result.m[2][2] = m.m[2][2];
    result.m[2][3] = m.m[3][2];
    result.m[3][0] = m.m[0][3];
    result.m[3][1] = m.m[1][3];
    result.m[3][2] = m.m[2][3];
    result.m[3][3] = m.m[3][3];
#endif

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
    float32 radian = angle * ANGLE_TO_RAD_FLOAT32;
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
mat4_identity(void)
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
    float32 radian = angle * ANGLE_TO_RAD_FLOAT32;
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
    float32 right  = width / 2.0f;
    float32 left   = -width / 2.0f;
    float32 top    = height / 2.0f;
    float32 bottom = -height / 2.0f;

    Mat4 result    = {0};
    result.m[0][0] = 2.0f / (right - left);
    result.m[1][1] = 2.0f / (top - bottom);
    result.m[2][2] = -2.0f / (far_plane - near_plane);
    result.m[0][3] = -(right + left) / (right - left);
    result.m[1][3] = -(top + bottom) / (top - bottom);
    result.m[2][3] = -(far_plane + near_plane) / (far_plane - near_plane);
    result.m[3][3] = 1.0f;
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

internal Vec2
closest_point_on_line(Vec2 start, Vec2 end, Vec2 point)
{
    Vec2 d = sub_vec2(end, start);
    Vec2 result;
    result = add_vec2(start, mul_vec2_f32(d, clamp(0, dot_vec2(sub_vec2(point, start), d) / dot_vec2(d, d), 1)));
    return result;
}

/** simple ray intersection, doesn't account for edge cases */
internal Vec2
vec2_intersection_fast(Vec2 a, Vec2 heading_a, Vec2 b, Vec2 heading_b)
{
    Vec2    d   = sub_vec2(b, a);
    float32 det = det_vec2(heading_b, heading_a);
    if (fabs(det) < EPSILON_FLOAT32)
        return a;
    float32 u = (d.y * heading_b.x - d.x * heading_b.y) / det;
    return add_vec2(a, mul_vec2_f32(heading_a, u));
}

internal Vec2
vec2_scale(Vec2 a, float32 s)
{
    return mul_vec2_f32(a, s);
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

internal Vec2
bounds_bl(Bounds bounds)
{
    return vec2(bounds.left, bounds.bottom);
}

internal Vec2
bounds_br(Bounds bounds)
{
    return vec2(bounds.right, bounds.bottom);
}

internal Vec2
bounds_tr(Bounds bounds)
{
    return vec2(bounds.right, bounds.top);
}

internal Vec2
bounds_tl(Bounds bounds)
{
    return vec2(bounds.left, bounds.top);
}

internal Vec2
bounds_cl(Bounds bounds)
{
    return vec2(bounds.left, (bounds.bottom + bounds.top) / 2.0f);
}

internal Vec2
bounds_cr(Bounds bounds)
{
    return vec2(bounds.right, (bounds.bottom + bounds.top) / 2.0f);
}

internal Vec2
bounds_cb(Bounds bounds)
{
    return vec2((bounds.left + bounds.right) / 2.0f, bounds.bottom);
}

internal Vec2
bounds_ct(Bounds bounds)
{
    return vec2((bounds.left + bounds.right) / 2.0f, bounds.bottom);
}

internal void
b32_flip(bool32* b)
{
    *b = !(*b);
}

internal Vec2
bounds_center(Bounds bounds)
{
    Vec2 result;
    result.x = (bounds.bl.x + bounds.tr.x) / 2.0f;
    result.y = (bounds.bl.y + bounds.tr.y) / 2.0f;
    return result;
}

internal Bounds
bounds_shrink(Bounds bounds, Vec2 amount)
{
    Bounds result = bounds;
    result.bl.x += amount.x / 2.0f;
    result.bl.y += amount.y / 2.0f;
    result.tr.x -= amount.x / 2.0f;
    result.tr.y -= amount.y / 2.0f;
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

internal Bounds
bounds_from_rect(Rect r)
{
    Bounds result;
    result.bl = rect_bl(r);
    result.tr = rect_tr(r);
    return result;
}

internal Bounds
quad_bounds(Quad a)
{
    Bounds result = (Bounds){.bl = vec2(MAX_FLOAT32, MAX_FLOAT32), .tr = vec2(MIN_FLOAT32, MIN_FLOAT32)};
    for (uint32 i = 0; i < QuadVertexIndexCOUNT; i++)
    {
        result.bl.x = min(result.bl.x, a.vertices[i].x);
        result.bl.y = min(result.bl.y, a.vertices[i].y);
        result.tr.x = max(result.tr.x, a.vertices[i].x);
        result.tr.y = max(result.tr.y, a.vertices[i].y);
    }
    return result;
}

/** Rounding */
internal float32
floor_to(float32 v, float32 floor_to)
{
    return (int32)(v / floor_to) * floor_to;
}