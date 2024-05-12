#pragma once
#include <base/defines.h>
#include <base/math.h>

#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>

typedef struct
{
    float32 last_frame_time; // TODO(selim): Do we need this?
    float32 current_frame_time;
    float32 dt;
} EngineTime;

internal EngineTime engine_time_new();
internal EngineTime engine_get_time(EngineTime prev);