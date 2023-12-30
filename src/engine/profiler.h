#pragma once
#include <core/defines.h>
#include <core/math.h>
#include <core/strings.h>

#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>

#define PROFILER_BUFFER_CAPACITY 128

typedef struct
{
    String  name;
    uint64  buffer_index;
    float64 start[PROFILER_BUFFER_CAPACITY];
    float64 elapsed[PROFILER_BUFFER_CAPACITY];

    float32 cached_min;
    float32 cached_max;
    float32 cached_avg;
} Profiler;

internal Profiler* profiler_new(Arena* arena, String name);
internal void      profiler_begin(Profiler* profiler);
internal void      profiler_end(Profiler* profiler);
internal float32   profiler_avg(Profiler* profiler);
internal float32   profiler_min(Profiler* profiler);
internal float32   profiler_max(Profiler* profiler);
internal void      profiler_refresh_cache(Profiler* profiler);