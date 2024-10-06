#pragma once
#include "os_core.h"

typedef void WindowKeyCallback(OS_Handle window_handle, int32 key, int32 scancode, int32 action, int32 mods);

/** Per OS Specific */
internal OS_Handle os_window_create(int32 width, int32 height, String name, WindowKeyCallback key_callback);
internal void      os_window_update(OS_Handle window_handle);
internal void      os_window_destroy(OS_Handle window_handle);
internal bool32    os_window_should_close(OS_Handle window_handle);
internal bool32    os_window_is_ready();