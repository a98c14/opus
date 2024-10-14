#include "time.h"

internal EngineTime
engine_time_new(void)
{
    EngineTime result         = {0};
    result.current_frame_time = (float32)glfwGetTime();
    return result;
}

internal EngineTime
engine_get_time(EngineTime prev)
{
    EngineTime result         = {0};
    result.last_frame_time    = prev.current_frame_time;
    result.current_frame_time = (float32)glfwGetTime() * 1000;
    result.dt                 = min((result.current_frame_time - result.last_frame_time) / 1000.0, 0.5);
    return result;
}