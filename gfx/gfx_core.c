#include "gfx_core.h"

internal GFX_Handle
gfx_handle_zero(void)
{
    GFX_Handle result = {0};
    return result;
}

internal GFX_Camera
gfx_camera_new(float32 width, float32 height, float32 near_plane, float32 far_plane, uint32 window_width, uint32 window_height)
{
    GFX_Camera result;
    result.projection    = mat4_ortho(width, height, near_plane, far_plane);
    result.view          = mat4_identity();
    result.inverse_view  = mat4_identity();
    result.world_width   = width;
    result.world_height  = height;
    result.window_width  = window_width;
    result.window_height = window_height;
    return result;
}