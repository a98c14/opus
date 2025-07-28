#include "base_strings.h"

internal String
string_new(Arena* arena, uint64 length)
{
    String s = {0};
    s.value  = arena_push(arena, length); // +1 for null-terminator
    s.length = length;
    return s;
}

internal String
string_null(void)
{
    return (String){.value = NULL, .length = 0};
}

internal String
string_create(char* buffer, uint64 size)
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

internal String
string_range(char* first, char* one_past_last)
{
    String result = {first, (uint64)(one_past_last - first)};
    return (result);
}

internal String16
string16(uint16* value, uint64 size)
{
    String16 result = {value, size};
    return result;
}

/** unicode */
// clang-format off
read_only global uint8 utf8_class[32] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
};
// clang-format on

internal String
utf8(Arena* arena, int32 codepoint)
{
    char   buffer[4];
    uint32 length = utf8_encode(buffer, codepoint);

    String result = {0};
    result.value  = arena_push_array_zero(arena, char, length);
    result.length = length;
    memory_copy(result.value, buffer, length);
    return result;
}

internal UnicodeDecode
utf8_decode(char* str, uint64 max)
{
    UnicodeDecode result     = {1, MAX_UINT32};
    uint8         byte       = str[0];
    uint8         byte_class = utf8_class[byte >> 3];
    switch (byte_class)
    {
    case 1:
    {
        result.codepoint = byte;
    }
    break;
    case 2:
    {
        if (2 < max)
        {
            uint8 cont_byte = str[1];
            if (utf8_class[cont_byte >> 3] == 0)
            {
                result.codepoint = (byte & bitmask5) << 6;
                result.codepoint |= (cont_byte & bitmask6);
                result.inc = 2;
            }
        }
    }
    break;
    case 3:
    {
        if (2 < max)
        {
            uint8 cont_byte[2] = {str[1], str[2]};
            if (utf8_class[cont_byte[0] >> 3] == 0 &&
                utf8_class[cont_byte[1] >> 3] == 0)
            {
                result.codepoint = (byte & bitmask4) << 12;
                result.codepoint |= ((cont_byte[0] & bitmask6) << 6);
                result.codepoint |= (cont_byte[1] & bitmask6);
                result.inc = 3;
            }
        }
    }
    break;
    case 4:
    {
        if (3 < max)
        {
            uint8 cont_byte[3] = {str[1], str[2], str[3]};
            if (utf8_class[cont_byte[0] >> 3] == 0 &&
                utf8_class[cont_byte[1] >> 3] == 0 &&
                utf8_class[cont_byte[2] >> 3] == 0)
            {
                result.codepoint = (byte & bitmask3) << 18;
                result.codepoint |= ((cont_byte[0] & bitmask6) << 12);
                result.codepoint |= ((cont_byte[1] & bitmask6) << 6);
                result.codepoint |= (cont_byte[2] & bitmask6);
                result.inc = 4;
            }
        }
    }
    }
    return result;
}

internal UnicodeDecode
utf16_decode(uint16* str, uint64 max)
{
    UnicodeDecode result = {1, MAX_UINT32};
    result.codepoint     = str[0];
    result.inc           = 1;
    if (max > 1 && 0xD800 <= str[0] && str[0] < 0xDC00 && 0xDC00 <= str[1] && str[1] < 0xE000)
    {
        result.codepoint = ((str[0] - 0xD800) << 10) | ((str[1] - 0xDC00) + 0x10000);
        result.inc       = 2;
    }
    return (result);
}

internal uint32
utf8_encode(char* str, int32 codepoint)
{
    uint32 inc = 0;
    if (codepoint <= 0x7F)
    {
        str[0] = (char)codepoint;
        inc    = 1;
    }
    else if (codepoint <= 0x7FF)
    {
        str[0] = (char)((bitmask2 << 6) | ((codepoint >> 6) & bitmask5));
        str[1] = (char)(bit8 | (codepoint & bitmask6));
        inc    = 2;
    }
    else if (codepoint <= 0xFFFF)
    {
        str[0] = (char)((bitmask3 << 5) | ((codepoint >> 12) & bitmask4));
        str[1] = (char)(bit8 | ((codepoint >> 6) & bitmask6));
        str[2] = (char)(bit8 | (codepoint & bitmask6));
        inc    = 3;
    }
    else if (codepoint <= 0x10FFFF)
    {
        str[0] = (char)((bitmask4 << 4) | ((codepoint >> 18) & bitmask3));
        str[1] = (char)(bit8 | ((codepoint >> 12) & bitmask6));
        str[2] = (char)(bit8 | ((codepoint >> 6) & bitmask6));
        str[3] = (char)(bit8 | (codepoint & bitmask6));
        inc    = 4;
    }
    else
    {
        str[0] = '?';
        inc    = 1;
    }
    return (inc);
}

