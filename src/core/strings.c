#include "strings.h"

internal String 
string_new(Arena* arena, uint64 length)
{
    String s = {0};
    s.value = arena_push(arena, length); // +1 for null-terminator
    s.length = length;
    return s;
}

internal String
string_null()
{
    return (String){ .value = NULL, .length = -1 };
}

internal String 
string_create(char* buffer, uint32 size)
{
    String s = {0};
    s.value = buffer;
    s.length = size;
    return s;
}

internal String 
string_pushfv(Arena* arena, const char* fmt, va_list args)
{
    va_list args2;
    va_copy(args2, args);

    uint32 buffer_size = 32;
    char* buffer = arena_push_array(arena, char, buffer_size);
    uint32 actual_size = vsnprintf(buffer, buffer_size, fmt, args);

    String result = {0};
    if (actual_size < buffer_size) 
    {
        arena_pop(arena, buffer_size - actual_size - 1);
        result = string_create(buffer, actual_size);
    }
    else
    {
        arena_pop(arena, buffer_size);
        char* fixed_buffer  = arena_push_array(arena, char, actual_size);
        uint32 final_size = vsnprintf(fixed_buffer, actual_size + 1, fmt, args2);
        result = string_create(fixed_buffer, final_size);
    }

    va_end(args2);

    return result;
}

internal String 
string_pushf(Arena* arena, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String result = string_pushfv(arena, fmt, args);
    va_end(args);
    return result;
}