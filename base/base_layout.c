#pragma once

#include "base_layout.h"

internal Cut
cut(Rect* rect, CutSide side)
{
    Cut result;
    result.rect = rect;
    result.side = side;
    return result;
}

internal Rect
rect_cut(Rect* r, float32 size, CutSide side)
{
    switch (side)
    {
    case CutSideTop:
        return rect_cut_top(r, size);
    case CutSideRight:
        return rect_cut_right(r, size);
    case CutSideLeft:
        return rect_cut_left(r, size);
    case CutSideBottom:
        return rect_cut_bottom(r, size);
    default:
        not_implemented();
    }
    return (Rect){0};
}

internal Rect
rect_cut_r(Rect* r, Rect size, CutSide side)
{
    switch (side)
    {
    case CutSideTop:
        return rect_cut_top(r, size.h);
    case CutSideRight:
        return rect_cut_right(r, size.w);
    case CutSideLeft:
        return rect_cut_left(r, size.w);
    case CutSideBottom:
        return rect_cut_bottom(r, size.h);
    default:
        not_implemented();
    }
    return (Rect){0};
}

internal Rect
rect_cut_left(Rect* r, float32 size)
{
    size        = min(r->w, size);
    Rect result = rect_from_xy_wh(rect_left(*r) + size / 2.0f, r->y, size, r->h);
    r->w -= size;
    r->x += size / 2.0f;
    return result;
}

internal Rect
rect_cut_right(Rect* r, float32 size)
{
    size        = min(r->w, size);
    Rect result = rect_from_xy_wh(rect_right(*r) - size / 2.0f, r->y, size, r->h);
    r->w -= size;
    r->x -= size / 2.0f;
    return result;
}

internal Rect
rect_cut_top(Rect* r, float32 size)
{
    size        = min(r->h, size);
    Rect result = rect_from_xy_wh(r->x, rect_top(*r) - size / 2.0f, r->w, size);
    r->h -= size;
    r->y -= size / 2.0f;
    return result;
}

internal Rect
rect_cut_bottom(Rect* r, float32 size)
{
    size        = min(r->h, size);
    Rect result = rect_from_xy_wh(r->x, rect_bottom(*r) + size / 2.0f, r->w, size);
    r->h -= size;
    r->y += size / 2.0f;
    return result;
}

internal Rect
rect_at(Vec2 position, Vec2 size, Alignment alignment)
{
    Rect result;
    result.center = position;
    result.size   = size;
    result        = rect_align(result, alignment);
    return result;
}

internal Rect
rect_aligned(float32 x, float32 y, float32 w, float32 h, Alignment alignment)
{
    Rect result;
    result.x = x;
    result.y = y;
    result.w = w;
    result.h = h;
    result.x += result.w * AlignmentMultiplierX[alignment];
    result.y += result.h * AlignmentMultiplierY[alignment];
    return result;
}

internal Vec2
rect_position_from_alignment(Rect rect, Alignment alignment)
{
    Vec2 result;
    result.x = rect.x - rect.w * AlignmentMultiplierX[alignment];
    result.y = rect.y - rect.h * AlignmentMultiplierY[alignment];
    return result;
}

internal Rect
rect_align(Rect rect, Alignment alignment)
{
    rect.x += rect.w * AlignmentMultiplierX[alignment];
    rect.y += rect.h * AlignmentMultiplierY[alignment];
    return rect;
}

internal Rect
rect_anchor(Rect child, Rect parent, Anchor anchor)
{
    Rect result = rect_align(child, anchor.child);
    result.x    = result.x + parent.x + AnchorMultiplierX[anchor.parent] * parent.w;
    result.y    = result.y + parent.y + AnchorMultiplierY[anchor.parent] * parent.h;
    return result;
}

internal Rect
rect_at_o(Vec2 position, Vec2 size, Vec2 offset, Alignment alignment)
{
    Rect result = rect_at(position, size, alignment);
    result      = rect_move(result, offset);
    return result;
}

internal Rect
rect_place(Rect child, Rect parent, Anchor anchor)
{
    Rect result = rect_from_wh(child.w, child.h);
    result      = rect_align(result, anchor.child);
    result.x    = result.x + parent.x + AnchorMultiplierX[anchor.parent] * parent.w;
    result.y    = result.y + parent.y + AnchorMultiplierY[anchor.parent] * parent.h;
    return result;
}

internal Rect
rect_place_under(Rect child, Rect parent)
{
    return rect_place(child, parent, ANCHOR_BL_TL);
}

internal Vec2
rect_relative(Rect rect, Alignment alignment)
{
    Vec2 result = rect.center;
    result.x    = rect.x - AlignmentMultiplierX[alignment] * rect.w;
    result.y    = rect.y - AlignmentMultiplierY[alignment] * rect.h;
    return result;
}

internal Rect
rect_expand_f32(Rect rect, float32 v)
{
    rect.w = max(0, rect.w + v);
    rect.h = max(0, rect.h + v);
    return rect;
}

internal Rect
rect_expand(Rect rect, Vec2 v)
{
    rect.w = max(0, rect.w + v.x);
    rect.h = max(0, rect.h + v.y);
    return rect;
}

internal Rect
rect_shrink_f32(Rect rect, float32 v)
{
    return rect_expand_f32(rect, -v);
}

internal Rect
rect_shrink(Rect rect, Vec2 v)
{
    return rect_expand(rect, mul_vec2_f32(v, -1));
}

internal Rect
rect_move(Rect rect, Vec2 v)
{
    Rect result = rect;
    result.x += v.x;
    result.y += v.y;
    return result;
}

internal Rect
rect_resize(Rect r, float32 w, float32 h, Alignment alignment)
{
    Anchor anchor;
    anchor.child  = alignment;
    anchor.parent = alignment;

    Rect result = rect_from_xy_wh(0, 0, w, h);
    result      = rect_anchor(result, r, anchor);
    return result;
}

internal Rect
rect_resize_height(Rect r, float32 h, Alignment alignment)
{
    return rect_resize(r, r.w, h, alignment);
}

internal Rect
rect_resize_width(Rect r, float32 w, Alignment alignment)
{
    return rect_resize(r, w, r.h, alignment);
}

/** Axis Functions */
internal float32
rect_axis_min(Rect rect, Axis axis)
{
    return axis == AxisVertical ? rect_bottom(rect) : rect_left(rect);
}

internal float32
rect_axis_max(Rect rect, Axis axis)
{
    return axis == AxisVertical ? rect_top(rect) : rect_right(rect);
}

internal Rect
rect_axis_set(Rect rect, Axis axis, float32 min, float32 max)
{
    Rect result = {0};

    result.size.w   = rect.size.w * (1 - AxisMultiplierX[axis]) + (max - min) * AxisMultiplierX[axis]; // TODO(selim): use lerp here
    result.size.h   = rect.size.h * (1 - AxisMultiplierY[axis]) + (max - min) * AxisMultiplierY[axis];
    result.center.x = rect.center.x - (rect.size.w - result.size.w) / 2.0f;
    result.center.y = rect.center.y + (rect.size.h - result.size.h) / 2.0f;
    return result;
}

internal Vec2
vec2_axis_add(Vec2 a, Vec2 b, Axis axis)
{
    Vec2 result;
    result.x = lerp_f32(a.x, a.x + b.x, AxisMultiplierX[axis]);
    result.y = lerp_f32(a.y, a.y + b.y, AxisMultiplierY[axis]);
    return result;
}
