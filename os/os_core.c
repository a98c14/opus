#include "os_core.h"

internal bool32
os_handle_match(OS_Handle a, OS_Handle b)
{
    return a.v == b.v;
}

internal OS_Handle
os_handle_zero(void)
{
    OS_Handle handle = {0};
    return handle;
}

internal bool32
os_handle_is_zero(OS_Handle h)
{
    return os_handle_match(h, os_handle_zero());
}