internal uint32
utf16_encode(uint16* str, int32 codepoint)
{
    uint32 inc = 1;
    if (codepoint == MAX_INT32)
    {
        str[0] = (uint16)'?';
    }
    else if (codepoint < 0x10000)
    {
        str[0] = (uint16)codepoint;
    }
    else
    {
        uint32 v = codepoint - 0x10000;
        str[0]   = safe_cast_uint16(0xD800 + (v >> 10));
        str[1]   = safe_cast_uint16(0xDC00 + (v & bitmask10));
        inc      = 2;
    }
    return (inc);
}

internal uint32
utf8_from_utf32_single(char* buffer, int32 character)
{
    return (utf8_encode(buffer, character));
}

/** string conversion */
internal String16
str16_from_8(Arena* arena, String in)
{
    uint64        cap  = in.length * 2;
    uint16*       str  = arena_push_array(arena, uint16, cap + 1);
    char*         ptr  = in.value;
    char*         opl  = ptr + in.length;
    uint64        size = 0;
    UnicodeDecode consume;
    for (; ptr < opl; ptr += consume.inc)
    {
        consume = utf8_decode(ptr, opl - ptr);
        size += utf16_encode(str + size, consume.codepoint);
    }
    str[size] = 0;
    arena_pop(arena, (cap - size) * 2);
    return string16(str, size);
}

