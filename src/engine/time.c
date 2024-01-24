#include "time.h"

internal EngineTime
engine_time_new()
{
    EngineTime result    = {0};
    result.current_frame = (float32)glfwGetTime();
    return result;
}

internal EngineTime
engine_get_time(EngineTime prev)
{
    EngineTime result    = {0};
    result.last_frame    = prev.current_frame;
    result.current_frame = (float32)glfwGetTime() * 1000;
    result.dt            = min((result.current_frame - result.last_frame) / 1000.0, 0.5);
    return result;
}