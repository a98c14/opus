#pragma once

#include "defines.h"
#include <pcg/pcg_basic.h>

global pcg32_random_t g_rng;

internal void    random_init(uint64 seed);
internal float32 random_between_f32(float32 min, float32 max);
internal float32 random_f32(float32 max);