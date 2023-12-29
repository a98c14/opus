#include "file.h"

internal String
get_current_directory(Arena* arena)
{
    char* temp_buffer = get_current_directory_internal(NULL, 4096);
    uint64 length = strlen(temp_buffer);
    char* buffer = arena_push_array_zero(arena, char, length);
    memcpy(buffer, temp_buffer, length);
    return string_create(buffer, length);
}

internal String
get_working_directory(Arena* arena)
{
    // TODO(selim): Fix the <windows.h> include issue
    return string_null();
    // uint64 initial_pos = arena->pos;
    // uint32 path_size = 4096;
    // char* path = arena_push_array_zero(arena, char, path_size);
    // uint32 actual_size = GetModuleFileName(0, path, path_size - 1);
    // if(actual_size == 0)
    // {
    //     arena_pop_to(arena, initial_pos);
    //     return string_null();
    // }
    // else if(actual_size < path_size - 1)
    // {
    //     arena_pop_to(arena, initial_pos + actual_size + 1);
    //     return string_create(path, actual_size + 1);
    // }
    // else
    // {
    //     printf("[ERROR] Insufficient buffer size for working directory");
    //     arena_pop_to(arena, initial_pos);
    //     return string_null();
    // }
}

internal String
file_read_all_as_string(Arena* arena, String path)
{
    FILE *file = fopen(path.value, "rb"); // open in binary mode
    if (!file) {
        log_error("failed to open file  %s", path.value);
        return string_null();
    }

    // Determine file size
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);  // reset file position

    uint64 arena_initial_pos = arena->pos;
    String s = string_new(arena, fsize+1);   // +1 for null-terminator
    usize read = fread(s.value, 1, fsize, file);
    if (read != fsize) {
        printf("[ERROR] Error reading file");
        arena->pos = arena_initial_pos;
        fclose(file);
        return string_null();
    }
    
    s.value[fsize] = 0;  // null-terminate the content
    fclose(file);
    return s;
}
