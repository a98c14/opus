#pragma once

#include "defines.h"
#include "math.h"
#include <pcg/pcg_basic.h>

global pcg32_random_t g_rng;

internal void    random_init(uint64 seed);
internal float32 random_between_f32(float32 min, float32 max);
internal int32   random_between_i32(int32 min, int32 max);
internal uint32  random_uint32();
internal float32 random_f32(float32 max);
internal Vec2    random_point_in_circle(Vec2 center, float32 radius);
internal Vec2    random_point_between_circle(Vec2 center, float32 min_radius, float32 max_radius);
internal void    random_shuffle(int32* numbers, uint32 count);
internal Vec2    random_direction(float32 radius);