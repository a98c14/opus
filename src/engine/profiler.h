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
} PerfTimer;

internal PerfTimer* perf_timer_new(Arena* arena, String name);
internal void       perf_timer_begin(PerfTimer* profiler);
internal void       perf_timer_end(PerfTimer* profiler);
internal float32    perf_timer_avg(PerfTimer* profiler);
internal float32    perf_timer_min(PerfTimer* profiler);
internal float32    perf_timer_max(PerfTimer* profiler);
internal void       perf_timer_refresh_cache(PerfTimer* profiler);