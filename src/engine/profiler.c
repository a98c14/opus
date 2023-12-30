#include "profiler.h"

internal Profiler*
profiler_new(Arena* arena, String name)
{
    Profiler* result = arena_push_struct_zero(arena, Profiler);
    result->name     = name;
    return result;
}

internal void
profiler_begin(Profiler* profiler)
{
    profiler->start[profiler->buffer_index % PROFILER_BUFFER_CAPACITY] = glfwGetTime() * 1000;
}

internal void
profiler_end(Profiler* profiler)
{
    uint64 index             = profiler->buffer_index % PROFILER_BUFFER_CAPACITY;
    profiler->elapsed[index] = glfwGetTime() * 1000 - profiler->start[index];
    profiler->buffer_index++;
}

internal float32
profiler_avg(Profiler* profiler)
{
    float32 sum = 0;
    for (int32 i = 0; i < PROFILER_BUFFER_CAPACITY; i++)
        sum += profiler->elapsed[i];
    return sum / PROFILER_BUFFER_CAPACITY;
}

internal float32
profiler_min(Profiler* profiler)
{
    float32 min_elapsed = FLOAT32_MAX;
    for (int32 i = 0; i < PROFILER_BUFFER_CAPACITY; i++)
        min_elapsed = min(profiler->elapsed[i], min_elapsed);
    return min_elapsed;
}

internal float32
profiler_max(Profiler* profiler)
{
    float32 max_elapsed = FLOAT32_MIN;
    for (int32 i = 0; i < PROFILER_BUFFER_CAPACITY; i++)
        max_elapsed = max(profiler->elapsed[i], max_elapsed);
    return max_elapsed;
}

internal void
profiler_refresh_cache(Profiler* profiler)
{
    profiler->cached_min = profiler_min(profiler);
    profiler->cached_max = profiler_max(profiler);
    profiler->cached_avg = profiler_avg(profiler);
}