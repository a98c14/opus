#include "profiler.h"

#if PROFILE_SUPERLUMINAL

internal void
profiler_init(Arena* arena)
{
    g_performance_api = arena_push_struct(arena, PerformanceAPI_Functions);
    PerformanceAPI_LoadFrom(L".\\PerformanceAPI.dll", g_performance_api);
}
#endif