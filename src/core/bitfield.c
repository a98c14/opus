#include "bitfield.h"

internal void
bitfield_set(BitField64* field, uint8 index)
{
    *field = *field | (1 << index);
}

internal bool32
bitfield_is_set(BitField64 field, uint8 index)
{
    return (field & (1 << index)) > 0;
}