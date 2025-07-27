#include "perf_timer.h"

internal PerfTimer*
perf_timer_new(Arena* arena, String name)
{
    PerfTimer* result = arena_push_struct_zero(arena, PerfTimer);
    result->name      = name;
    return result;
}

internal void
perf_timer_begin(PerfTimer* profiler)
{
    profiler->start[profiler->buffer_index % PROFILER_BUFFER_CAPACITY] = glfwGetTime() * 1000;
}

internal void
perf_timer_end(PerfTimer* profiler)
{
    uint64 index             = profiler->buffer_index % PROFILER_BUFFER_CAPACITY;
    profiler->elapsed[index] = glfwGetTime() * 1000 - profiler->start[index];
    profiler->buffer_index++;
}

internal float32
perf_timer_avg(PerfTimer* profiler)
{
    float32 sum = 0;
    for (int32 i = 0; i < PROFILER_BUFFER_CAPACITY; i++)
        sum += profiler->elapsed[i];
    return sum / PROFILER_BUFFER_CAPACITY;
}

internal float32
perf_timer_min(PerfTimer* profiler)
{
    float32 min_elapsed = MAX_FLOAT32;
    for (int32 i = 0; i < PROFILER_BUFFER_CAPACITY; i++)
        min_elapsed = min(profiler->elapsed[i], min_elapsed);
    return min_elapsed;
}

internal float32
perf_timer_max(PerfTimer* profiler)
{
    float32 max_elapsed = MIN_FLOAT32;
    for (int32 i = 0; i < PROFILER_BUFFER_CAPACITY; i++)
        max_elapsed = max(profiler->elapsed[i], max_elapsed);
    return max_elapsed;
}

internal void
perf_timer_refresh_cache(PerfTimer* profiler)
{
    profiler->cached_min = perf_timer_min(profiler);
    profiler->cached_max = perf_timer_max(profiler);
    profiler->cached_avg = perf_timer_avg(profiler);
}