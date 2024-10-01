#include "os_core.c"
#include "os_file.c"

#if OS_WINDOWS
#include "win/os_core_win32.c"
#include "win/os_file_win32.c"
#include "glfw/os_window_glfw.c"
#else
#error no OS layer setup
#endif