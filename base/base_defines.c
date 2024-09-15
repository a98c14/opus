#include "base_defines.h"

/** safe cast */
internal uint16
safe_cast_uint16(uint32 x)
{
    xstatic_assert(x <= MAX_UINT16);
    uint16 result = (uint16)x;
    return result;
}

internal uint32
safe_cast_uint32(uint64 x)
{
    xstatic_assert(x <= MAX_UINT32);
    uint32 result = (uint32)x;
    return result;
}

internal int32
safe_cast_int32(int64 x)
{
    xstatic_assert(x <= MAX_INT32);
    int32 result = (int32)x;
    return result;
}

/** Bit Operations */
internal uint32
saturate_uint32_from_uint64(uint64 value)
{
    uint32 result = (value > MAX_UINT32) ? MAX_UINT32 : (uint32)value;
    return result;
}