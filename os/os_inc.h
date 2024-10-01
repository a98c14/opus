#pragma once
#include "os_core.h"
#include "os_file.h"

#if OS_WINDOWS
#include "win/os_core_win32.h"
#include "win/os_file_win32.h"
#include "glfw/os_window_glfw.h"
#else
#error no OS layer setup
#endif
