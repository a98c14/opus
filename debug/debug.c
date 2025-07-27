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

internal void
o_debug_rect_packer(RectPacker* rect_packer)
{
    ArenaTemp   temp = scratch_begin(0, 0);
    RectPacker* rp   = rect_packer;
    d_rect(rp->rect, 1, ColorRed400);
    for (int32 i = 0; i < rp->rect_count; i++)
    {
        d_rect(rp->rects[i], 1, ColorOrange200);
        d_string(rp->rects[i], string_pushf(temp.arena, "%d", i), 12, ColorBlack, ANCHOR_C_C);
    }

    // draw skyline
    for (int32 i = 0; i < rp->point_count - 1; i++)
    {
        Vec2 cur     = rp->points[i];
        Vec2 next    = rp->points[i + 1];
        Vec2 cur_end = add_vec2(cur, vec2(next.x - cur.x, 0));
        d_line(cur, cur_end, 2, ColorBlue400);
        if (next.y != cur.y)
        {
            d_line(cur_end, next, 2, ColorBlue400);
        }
    }

    scratch_end(temp);
}