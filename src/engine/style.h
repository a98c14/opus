// TODO: Move styles to `/ui` folder
#pragma once

#include <core/defines.h>
#include <core/math.h>
#include "layout.h"

typedef union 
{
    struct { float32 tr; float32 br; float32 tl; float32 bl; };
    Vec4 v;
} BorderRadius;

typedef struct
{
    Vec4 color;
    Vec4 border_color;
    BorderRadius border_radius;
    float32 softness;
    float32 border_thickness;
} StyleRect;

typedef struct
{
    Vec4 color;
    Vec4 outline_color;
    /** Softness of the glyph edges. Recommended value: 30 */
    float32 softness;
    /** Thickness of the glyph. Recommended value: 0.5, Values outside of the range 0.4 and 0.9 are not really usable */
    float32 thickness;
    /** Outline thickness. Should be between 0 and 0.5 */
    float32 outline_thickness;
    float32 font_size;
    /** Letter y coordinate is offseted by this amount */
    float32 base_line;
} StyleText;

typedef struct
{
    Vec2 margin;
    Vec2 padding;
    StyleRect background;
} StyleContainer;

typedef struct
{
    Vec2 padding;
    StyleRect background;
    StyleRect header_background;
    StyleRect header_background_hover;
    StyleText header_font;
} StyleWindow;

typedef struct
{
    Vec2 padding;
    Alignment alignment;
    StyleRect background;
    StyleText font;
} StyleLabel;


typedef struct
{
    Vec2 padding;
    StyleText font;
    StyleRect background;
    StyleRect background_hover;
} StyleButton;

typedef struct
{
    Vec2 padding;
    StyleText font;
    StyleRect background;
    StyleRect background_hover;
    StyleRect inner_background;
    StyleRect inner_background_hover;
    StyleRect inner_background_active;
} StyleToggleButton;

typedef struct
{
    StyleRect slider;
} StyleSlider;

internal StyleText
font_style_override_size(StyleText style, float32 new_size);
