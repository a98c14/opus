#pragma once

#include <base.h>
#include <draw/draw_inc.h>

typedef struct
{
    Vec2* vertices;
    int32 vertex_count;
    Vec2* normals;
    int32 normal_count;
} P_Polygon;

typedef struct
{
    Vec2 start;
    Vec2 end;
} P_Edge;

typedef union
{
    struct
    {
        float32 min;
        float32 max;
    };

    Vec2 v;
} Projection;

typedef struct
{
    Vec2   position;
    Vec2   mtv;
    bool32 intersects;
} Intersection;

typedef struct
{
    Vec2 start;
    Vec2 direction;
} Ray2;

internal Intersection intersects_rect_point(Rect a, Vec2 b);
internal Intersection intersects_circle_point(Circle a, Vec2 b);
internal bool32       intersects_bounds_fast(Bounds a, Bounds b);
internal bool32       intersects_rect_fast(Rect a, Rect b);
internal Intersection intersects_ray(Ray2 a, Ray2 b);
internal Intersection intersects_circle(Circle a, Circle b);
internal Intersection intersects_circle_rect(Circle a, Rect b);
internal Intersection intersects_polygon(P_Polygon a, P_Polygon b);
internal Intersection intersects_rect(Rect a, Rect b);
internal Intersection intersects_quad(Quad a, Quad b);

internal Projection project_circle(Circle c, Vec2 line);
internal Projection project_bounds(Bounds b, Vec2 line);
internal Projection project_rect(Rect r, Vec2 line);
internal Projection project_quad(Quad q, Vec2 line);
internal Projection project_polygon(P_Polygon a, Vec2 line);
internal Projection project_vertices(Vec2* vertices, uint32 vertex_count, Vec2 line);
internal bool32     projection_overlaps(Projection p0, Projection p1);
internal bool32     projection_contains(Projection a, Projection b);
internal bool32     projection_overlaps_point(Projection p, float32 v);
internal float32    projection_overlap_amount(Projection p0, Projection p1);

internal Vec2 closest_point_rect(Rect r, Vec2 v);
internal Vec2 minimum_translation_vector(Vec2 normal, Projection p0, Projection p1, Vec2 center_0, Vec2 center_1);

// TODO(selim): move these to another file
internal Ray2   ray(Vec2 start, Vec2 direction);
internal Bounds p_quad_get_bounds(Quad a);
internal Vec2   p_polygon_tr(P_Polygon p);