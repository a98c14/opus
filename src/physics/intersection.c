#include "intersection.h"
#include <base/math.h>
#include <physics/intersection.h>

internal Intersection
intersects_rect_point(Rect a, Vec2 b)
{
    Intersection result;
    Vec2         bl = rect_bl(a);
    Vec2         tr = rect_tr(a);

    result.position   = b;
    result.intersects = !(b.x < bl.x || b.y < bl.y || b.x > tr.x || b.y > tr.y);
    return result;
}

internal Intersection
intersects_circle_point(Circle a, Vec2 b)
{
    Intersection result;
    result.position   = b;
    result.intersects = lensqr_vec2(sub_vec2(b, a.center)) < (a.radius / 2 * a.radius / 2);
    return result;
}

internal bool32
intersects_bounds_fast(Bounds a, Bounds b)
{
    return !(a.bl.x > b.tr.x || a.tr.x < b.bl.x || a.bl.y > b.tr.y || a.tr.y < b.bl.y);
}

internal bool32
intersects_rect_fast(Rect a, Rect b)
{
    return !(rect_left(a) > rect_right(b) || rect_right(a) < rect_left(b) || rect_bottom(a) > rect_top(b) || rect_top(a) < rect_bottom(b));
}

internal Intersection
intersects_circle(Circle a, Circle b)
{
    Vec2       normal = heading_to_vec2(a.center, b.center);
    Projection p_a    = project_circle(a, normal);
    Projection p_b    = project_circle(b, normal);

    Intersection result = {0};
    result.intersects   = projection_overlaps(p_a, p_b);
    if (result.intersects)
    {
        result.position = add_vec2(a.center, mul_vec2_f32(normal, projection_overlap_amount(p_a, p_b)));
        result.mtv      = minimum_translation_vector(normal, p_a, p_b, a.center, b.center);
    }
    return result;
}

internal Intersection
intersects_rect(Rect a, Rect b)
{
    Intersection result   = {0};
    Vec2         normal_x = vec2(1, 0);
    Vec2         normal_y = vec2(0, 1);

    Projection projection_ax = project_rect(a, normal_x);
    Projection projection_bx = project_rect(b, normal_x);

    result.intersects = projection_overlaps(projection_ax, projection_bx);
    if (!result.intersects)
        return result;

    Projection projection_ay = project_rect(a, normal_y);
    Projection projection_by = project_rect(b, normal_y);
    result.intersects        = projection_overlaps(projection_ay, projection_by);
    if (!result.intersects)
        return result;

    bool32 x_smaller_than_y = projection_overlap_amount(projection_ax, projection_bx) < projection_overlap_amount(projection_ay, projection_by);
    result.mtv              = x_smaller_than_y
                                  ? minimum_translation_vector(normal_x, projection_ax, projection_bx, a.center, b.center)
                                  : minimum_translation_vector(normal_y, projection_ay, projection_by, a.center, b.center);

    result.position = lerp_vec2(a.center, b.center, 0.5);
    return result;
}

internal Intersection
intersects_circle_rect(Circle a, Rect b)
{
    Intersection result;

    // temporary variables to set edges for testing
    Vec2 test;

    // which edge is closest?
    if (a.center.x < b.center.x)
        test.x = b.center.x - b.size.x;
    else if (a.center.x > b.center.x)
        test.x = b.center.x + b.size.x;
    if (a.center.y < b.center.y)
        test.y = b.center.y - b.size.y;
    else if (a.center.y > b.center.y)
        test.y = b.center.y + b.size.y;

    float32 distance  = dist_vec2(a.center, test);
    result.intersects = distance < a.radius;
    result.position   = move_towards_vec2(a.center, test, distance);
    return result;
}

internal Projection
project_circle(Circle c, Vec2 line)
{
    Projection result;
    float32    dot = dot_vec2(c.center, line);
    result.min     = dot - c.radius / 2.0f;
    result.max     = dot + c.radius / 2.0f;
    return result;
}

internal Projection
project_bounds(Bounds b, Vec2 line)
{
    Projection result;
    float32    d0 = dot_vec2(b.bl, line);
    float32    d1 = dot_vec2(vec2(b.left, b.top), line);
    float32    d2 = dot_vec2(b.tr, line);
    float32    d3 = dot_vec2(vec2(b.right, b.bottom), line);

    result.min = min(min(d0, d1), min(d2, d3));
    result.max = max(max(d0, d1), max(d2, d3));
    return result;
}

internal Projection
project_rect(Rect r, Vec2 line)
{
    Projection result;
    float32    d0 = dot_vec2(rect_bl(r), line);
    float32    d1 = dot_vec2(rect_tl(r), line);
    float32    d2 = dot_vec2(rect_tr(r), line);
    float32    d3 = dot_vec2(rect_br(r), line);

    result.min = min(min(d0, d1), min(d2, d3));
    result.max = max(max(d0, d1), max(d2, d3));
    return result;
}

internal Projection
project_vertices(Vec2* vertices, uint32 vertex_count, Vec2 line)
{
    Projection result;
    float32    min = FLOAT32_MAX;
    float32    max = FLOAT32_MIN;
    for (uint32 i = 0; i < vertex_count; i++)
    {
        Vec2    vertex = vertices[i];
        float32 d      = dot_vec2(vertex, line);
        min            = min(d, min);
        max            = max(d, max);
    }

    result.min = min;
    result.max = max;
    return result;
}

internal bool32
projection_overlaps(Projection p0, Projection p1)
{
    return !(p0.min > p1.max || p1.min > p0.max);
}

internal bool32
projection_overlaps_point(Projection p, float32 v)
{
    return v > p.min && v < p.max;
}

internal float32
projection_overlap_amount(Projection p0, Projection p1)
{
    return min(p0.max - p1.min, p1.max - p0.min);
}

internal Vec2
minimum_translation_vector(Vec2 normal, Projection p0, Projection p1, Vec2 center_0, Vec2 center_1)
{
    float32 overlap_0 = p0.max - p1.min;
    float32 overlap_1 = p0.min - p1.max;
    float32 m         = min(fabs(overlap_0), fabs(overlap_1));
    Vec2    v         = sub_vec2(center_1, center_0);
    float32 d         = dot_vec2(v, normal);
    float32 sign      = d > 0 ? 1 : -1;
    return mul_vec2_f32(mul_vec2_f32(normal, sign), m);
}