#include "color.h"

internal uint8
color_red(Color c)
{
    return (c >> 24 & ((1 << 8) - 1));
}

internal uint8
color_green(Color c)
{
    return (c >> 16 & ((1 << 8) - 1));
}

internal uint8
color_blue(Color c)
{
    return (c >> 8 & ((1 << 8) - 1));
}

internal uint8
color_alpha(Color c)
{
    return (c >> 0 & ((1 << 8) - 1));
}

internal Color
lerp_color(Color a, Color b, float32 t)
{
    return (uint64)a * (1.0f - t) + (uint64)b * t;
}