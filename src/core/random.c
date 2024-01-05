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