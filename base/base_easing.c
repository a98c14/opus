#include "base_easing.h"

internal float32
ease_dynamic(float32 t, EasingType type)
{
    switch (type)
    {
    case EasingTypeEaseLinear:
        return ease_linear(t);
        break;
    case EasingTypeEaseOutElastic:
        return ease_out_elastic(t);
        break;
    case EasingTypeEaseInSin:
        return ease_in_sin(t);
        break;
    case EasingTypeEaseOutSin:
        return ease_out_sin(t);
        break;
    case EasingTypeEaseInCubic:
        return ease_in_cubic(t);
        break;
    case EasingTypeEaseOutCubic:
        return ease_out_cubic(t);
        break;
    default:
        return t;
    }
}

internal float32
ease_linear(float32 t)
{
    return t;
}

internal float32
ease_in_sin(float32 t)
{
    return 1 - cosf((t * PI_FLOAT32) / 2);
}

internal float32
ease_out_sin(float32 t)
{
    return sinf((t * PI_FLOAT32) / 2);
}

internal float32
ease_out_elastic(float32 t)
{
    const float32 c4 = (2 * PI_FLOAT32) / 3;

    return t <= 0   ? 0
           : t >= 1 ? 1
                    : powf(2, -10 * t) * sinf((t * 10 - 0.75) * c4) + 1;
}

internal float32
ease_in_cubic(float32 t)
{
    return t * t * t;
}

internal float32
ease_out_cubic(float32 t)
{
    return 1 - powf(1 - t, 3);
}
