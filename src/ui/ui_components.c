#include "ui_components.h"

internal bool32
ui_button(UI_Key key, String label)
{
    Rect         button_container = ui_cut_top(32);
    Intersection i                = intersects_rect_point(button_container, ui_state->input_mouse.screen);
    if (i.intersects)
    {
        ui_state->hot = key;
    }

    d_rect(button_container, 0, i.intersects ? ColorSlate600 : ColorSlate800);
    d_string(button_container, label, 24, ColorWhite, ANCHOR_C_C);

    bool32 is_hot = ui_key_same(ui_state->hot, key);
    if (is_hot && input_mouse_pressed(ui_state->input_mouse, MouseButtonStateLeft))
    {
        log_info("clicked %s", label.value);
        return true;
    }

    return false;
}
