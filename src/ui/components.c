#include "components.h"

internal UIWindow
ui_window(UIContext* ctx, Rect* container, UIID id, String name, bool32* is_expanded, StyleWindow style)
{
    Rect       base_container = *container;
    const Vec2 header_padding = vec2(4, 4);
    Rect       header         = rect_from_wh(base_container.w, em(1) + header_padding.y);
    header                    = rect_anchor(header, base_container, ANCHOR_TL_TL);
    Rect header_inner         = rect_shrink(header, header_padding);

    bool32    hover        = intersects_rect_point(header, ctx->mouse.world);
    StyleRect header_style = hover ? style.header_background_hover : style.header_background;
    draw_rect_internal(header, SORT_LAYER_INDEX_UI, ViewTypeScreen, header_style);
    draw_text_at(rect_cl(header_inner), name, AlignmentLeft, style.header_font, ViewTypeScreen, SORT_LAYER_INDEX_UI);
    if (!ui_is_active_any(ctx) && hover && input_mouse_pressed(ctx->mouse, MouseButtonStateLeft))
    {
        ui_activate(ctx, id);
        ctx->drag_offset = sub_vec2(container->center, ctx->mouse.world);
    }
    else if (ui_is_active(ctx, id) && input_mouse_pressed(ctx->mouse, MouseButtonStateLeft))
    {
        container->x = ctx->mouse.world.x + ctx->drag_offset.x;
        container->y = ctx->mouse.world.y + ctx->drag_offset.y;
    }
    else if (ui_is_active(ctx, id) && input_mouse_released(ctx->mouse, MouseButtonStateLeft))
    {
        if (ui_active_time(ctx) < 200)
            b32_flip(is_expanded);

        ui_active_clear(ctx);
    }

    UIWindow result;
    result.header      = rect_place_under(rect_from_wh(container->w, 30), header);
    result.is_expanded = *is_expanded;
    return result;
}

internal Rect
ui_container(UIContext* ctx, Rect container, StyleContainer style)
{
    Rect outer = rect_shrink(container, style.margin);
    Rect inner = rect_shrink(container, style.padding);
    if (style.background.color.a > 0)
        draw_rect_internal(outer, SORT_LAYER_INDEX_UI, ViewTypeScreen, style.background);
    return inner;
}

internal void
ui_label(UIContext* ctx, Rect container, String str, StyleLabel style)
{
    Rect inner = rect_shrink(container, style.padding);
    if (style.background.color.a > 0 || style.background.border_color.a > 0)
        draw_rect_internal(container, SORT_LAYER_INDEX_UI, ViewTypeScreen, style.background);
    draw_text_at(rect_relative(inner, style.alignment), str, style.alignment, style.font, ViewTypeScreen, SORT_LAYER_INDEX_UI);
}

internal bool32
ui_slider(UIContext* ctx, Rect rect, UIID id, Range range, float32* value, StyleSlider style)
{
    const float32 handle_radius = px(20);
    *value                      = clamp(range.min, *value, range.max);

    // draw bar
    Rect bar = rect;
    bar.h    = em(0.5);
    draw_rect_internal(bar, SORT_LAYER_INDEX_UI + 1, ViewTypeScreen, style.slider);

    // since handle is centered at the origin, this reduces the overflow
    // that happens when the handle is at max/min
    rect.w -= handle_radius / 2;
    float32 normalized = (*value - range.min) / (range.max - range.min);
    float32 left       = rect_left(rect);
    float32 right      = rect_right(rect);
    float32 x          = left + rect.w * normalized;

    Circle handle = circle(vec2(x, rect.y), handle_radius);
    bool32 hover  = intersects_circle_point(handle, ctx->mouse.world);
    if (hover && ui_is_free(ctx) && input_mouse_pressed(ctx->mouse, MouseButtonStateLeft))
    {
        ui_activate(ctx, id);
        ctx->drag_offset = sub_vec2(ctx->mouse.world, handle.center);
    }
    else if (ui_is_active(ctx, id) && input_mouse_pressed(ctx->mouse, MouseButtonStateLeft))
    {
        float32 new_x = sub_vec2(ctx->mouse.world, ctx->drag_offset).x;
        new_x         = clamp(left, new_x, right);
        *value        = range.min + ((new_x - left) / rect.w) * (range.max - range.min);
    }
    else if (ui_is_active(ctx, id) && input_mouse_released(ctx->mouse, MouseButtonStateLeft))
    {
        ui_active_clear(ctx);
    }

    Color c = hover ? ColorRed400 : ColorRed600;
    draw_circle_filled(handle, c, SORT_LAYER_INDEX_UI);
    return ui_is_active(ctx, id);
}

internal bool32
ui_toggle(UIContext* ctx, Rect rect, UIID id, bool32* is_toggled, StyleToggleButton style)
{
    bool32 hover   = intersects_rect_point(rect, ctx->mouse.world);
    bool32 clicked = ui_is_active(ctx, id) && input_mouse_released(ctx->mouse, MouseButtonStateLeft);
    if (hover && ui_is_free(ctx) && input_mouse_pressed(ctx->mouse, MouseButtonStateLeft))
    {
        ui_activate(ctx, id);
    }
    else if (clicked)
    {
        *is_toggled = !(*is_toggled);
        ui_active_clear(ctx);
    }

    StyleRect c                 = hover ? style.background_hover : style.background;
    Rect      inner_row         = rect_shrink(rect, style.padding);
    Rect      toggle_box        = rect_anchor(rect_from_wh(em(1), em(1)), inner_row, ANCHOR_L_L);
    Rect      toggle_inside_box = rect_shrink(toggle_box, vec2(2, 2));
    draw_rect_internal(toggle_box, SORT_LAYER_INDEX_UI, ViewTypeScreen, c);
    draw_rect_internal(toggle_inside_box, SORT_LAYER_INDEX_UI, ViewTypeScreen, *is_toggled ? style.inner_background_active : style.inner_background);
    return *is_toggled;
}

internal bool32
ui_button(UIContext* ctx, Rect rect, UIID id, String label, StyleButton style)
{
    Alignment alignment = AlignmentCenter;
    Rect      inner_row = rect_shrink(rect, style.padding);
    draw_text_at(rect_relative(inner_row, alignment), label, alignment, style.font, ViewTypeScreen, ctx->sort_layer_index);
    bool32 hover   = intersects_rect_point(rect, ctx->mouse.screen);
    bool32 clicked = ui_is_active(ctx, id) && input_mouse_released(ctx->mouse, MouseButtonStateLeft);
    if (hover && ui_is_free(ctx) && input_mouse_pressed(ctx->mouse, MouseButtonStateLeft))
    {
        ui_activate(ctx, id);
    }
    else if (clicked)
    {
        ui_active_clear(ctx);
    }

    StyleRect c = hover ? style.background_hover : style.background;
    draw_rect_internal(inner_row, ctx->sort_layer_index, ViewTypeScreen, c);
    return clicked;
}