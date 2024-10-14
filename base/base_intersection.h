#pragma once
#include "base_defines.h"
#include "base_math.h"
#include "base_memory.h"
#include "base_thread_context.h"

typedef struct
{
    Vec2* vertices;
    int32 vertex_count;
    Vec2* normals;
    int32 normal_count;
} BPolygon;

typedef struct
{
    Vec2 start;
    Vec2 end;
} LineSegment;

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
internal Intersection intersects_polygon(BPolygon a, BPolygon b);
internal Intersection intersects_rect(Rect a, Rect b);
internal Intersection intersects_quad(Quad a, Quad b);

internal Projection project_circle(Circle c, Vec2 line);
internal Projection project_bounds(Bounds b, Vec2 line);
internal Projection project_rect(Rect r, Vec2 line);
internal Projection project_quad(Quad q, Vec2 line);
internal Projection project_polygon(BPolygon a, Vec2 line);
internal Projection project_vertices(Vec2* vertices, uint32 vertex_count, Vec2 line);
internal bool32     projection_overlaps(Projection p0, Projection p1);
internal bool32     projection_contains(Projection a, Projection b);
internal bool32     projection_overlaps_point(Projection p, float32 v);
internal float32    projection_overlap_amount(Projection p0, Projection p1);

internal Vec2 closest_point_rect(Rect r, Vec2 v);
internal Vec2 minimum_translation_vector(Vec2 normal, Projection p0, Projection p1, Vec2 center_0, Vec2 center_1);

/** ray */
internal Ray2 ray(Vec2 start, Vec2 direction);

internal Vec2 polygon_tr(BPolygon p);