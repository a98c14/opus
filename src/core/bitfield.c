#include "bitfield.h"

internal bool32
bitfield_is_set(BitField64 field, uint8 index)
{
    return field & (1 << index);
}