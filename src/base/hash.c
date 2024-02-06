#include "hash.h"

internal uint32
hash_uint64(uint64 value)
{
    value = (value ^ (value >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    value = (value ^ (value >> 27)) * UINT64_C(0x94d049bb133111eb);
    value = value ^ (value >> 31);
    return (uint32)value;
}

internal uint32
hash_xy(int32 x, int32 y)
{
    int32 result = (x * 92837111) ^ (y * 689287499);
    return abs(result);
}

internal uint64
hash_chars(const char* buffer)
{
    uint64 hash = 5381;
    int32  c;
    while ((c = *buffer++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

internal uint64
hash_string(String str)
{
    uint64 hash = 5381;
    for (uint64 i = 0; i < str.length; i++)
        hash = ((hash << 5) + hash) + (int32)str.value[i]; /* hash * 33 + c */
    return hash;
}
