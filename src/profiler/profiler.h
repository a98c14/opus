// clang-format off
#pragma once


#define PROFILER_COLOR_DEFAULT 0xFFFFFFFF
#define PROFILER_COLOR_RED     0xFF0000FF
#define PROFILER_COLOR_UPDATE  0x44BBFFFF

#if !defined(PROFILE_SUPERLUMINAL)
    #define PROFILE_SUPERLUMINAL 0
#endif


#if PROFILE_SUPERLUMINAL
#include <base/defines.h>
#include <base/memory.h>

#include <windows.h>
#include <superluminal/PerformanceAPI_capi.h>
#include <superluminal/PerformanceAPI_loader.h>

global PerformanceAPI_Functions* g_performance_api;
internal void profiler_init(Arena* arena);

#define profiler_thread_name(name) (g_performance_api->SetCurrentThreadName(name))
#define profiler_begin_data_color(name, data, color) (g_performance_api->BeginEvent(name, data, color))
#define profiler_begin_color(name, color) (profiler_begin_data_color(name, 0, color))
#define profiler_begin(name) profiler_begin_color(name, PROFILER_COLOR_DEFAULT)
#define profiler_end() g_performance_api->EndEvent()
#else
#define profiler_thread_name(name) (0)
#define profiler_begin_data_color(name, data, color) (0)
#define profiler_begin_color(name, color) (0)
#define profiler_begin(name) (0)
#define profiler_end() (0)
#define profiler_init(arena) (0)
#endif

#define profiler_scope(name, color) defer_loop(profiler_begin_color(name, color), profiler_end())
#define profiler_scope_with_ctx(name, ctx, color) defer_loop(profiler_begin_data_color(name, ctx, color), profiler_end())

