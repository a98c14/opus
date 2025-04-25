#include "debug.h"

internal void
o_debug()
{
#if _DEBUG_MOUSE == 1
    o_debug_mouse();
#endif
}

internal void
o_debug_mouse()
{
    Input_MouseInfo mouse_info = input_mouse_info();

    d_line(vec2_zero(), mouse_info.world, 1, ColorWhite);
}