#pragma once

#include "defines.h"
#include "math.h"

typedef enum
{
    EasingTypeEaseLinear,
    EasingTypeEaseInSin,
    EasingTypeEaseOutElastic,
} EasingType;

internal float32 ease_dynamic(float32 t, EasingType type);
internal float32 ease_out_elastic(float32 t);
internal float32 ease_in_sin(float32 t);