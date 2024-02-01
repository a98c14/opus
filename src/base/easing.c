#include "easing.h"

internal float32
ease_dynamic(float32 t, EasingType type)
{
    switch (type)
    {
    case EasingTypeEaseLinear:
        return ease_out_elastic(t);
        break;
    case EasingTypeEaseOutElastic:
        return ease_out_elastic(t);
        break;
    case EasingTypeEaseInSin:
        return ease_in_sin(t);
        break;
    default:
        return t;
    }
}

internal float32
ease_in_sin(float32 t)
{
    return 1 - cosf((t * PI_FLOAT32) / 2);
}

internal float32
ease_out_elastic(float32 t)
{
    const float32 c4 = (2 * PI_FLOAT32) / 3;

    return t <= 0   ? 0
           : t >= 1 ? 1
                    : powf(2, -10 * t) * sinf((t * 10 - 0.75) * c4) + 1;
}