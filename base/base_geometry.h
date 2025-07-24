#pragma once

#include "base_math.h"
#include "base_memory.h"
#include "base_thread_context.h"

typedef struct
{
    Vec2*  points;
    uint64 count;
} PointArray;

internal PointArray geo_generate_circle_points(Arena* arena, float32 r);