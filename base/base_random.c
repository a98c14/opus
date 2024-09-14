#include "base_random.h"
#include <opus/third_party/pcg/pcg_basic.c>

// TODO(selim): add non-global random functions
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

internal int32
random_between_i32(int32 min, int32 max)
{
    return min + pcg32_boundedrand_r(&g_rng, (max - min));
}

internal uint32
random_uint32()
{
    return pcg32_boundedrand_r(&g_rng, MAX_INT32);
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
random_direction(float32 radius)
{
    float32 angle = random_between_f32(-180, 180);
    float32 scale = random_between_f32(0, radius);
    return direction_vec2(angle, scale);
}

internal Vec2
random_heading()
{
    float32 angle = random_between_f32(-180, 180);
    return direction_vec2(angle, 1);
}

internal Vec2
random_point_between_circle(Vec2 center, float32 min_radius, float32 max_radius)
{
    float32 angle = random_between_f32(-180, 180);
    float32 scale = random_between_f32(min_radius, max_radius);
    Vec2    v     = direction_vec2(angle, scale);
    return add_vec2(center, v);
}

internal void
random_shuffle(int32* numbers, uint32 count)
{
    for (int32 i = count - 1; i > 0; i--)
    {
        int j      = random_uint32() % (i + 1);
        int temp   = numbers[i];
        numbers[i] = numbers[j];
        numbers[j] = temp;
    }
}
