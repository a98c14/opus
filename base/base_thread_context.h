#pragma once
#include "base_defines.h"
#include "base_memory.h"
#include "base_strings.h"

typedef struct
{
    Arena* arenas[2];

    char   thread_name[32];
    uint64 thread_name_size;

    char*  file_name;
    uint64 line_number;
} ThreadContext;

internal void           tctx_init_and_equip(ThreadContext* tctx);
internal void           tctx_release(void);
internal ThreadContext* tctx_get_equipped(void);

internal Arena* tctx_get_scratch(Arena** conflicts, uint64 count);

internal void   tctx_set_thread_name(String name);
internal String tctx_get_thread_name(void);

internal void tctx_write_srcloc(char* file_name, uint64 line_number);
internal void tctx_read_srcloc(char** file_name, uint64* line_number);
#define tctx_write_this_srcloc() tctx_write_srcloc(__FILE__, __LINE__)

#define scratch_begin(conflicts, count) arena_begin_temp(tctx_get_scratch((conflicts), (count)))
#define scratch_end(scratch)            arena_end_temp(scratch)