internal String
str8_from_16(Arena* arena, String16 in)
{
    uint64  cap  = in.size * 3;
    char*   str  = arena_push_array(arena, char, cap + 1);
    uint16* ptr  = in.value;
    uint16* opl  = ptr + in.size;
    uint64  size = 0;

    UnicodeDecode consume;
    for (; ptr < opl; ptr += consume.inc)
    {
        consume = utf16_decode(ptr, opl - ptr);
        size += utf8_encode(str + size, consume.codepoint);
    }
    str[size] = 0;
    arena_pop(arena, (cap - size));
    return string_create((char*)str, size);
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

internal char
char_to_correct_slash(char c)
{
    if (char_is_slash(c))
    {
        c = '/';
    }
    return (c);
}

/** slicing */
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

/** matching */
internal bool32
string_match(String a, String b, StringMatchFlags flags)
{
    bool32 result = 0;
    if (a.length == b.length || (flags & StringMatchFlag_RightSideSloppy))
    {
        bool32 case_insensitive  = (flags & StringMatchFlag_CaseInsensitive);
        bool32 slash_insensitive = (flags & StringMatchFlag_SlashInsensitive);
        uint64 size              = min(a.length, b.length);
        result                   = 1;
        for (uint64 i = 0; i < size; i += 1)
        {
            uint8 at = a.value[i];
            uint8 bt = b.value[i];
            if (case_insensitive)
            {
                at = char_to_upper(at);
                bt = char_to_upper(bt);
            }
            if (slash_insensitive)
            {
                at = char_to_correct_slash(at);
                bt = char_to_correct_slash(bt);
            }
            if (at != bt)
            {
                result = 0;
                break;
            }
        }
    }
    return (result);
}

internal uint64
string_find(String string, String substr, uint64 start_pos, StringMatchFlags flags)
{
    char*  p           = string.value + start_pos;
    uint64 stop_offset = max(string.length + 1, substr.length) - substr.length;
    char*  stop_p      = string.value + stop_offset;
    if (substr.length > 0)
    {
        char*            string_opl                 = string.value + string.length;
        String           search_tail                = string_skip(substr, 1);
        StringMatchFlags adjusted_flags             = flags | StringMatchFlag_RightSideSloppy;
        char             search_first_char_adjusted = substr.value[0];
        if (adjusted_flags & StringMatchFlag_CaseInsensitive)
        {
            search_first_char_adjusted = char_to_upper(search_first_char_adjusted);
        }
        for (; p < stop_p; p += 1)
        {
            char target_char_adjusted = *p;
            if (adjusted_flags & StringMatchFlag_CaseInsensitive)
            {
                target_char_adjusted = char_to_upper(target_char_adjusted);
            }
            if (target_char_adjusted == search_first_char_adjusted)
            {
                if (string_match(string_range(p + 1, string_opl), search_tail, adjusted_flags))
                {
                    break;
                }
            }
        }
    }
    uint64 result = string.length;
    if (p < stop_p)
    {
        result = (uint64)(p - string.value);
    }
    return (result);
}

/* string list */
internal StringList
string_list(void)
{
    StringList result = {0};
    return result;
}

internal StringNode*
string_list_push(Arena* arena, StringList* list, String str)
{
    StringNode* node = arena_push_struct_zero(arena, StringNode);
    node->value      = str;
    queue_push(list->first, list->last, node);
    list->count += 1;
    list->size += str.length;
    return node;
}

internal StringNode*
string_list_push_front(Arena* arena, StringList* list, String str)
{
    StringNode* node = arena_push_struct_zero(arena, StringNode);
    node->value      = str;
    queue_push_front(list->first, list->last, node);
    list->count += 1;
    list->size += str.length;
    return node;
}

internal StringNode*
string_list_pushf(Arena* arena, StringList* list, char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String      string = string_pushfv(arena, fmt, args);
    StringNode* result = string_list_push(arena, list, string);
    va_end(args);
    return (result);
}

internal StringNode*
string_list_push_frontf(Arena* arena, StringList* list, char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String      string = string_pushfv(arena, fmt, args);
    StringNode* result = string_list_push_front(arena, list, string);
    va_end(args);
    return (result);
}

internal String
string_list_join(Arena* arena, StringList* list, StringJoin* optional_params)
{
    StringJoin join = {0};
    if (optional_params != 0)
    {
        memory_copy_struct(&join, optional_params);
    }

    uint64 sep_count = 0;
    if (list->count > 0)
    {
        sep_count = list->count - 1;
    }

    String result;
    result.length = join.pre.length + join.post.length + sep_count * join.sep.length + list->size;
    char* ptr = result.value = arena_push_array(arena, char, result.length + 1);

    memory_copy(ptr, join.pre.value, join.pre.length);
    ptr += join.pre.length;
    for (StringNode* node = list->first;
         node != 0;
         node = node->next)
    {
        memory_copy(ptr, node->value.value, node->value.length);
        ptr += node->value.length;
        if (node->next != 0)
        {
            memory_copy(ptr, join.sep.value, join.sep.length);
            ptr += join.sep.length;
        }
    }
    memory_copy(ptr, join.post.value, join.post.length);
    ptr += join.post.length;

    *ptr = 0;
    return (result);
}

internal StringList
string_split(Arena* arena, String str, String separator)
{
    StringList list         = {0};
    bool32     keep_empties = true; // TODO(selim): this should be a parameter?
    char*      ptr          = str.value;
    char*      opl          = str.value + str.length;
    for (; ptr < opl;)
    {
        char* first = ptr;
        for (; ptr < opl; ptr += 1)
        {
            char   c        = *ptr;
            bool32 is_split = 0;
            for (uint64 i = 0; i < separator.length; i += 1)
            {
                if (separator.value[i] == c)
                {
                    is_split = 1;
                    break;
                }
            }

            if (is_split)
            {
                break;
            }
        }

        String string = string_range(first, ptr);
        if (keep_empties || string.length > 0)
        {
            string_list_push(arena, &list, string);
        }
        ptr += 1;
    }

    return (list);
}

/** string storage */
internal void
string_storage_init(Arena* arena, uint64 capacity)
{
    g_static_string_storage                 = arena_push_struct_zero(arena, StaticStringStorage);
    g_static_string_storage->static_strings = arena_push_array(arena, String, capacity);
    g_static_string_storage->arena          = arena;
}

internal StaticStringIndex
string_static_new(String str)
{
    StaticStringIndex index                        = g_static_string_storage->count;
    g_static_string_storage->static_strings[index] = string_new(g_static_string_storage->arena, str.length);
    memcpy(g_static_string_storage->static_strings[index].value, str.value, str.length);
    g_static_string_storage->count++;
    return index;
}

internal String
string_static_get(StaticStringIndex index)
{
    return g_static_string_storage->static_strings[index];
}

/** utility */
internal bool32
string_is_empty(String str)
{
    return str.value != 0 && str.length <= 0;
}