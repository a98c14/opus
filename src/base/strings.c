#include "strings.h"
internal String
string_new(Arena* arena, uint64 length)
{
    String s = {0};
    s.value  = arena_push(arena, length); // +1 for null-terminator
    s.length = length;
    return s;
}

internal String
string_null()
{
    return (String){.value = NULL, .length = -1};
}

internal String
string_create(char* buffer, uint32 size)
{
    String s = {0};
    s.value  = buffer;
    s.length = size;
    return s;
}

internal String
string_pushfv(Arena* arena, const char* fmt, va_list args)
{
    va_list args2;
    va_copy(args2, args);

    uint32 buffer_size = 32;
    char*  buffer      = arena_push_array(arena, char, buffer_size);
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
        char*  fixed_buffer = arena_push_array(arena, char, actual_size);
        uint32 final_size   = vsnprintf(fixed_buffer, actual_size + 1, fmt, args2);
        result              = string_create(fixed_buffer, final_size);
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

/** char helpers */
internal bool32
char_is_space(char c)
{
    return (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v');
}

internal bool32
char_is_upper(char c)
{
    return ('A' <= c && c <= 'Z');
}

internal bool32
char_is_lower(char c)
{
    return ('a' <= c && c <= 'z');
}

internal bool32
char_is_alpha(char c)
{
    return (char_is_upper(c) || char_is_lower(c));
}

internal bool32
char_is_slash(char c)
{
    return (c == '/' || c == '\\');
}

internal char
char_to_lower(char c)
{
    if (char_is_upper(c))
    {
        c += ('a' - 'A');
    }
    return (c);
}

internal char
char_to_upper(char c)
{
    if (char_is_lower(c))
    {
        c += ('A' - 'a');
    }
    return (c);
}

/** helpers */
internal String
string_skip(String str, uint64 amount)
{
    amount = min(amount, str.length);
    str.value += amount;
    str.length -= amount;
    return (str);
}

internal String
string_substr(String str, uint64 min, uint64 max)
{
    min = clamp_top(min, str.length);
    max = clamp_top(max, str.length);
    str.value += min;
    str.length = max - min;
    return str;
}

/* string list */
internal void
string_list_push(Arena* arena, StringList* list, String str)
{
    StringNode* node = arena_push_struct_zero(arena, StringNode);
    node->value      = str;
    if (list->last)
        list->last->next = node;

    if (!list->first)
        list->first = node;

    list->last = node;
    list->count++;
}