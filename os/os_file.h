#pragma once
#include "os_core.h"

typedef enum
{
    OS_AccessFlag_Read       = (1 << 0),
    OS_AccessFlag_Write      = (1 << 1),
    OS_AccessFlag_Execute    = (1 << 2),
    OS_AccessFlag_Append     = (1 << 3),
    OS_AccessFlag_ShareRead  = (1 << 4),
    OS_AccessFlag_ShareWrite = (1 << 5),
} OS_AccessFlags;

/** Per OS Specific */
internal OS_Handle os_file_open(String path, OS_AccessFlags flags);
internal bool32    os_file_close(OS_Handle file);
internal uint64    os_file_size(OS_Handle file);
internal bool32    os_file_exists_at_path(String path);
internal bool32    os_file_delete_at_path(String path);
internal uint64    os_file_read(OS_Handle file, uint64 offset, uint64 size, void* out_data);
internal uint64    os_file_write(OS_Handle file, uint64 offset, uint64 size, void* data);

internal String os_file_get_working_directory(Arena* arena);

/** Generic */
internal Buffer os_file_read_all(OS_Handle file, Arena* arena);
internal String os_file_read_all_as_string(OS_Handle file, Arena* arena);
