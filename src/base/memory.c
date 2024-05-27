#include "memory.h"

internal Arena*
make_arena_reserve(uint64 reserve_size)
{
    Arena* result      = 0;
    void*  memory      = malloc(reserve_size);
    result             = (Arena*)memory;
    result->cap        = reserve_size;
    result->commit_pos = reserve_size;
    result->pos        = sizeof(Arena);
    return result;
}

internal Arena*
make_arena()
{
    Arena* result = make_arena_reserve(DEFAULT_RESERVE_SIZE);
    return result;
}

internal void
arena_release(Arena* arena)
{
    free(arena);
}

internal void*
arena_push(Arena* arena, uint64 size)
{
    xassert(arena->pos + size <= arena->cap, "arena buffer overflow");
    void* result = 0;
    if (arena->pos + size <= arena->cap)
    {
        arena->pos = align_pow2(arena->pos, 16);
        result     = ((uint8*)arena) + arena->pos;
        arena->pos += size;
    }
    return result;
}

internal void*
arena_push_zero(Arena* arena, uint64 size)
{
    void* result = arena_push(arena, size);
    memset(result, 0, size);
    return result;
}

internal void
arena_pop(Arena* arena, uint64 size)
{
    arena->pos -= size;
}

// NOTE(selim): Never call this with pos=0, use `arena_reset` instead
// otherwise base memory arena handle will be overwritten.
internal void
arena_pop_to(Arena* arena, uint64 pos)
{
    // pos += sizeof(Arena);
    if (pos < arena->pos)
    {
        arena->pos = pos;
    }
}

internal void
arena_reset(Arena* arena)
{
    arena->pos = sizeof(Arena);
}

internal ArenaTemp
arena_begin_temp(Arena* arena)
{
    ArenaTemp temp = {arena, arena->pos};
    return temp;
}

internal void
arena_end_temp(ArenaTemp temp)
{
    arena_pop_to(temp.arena, temp.pos);
}

/** ring buffers */
internal RingBuffer
make_ring_buffer(Arena* arena, uint64 size)
{
    RingBuffer result = {0};
    result.base       = arena_push_array_zero(arena, uint8, size);
    result.size       = size;
    return result;
}

internal RingBufferEntry
ring_write(RingBuffer* buffer, void* src, uint64 size)
{
    xassert(size <= buffer->size, "ring buffer doesn't have enough room to write given data");
    RingBufferEntry result;
    result.position = buffer->write_pos;
    result.size     = size;

    uint64 ring_off           = buffer->write_pos % buffer->size;
    uint64 bytes_before_split = buffer->size - ring_off;
    uint64 pre_split_bytes    = min(bytes_before_split, size);
    uint64 pst_split_bytes    = size - pre_split_bytes;
    void*  pre_split_data     = src;
    void*  pst_split_data     = ((uint8*)src + pre_split_bytes);
    memory_copy(buffer->base + ring_off, pre_split_data, pre_split_bytes);
    memory_copy(buffer->base + 0, pst_split_data, pst_split_bytes);
    buffer->write_pos += size;

    return result;
}

internal RingBufferEntry
ring_write_contiguous(RingBuffer* buffer, void* src, uint64 size)
{
    xassert(size <= buffer->size, "ring buffer doesn't have enough room to write given data");
    uint64 ring_off = buffer->write_pos % buffer->size;
    if (ring_off + size > buffer->size)
        ring_off = 0;

    RingBufferEntry result;
    result.position = ring_off;
    result.size     = size;

    memory_copy(buffer->base + ring_off, src, size);
    buffer->write_pos += size;
    return result;
}

internal RingBufferEntry
ring_read(RingBuffer* buffer, void* dst, uint32 size)
{
    xassert(size <= buffer->size, "ring buffer doesn't have enough room to read given data");
    RingBufferEntry result;
    result.position = buffer->read_pos;
    result.size     = size;

    ring_read_entry(buffer, dst, result);
    buffer->read_pos += size;
    return result;
}

internal void
ring_read_entry(RingBuffer* buffer, void* dst, RingBufferEntry entry)
{
    xassert(entry.size <= buffer->size, "ring buffer doesn't have enough room to read given data");
    xassert(entry.position + buffer->size > buffer->write_pos, "trying to read overwritten data from ring buffer");
    uint64 ring_off           = entry.position % buffer->size;
    uint64 bytes_before_split = buffer->size - ring_off;
    uint64 pre_split_bytes    = min(bytes_before_split, entry.size);
    uint64 pst_split_bytes    = entry.size - pre_split_bytes;
    memory_copy(dst, buffer->base + ring_off, pre_split_bytes);
    memory_copy((uint8*)dst + pre_split_bytes, buffer->base + 0, pst_split_bytes);
}

internal void*
ring_peek_entry(RingBuffer* buffer, RingBufferEntry entry)
{
    uint64 ring_off = entry.position % buffer->size;
    xassert(entry.size <= buffer->size, "ring buffer doesn't have enough room to read given data");
    xassert(ring_off + entry.size < buffer->size, "trying to peek to an entry that wraps around. use `ring_write_contiguous` to create entries that doesn't wrap around.");
    return buffer->base + ring_off;
}