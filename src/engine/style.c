#include "style.h"

internal StyleText
font_style_override_size(StyleText style, float32 new_size)
{
    StyleText result = style;
    result.font_size = new_size;
    return result;
}