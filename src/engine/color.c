#include "color.h"

internal Color
lerp_color(Color a, Color b, float32 t)
{
    Color result = {0};
    result.r = a.r * (1.0f - t) + b.r * t;
    result.g = a.g * (1.0f - t) + b.g * t;
    result.b = a.b * (1.0f - t) + b.b * t;
    result.a = a.a * (1.0f - t) + b.a * t;
    return result;
}