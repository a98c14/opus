#include "os_file.h"

internal Buffer
os_file_read_all(OS_Handle file, Arena* arena)
{
    Buffer result    = {0};
    uint64 file_size = os_file_size(file);
    result.data      = arena_push_array(arena, uint8, file_size);
    result.size      = os_file_read(file, 0, MAX_UINT64, result.data);
    return result;
}

internal String
os_file_read_all_as_string(OS_Handle file, Arena* arena)
{
    Buffer buffer = os_file_read_all(file, arena);
    return string_create(buffer.data, buffer.size);
}