#pragma once
#include "defines.h"
#include <stdlib.h>
#include <string.h>

/* Arena Functions */
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
internal void* arena_push_aligned(Arena* arena, uint64 size, uint8 alignment);
internal void  arena_pop(Arena* arena, uint64 size);
internal void  arena_pop_to(Arena* arena, uint64 pos);
internal void* arena_push_zero(Arena* arena, uint64 size);
internal void* arena_push_zero_aligned(Arena* arena, uint64 size, uint8 alignment);

internal ArenaTemp arena_begin_temp(Arena* arena);
internal void      arena_end_temp(ArenaTemp temp);

#define arena_push_array(arena, type, count)                         (type*)arena_push((arena), sizeof(type) * (count))
#define arena_push_array_aligned(arena, type, count, alignment)      (type*)arena_push_aligned((arena), sizeof(type) * (count), alignment)
#define arena_push_array_zero(arena, type, count)                    (type*)arena_push_zero((arena), sizeof(type) * (count))
#define arena_push_array_zero_aligned(arena, type, count, alignment) (type*)arena_push_zero_aligned((arena), sizeof(type) * (count), alignment)
#define arena_push_struct(arena, type)                               arena_push_array((arena), type, 1)
#define arena_push_struct_zero(arena, type)                          arena_push_array_zero((arena), type, 1)
#define arena_push_struct_zero_aligned(arena, type, alignment)       arena_push_array_zero_aligned((arena), type, 1, alignment)