#include "base.h"

internal Theme*
theme_init_default(Arena* arena, Renderer* renderer)
{
    Theme* t = arena_push_struct_zero_aligned(arena, Theme, 16);
    t->p0 = vec2(0, 0);
    t->p2 = vec2(4, 4);

    t->m0 = vec2(0, 0);
    t->m2 = vec2(4, 4);

    t->alignment = AlignmentTopLeft;
    t->padding = vec2(4, 4);
    t->line_height = 8;
    t->spacing = 2;

    t->rounded_none = (BorderRadius){ .bl =  0, .br = 0, .tr = 0, .tl = 0 };
    t->rounded_sm   = (BorderRadius){ .bl =  2, .br = 2, .tr = 2, .tl = 2 };
    t->rounded_md   = (BorderRadius){ .bl = 10, .br = 10, .tr = 10, .tl = 10 };
    t->rounded_lg   = (BorderRadius){ .bl = 20, .br = 20, .tr = 20, .tl = 20 };
    t->rounded_xl   = (BorderRadius){ .bl = 30, .br = 30, .tr = 30, .tl = 30 };

    t->rect_default = (StyleRect){ .softness = 2, .color = color_to_vec4(ColorSlate800_Transparent), .border_radius = t->rounded_sm };
    t->rect_debug = (StyleRect){ .border_thickness = 3, .softness = 0, .color = color_to_vec4(ColorInvisible), .border_color = color_to_vec4(ColorRed600), .border_radius = t->rounded_none };
    t->rect_debug2 = (StyleRect){ .border_thickness = 3, .softness = 0, .color = color_to_vec4(ColorInvisible), .border_color = color_to_vec4(ColorGreen600), .border_radius = t->rounded_none };
    t->rect_view = (StyleRect){ .softness = 2, .color = color_to_vec4(ColorWhite100), .border_radius = { .bl = 3, .br = 3, .tr = 0, .tl = 0 } };
    t->rect_header = (StyleRect){ .border_thickness = 0, .softness = 2, .color = color_to_vec4(ColorSlate700), .border_color = color_to_vec4(ColorInvisible), .border_radius = { .bl = 0, .br = 0, .tr = 3, .tl = 3 } };
    t->rect_slider_bar = (StyleRect){ .softness = 2, .color = color_to_vec4(ColorBlack), .border_radius = t->rounded_sm };
    t->rect_button = (StyleRect){ .softness = 2, .color = color_to_vec4(ColorAmber600), .border_radius = t->rounded_sm };
    t->rect_button_hover = (StyleRect){ .softness = 2, .color = color_to_vec4(ColorAmber400), .border_radius = t->rounded_sm };
    t->rect_toggle = (StyleRect){ .softness = 2, .color = color_to_vec4(ColorBlack), .border_radius = t->rounded_sm };
    t->rect_toggle_active = (StyleRect){ .softness = 2, .color = color_to_vec4(ColorWhite), .border_radius = t->rounded_sm };
    t->rect_toggle_hover = (StyleRect){ .softness = 2, .color = color_to_vec4(ColorBlack), .border_radius = t->rounded_sm };
    t->rect_container = (StyleRect){ .softness = 2, .color = color_to_vec4(ColorBlueShadow100), .border_radius = t->rounded_sm };
    
    t->rect_header_hover = t->rect_header;
    t->rect_header_hover.color = color_to_vec4(ColorSlate500);

    t->font_default = (StyleText){ .color = color_to_vec4(ColorBlack), .outline_color = color_to_vec4(ColorInvisible), .softness = 30, .thickness = 0.50, .outline_thickness = 0.20, .font_size = em(1), .base_line = px(3)};
    t->font_default_light = (StyleText){ .color = color_to_vec4(ColorWhite), .outline_color = color_to_vec4(ColorInvisible), .softness = 30, .thickness = 0.50, .outline_thickness = 0.20, .font_size = em(1), .base_line = px(3)};
    t->font_window_header = (StyleText){ .color = color_to_vec4(ColorSlate200), .outline_color = color_to_vec4(ColorInvisible), .softness = 30, .thickness = 0.50, .outline_thickness = 0.20, .font_size = px(16), .base_line = px(3)};    
    t->font_debug = (StyleText){ .color = color_to_vec4(ColorSlate100), .outline_color = color_to_vec4(ColorInvisible), .softness = 30, .thickness = 0.50, .outline_thickness = 0.20, .font_size = px(16), .base_line = px(3)};

    /* containers */
    t->container_default.background = t->rect_container;
    t->container_default.margin = t->m0;
    t->container_default.padding = t->p0;

    t->container_light = t->container_default;
    t->container_light.background = t->rect_default;

    /* windows */
    t->window_default.background = t->rect_default;
    t->window_default.header_background = t->rect_header;
    t->window_default.header_background_hover = t->rect_header_hover;
    t->window_default.header_font = t->font_window_header;
    t->window_default.padding = t->p2;

    t->slider_default.slider = t->rect_slider_bar;

    t->button_default.background = t->rect_button;
    t->button_default.background_hover = t->rect_button_hover;
    t->button_default.font = t->font_default_light;
    t->button_default.padding = t->p0;

    t->toggle_default.background = t->rect_button;
    t->toggle_default.background_hover = t->rect_button_hover;
    t->toggle_default.inner_background = t->rect_toggle;
    t->toggle_default.inner_background_hover = t->rect_toggle_hover;
    t->toggle_default.inner_background_active = t->rect_toggle_active;
    t->toggle_default.font = t->font_default_light;
    t->toggle_default.padding = t->p0;

    /* labels */
    t->label_default.font = t->font_default_light;
    // t->label_default.font.font_size = px(20);
    t->label_default.padding = vec2(2, 2);
    t->label_default.alignment = AlignmentLeft;

    t->label_bold = t->label_default;
    t->label_bold.font.thickness = 0.60;

    t->label_debug = t->label_default;
    t->label_debug.background = t->rect_debug;


    return t;
}