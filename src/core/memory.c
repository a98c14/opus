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