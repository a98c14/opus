#pragma once

#include "defines.h"
#include "strings.h"

internal uint32        hash_uint64(uint64 value);
internal uint32        hash_xy(int32 x, int32 y);
internal inline uint64 hash_chars(const char* buffer);
internal inline uint64 hash_string(String str);
internal inline uint64 hash_array_int32(int32* arr, uint32 count);
internal inline uint64 hash_array_uint64(uint64* arr, uint32 count);
