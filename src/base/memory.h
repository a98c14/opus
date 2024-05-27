#pragma once
#include "asserts.h"
#include "defines.h"
#include <stdlib.h>
#include <string.h>

/* arena functions */
#define DEFAULT_RESERVE_SIZE mb(256)
#define COMMIT_BLOCK_SIZE    mb(64)

typedef struct
{
    uint64 cap;
    uint64 pos;
    uint64 commit_pos;
} Arena;

typedef struct
{
    Arena* arena;
    uint64 pos;
} ArenaTemp;

internal Arena* make_arena_reserve(uint64 reserve_size);
internal Arena* make_arena();

internal void  arena_release(Arena* arena);
internal void  arena_reset(Arena* arena);
internal void* arena_push(Arena* arena, uint64 size);
internal void  arena_pop(Arena* arena, uint64 size);
internal void  arena_pop_to(Arena* arena, uint64 pos);
internal void* arena_push_zero(Arena* arena, uint64 size);

internal ArenaTemp arena_begin_temp(Arena* arena);
internal void      arena_end_temp(ArenaTemp temp);

#define arena_push_array(arena, type, count)      (type*)arena_push((arena), sizeof(type) * (count))
#define arena_push_array_zero(arena, type, count) (type*)arena_push_zero((arena), sizeof(type) * (count))
#define arena_push_struct(arena, type)            arena_push_array((arena), type, 1)
#define arena_push_struct_zero(arena, type)       arena_push_array_zero((arena), type, 1)

/** ring buffers */
typedef struct
{
    uint8* base;
    uint64 size;
    uint64 write_pos;
    uint64 read_pos;

    // TODO(selim):
    // OS_Handle mutex;
    // OS_Handle cv;
} RingBuffer;

typedef struct
{
    uint64 position;
    uint64 size;
} RingBufferEntry;

internal RingBuffer      make_ring_buffer(Arena* arena, uint64 size);
internal RingBufferEntry ring_write(RingBuffer* buffer, void* src, uint64 size);
internal RingBufferEntry ring_write_contiguous(RingBuffer* buffer, void* src, uint64 size);
internal RingBufferEntry ring_read(RingBuffer* buffer, void* dst, uint32 size);
internal void            ring_read_entry(RingBuffer* buffer, void* dst, RingBufferEntry entry);
internal void*           ring_peek_entry(RingBuffer* buffer, RingBufferEntry entry);
#define ring_write_struct(buffer, ptr) ring_write((buffer), (ptr), sizeof(*(ptr)))
#define ring_read_struct(buffer, ptr)  ring_read((buffer), (ptr), sizeof(*(ptr)))
