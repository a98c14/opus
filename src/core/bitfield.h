#pragma once

#include <core/defines.h>

typedef uint64 BitField64;

internal void
bitfield_set(BitField64* field, uint8 index);

internal bool32
bitfield_is_set(BitField64 field, uint8 index);
