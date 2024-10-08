#pragma once
#include "./gfx_core.h"

/* converts the unit value to actual screen pixel*/
internal float32 px(float32 u);
internal float32 px_inverse(float32 u);
internal float32 em(float32 v);

/** helpers */
internal float32 screen_top();
internal float32 screen_left();
internal float32 screen_right();
internal float32 screen_bottom();
internal float32 screen_height();
internal float32 screen_width();
internal Rect    screen_rect();