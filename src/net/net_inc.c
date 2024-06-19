#include "net_core.c"

#if OS_WINDOWS
#include "win/net_core_win32.c"
#else
#error no OS layer setup
#endif