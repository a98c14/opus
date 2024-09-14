#include "base_thread_context.h"

per_thread ThreadContext* tctx_thread_local;

internal void
tctx_init_and_equip(ThreadContext* tctx)
{
    memory_zero_struct(tctx);
    Arena** arena_ptr = tctx->arenas;
    for (uint64 i = 0; i < array_count(tctx->arenas); i += 1, arena_ptr += 1)
    {
        *arena_ptr = arena_new_reserve(mb(64));
    }
    tctx_thread_local = tctx;
}

internal void
tctx_release(void)
{
    for (uint64 i = 0; i < array_count(tctx_thread_local->arenas); i += 1)
    {
        arena_release(tctx_thread_local->arenas[i]);
    }
}

internal ThreadContext*
tctx_get_equipped(void)
{
    return (tctx_thread_local);
}

internal Arena*
tctx_get_scratch(Arena** conflicts, uint64 count)
{
    ThreadContext* tctx = tctx_get_equipped();

    Arena*  result    = 0;
    Arena** arena_ptr = tctx->arenas;
    for (uint64 i = 0; i < array_count(tctx->arenas); i += 1, arena_ptr += 1)
    {
        Arena** conflict_ptr = conflicts;
        bool32  has_conflict = 0;
        for (uint64 j = 0; j < count; j += 1, conflict_ptr += 1)
        {
            if (*arena_ptr == *conflict_ptr)
            {
                has_conflict = 1;
                break;
            }
        }
        if (!has_conflict)
        {
            result = *arena_ptr;
            break;
        }
    }

    return (result);
}

internal void
tctx_set_thread_name(String string)
{
    ThreadContext* tctx = tctx_get_equipped();
    uint64         size = clamp_top(string.length, sizeof(tctx->thread_name));
    memcpy(tctx->thread_name, string.value, size);
    tctx->thread_name_size = size;
}

internal String
tctx_get_thread_name(void)
{
    ThreadContext* tctx   = tctx_get_equipped();
    String         result = string_create(tctx->thread_name, tctx->thread_name_size);
    return (result);
}

internal void
tctx_write_srcloc(char* file_name, uint64 line_number)
{
    ThreadContext* tctx = tctx_get_equipped();
    tctx->file_name     = file_name;
    tctx->line_number   = line_number;
}

internal void
tctx_read_srcloc(char** file_name, uint64* line_number)
{
    ThreadContext* tctx = tctx_get_equipped();
    *file_name          = tctx->file_name;
    *line_number        = tctx->line_number;
}
