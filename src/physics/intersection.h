#pragma once

#include <base/defines.h>
#include <base/math.h>

internal bool32 intersects_rect_point(Rect a, Vec2 b);
internal bool32 intersects_circle_point(Circle a, Vec2 b);
internal bool32 intersects_bounds(Bounds a, Bounds b);
internal bool32 intersects_circle(Circle a, Circle b);