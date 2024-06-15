#pragma once
#include "os_core.h"

#if OS_WINDOWS
#include "win/os_core_win32.h"
#else
#error no OS layer setup
#endif
