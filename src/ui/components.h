#pragma once

#include "base.h"
#include <core/defines.h>
#include <engine/draw.h>

typedef struct
{
    Rect   header;
    bool32 is_expanded;
} UIWindow;

internal Rect     ui_container(UIContext* ctx, Rect container, StyleContainer style);
internal UIWindow ui_window(UIContext* ctx, Rect* container, UIID id, String name, bool32* is_expanded, StyleWindow style);
internal void     ui_label(UIContext* ctx, Rect container, String str, StyleLabel style);
internal bool32   ui_slider(UIContext* ctx, Rect rect, UIID id, Range range, float32* value, StyleSlider style);
internal bool32   ui_button(UIContext* ctx, Rect rect, UIID id, String label, StyleButton style);
internal bool32   ui_toggle(UIContext* ctx, Rect rect, UIID id, bool32* is_toggled, StyleToggleButton style);