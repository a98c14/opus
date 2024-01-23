#include "random.h"
#include <pcg/pcg_basic.c>

internal void
random_init(uint64 seed)
{
    pcg32_srandom_r(&g_rng, seed, 54u);
}

internal float32
random_between_f32(float32 min, float32 max)
{
    const float32 resolution = 8192.0f;
    return min + pcg32_boundedrand_r(&g_rng, resolution * (max - min)) / resolution;
}

internal float32
random_f32(float32 max)
{
    const float32 resolution = 8192.0f;
    return pcg32_boundedrand_r(&g_rng, resolution * max) / resolution;
}

internal Vec2
random_point_in_circle(Vec2 center, float32 radius)
{
    float32 angle = random_between_f32(-180, 180);
    float32 scale = random_between_f32(0, radius);
    Vec2    v     = direction_vec2(angle, scale);
    return add_vec2(center, v);
}

internal Vec2
random_point_between_circle(Vec2 center, float32 min_radius, float32 max_radius)
{
    float32 angle = random_between_f32(-180, 180);
    float32 scale = random_between_f32(min_radius, max_radius);
    Vec2    v     = direction_vec2(angle, scale);
    return add_vec2(center, v);
}