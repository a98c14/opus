#include "bitfield.h"

internal void
bitfield_set(BitField64* field, uint8 index)
{
    *field = *field | (1 << index);
}

internal void
bitfield_clear(BitField64* field)
{
    *field = 0;
}

internal bool32
bitfield_is_set(BitField64 field, uint8 index)
{
    return (field & (1 << index)) > 0;
}

internal bool32
bitfield_is_activated(BitField64 prev, BitField64 next, uint8 index)
{
    return !bitfield_is_set(prev, index) && bitfield_is_set(next, index);
}