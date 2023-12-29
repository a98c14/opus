#pragma once
#include <core/defines.h>
#include <core/memory.h>
#include <engine/draw.h>

typedef struct 
{
    /* padding */
    Vec2 p0;
    Vec2 p2;

    /* margin */
    Vec2 m0;
    Vec2 m2;

    /* border radius */
    BorderRadius rounded_none;
    BorderRadius rounded_sm;
    BorderRadius rounded_md;
    BorderRadius rounded_lg;
    BorderRadius rounded_xl;

    StyleRect rect_default;
    StyleRect rect_debug;
    StyleRect rect_debug2;
    StyleRect rect_view;
    StyleRect rect_header;
    StyleRect rect_slider_bar;
    StyleRect rect_header_hover;
    StyleRect rect_button;
    StyleRect rect_button_hover;
    StyleRect rect_container;
    StyleRect rect_toggle;
    StyleRect rect_toggle_active;
    StyleRect rect_toggle_hover;

    StyleText font_default;
    StyleText font_default_light;
    StyleText font_debug;
    StyleText font_window_header;

    /* containers */
    StyleContainer container_default;
    StyleContainer container_light;

    /* windows */
    StyleWindow window_default;

    /* labels */
    StyleLabel label_default;
    StyleLabel label_bold;
    StyleLabel label_debug;

    StyleSlider slider_default;

    StyleButton button_default;
    StyleToggleButton toggle_default;

    /* Global Styles */
    Vec2 padding;
    float32 line_height;
    float32 spacing;
    Alignment alignment;
} Theme;

internal Theme*
theme_init_default(Arena* arena, Renderer* renderer);