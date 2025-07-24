#include "base_color.h"

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
color_set_alpha(Color c, float32 a)
{
    Color result = c & ((uint8)(a * 255));
    return result;
}

internal Color
color_lerp(Color a, Color b, float32 t)
{
    return color_from_vec4(lerp_vec4(color_v4(a), color_v4(b), t));
}

internal Vec4
color_v4(Color hex)
{
    Vec4 result;
    result.r = (hex >> 24 & ((1 << 8) - 1)) / 255.0f;
    result.g = (hex >> 16 & ((1 << 8) - 1)) / 255.0f;
    result.b = (hex >> 8 & ((1 << 8) - 1)) / 255.0f;
    result.a = (hex >> 0 & ((1 << 8) - 1)) / 255.0f;
    return result;
}

internal Color
color_from_vec4(Vec4 c)
{
    uint32 r = (uint8)(c.r * 255.f) << 24;
    uint32 g = (uint8)(c.g * 255.f) << 16;
    uint32 b = (uint8)(c.b * 255.f) << 8;
    uint32 a = (uint8)(c.a * 255.f) << 0;
    return r + g + b + a;
}

internal bool32
color_is_dark(Color c)
{
    Vec4 v = color_v4(c);
    return v.x + v.y + v.z <= 0.6;
}