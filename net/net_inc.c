#include "net_inc.h"

#include "net_core.c"

#if OS_WINDOWS
#include "win/net_impl_win.c"
#else
#error no OS layer setup
#endif