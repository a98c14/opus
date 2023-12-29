#include "intersection.h"

internal bool32
intersects_rect_point(Rect a, Vec2 b)
{
    Vec2 bl = rect_bl(a);
    Vec2 tr = rect_tr(a);
    
    return !(b.x < bl.x || b.y < bl.y || b.x > tr.x || b.y > tr.y);
}

internal bool32
intersects_circle_point(Circle a, Vec2 b)
{
    return lensqr_vec2(sub_vec2(b, a.center)) < (a.radius/2 * a.radius/2);
}