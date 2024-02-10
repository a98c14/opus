#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <vadefs.h>

#include "defines.h"
#include "math.h"
#include "memory.h"

#define string(s)             \
    (String)                  \
    {                         \
        (char*)s, lengthof(s) \
    }

#define string_comp(s)         \
    {                          \
        (char*)s, lengthof(s), \
    }

typedef uint32 StringMatchFlags;
enum
{
    StringMatchFlag_CaseInsensitive  = (1 << 0),
    StringMatchFlag_RightSideSloppy  = (1 << 1),
    StringMatchFlag_SlashInsensitive = (1 << 2),
};

typedef struct
{
    char*  value;
    uint64 length;
} String;

typedef struct
{
    char   value[4];
    uint32 length;
} String4;

typedef struct
{
    char   value[8];
    uint32 length;
} String8;

/* base */
internal String string_new(Arena* arena, uint64 length);
internal String string_create(char* buffer, uint32 size);
internal String string_null();
internal String string_pushfv(Arena* arena, const char* fmt, va_list args);
internal String string_pushf(Arena* arena, const char* fmt, ...);
internal String string_range(char* first, char* one_past_last);

/** slicing */
internal String string_skip(String str, uint64 amount);
internal String string_substr(String str, uint64 min, uint64 max);

/** matching */
internal bool32 string_match(String a, String b, StringMatchFlags flags);
internal uint64 string_find(String string, String substr, uint64 start_pos, StringMatchFlags flags);

/** char conversion */
internal bool32 char_is_space(char c);
internal bool32 char_is_upper(char c);
internal bool32 char_is_lower(char c);
internal bool32 char_is_alpha(char c);
internal bool32 char_is_slash(char c);
internal char   char_to_lower(char c);
internal char   char_to_upper(char c);

/* string list */
typedef struct StringNode StringNode;
struct StringNode
{
    String value;

    StringNode* prev;
    StringNode* next;
};

typedef struct
{
    StringNode* first;
    StringNode* last;
    uint32      count;
} StringList;

internal void string_list_push(Arena* arena, StringList* list, String str);