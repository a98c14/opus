#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <vadefs.h>
#include "base_defines.h"
#include "base_math.h"
#include "base_memory.h"

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

    StringNode* next;
};

typedef struct
{
    StringNode* first;
    StringNode* last;
    uint32      count;
    uint32      size;
} StringList;

typedef struct
{
    String pre;
    String sep;
    String post;
} StringJoin;

internal StringList  string_list();
internal StringNode* string_list_push(Arena* arena, StringList* list, String str);
internal StringNode* string_list_pushf(Arena* arena, StringList* list, char* fmt, ...);
internal StringNode* string_list_push_front(Arena* arena, StringList* list, String str);
internal StringNode* string_list_push_frontf(Arena* arena, StringList* list, char* fmt, ...);
internal String      string_list_join(Arena* arena, StringList* list, StringJoin* optional_params);

/** string storage */
typedef uint64 StaticStringIndex;

typedef struct
{
    Arena* arena;

    uint64  capacity;
    uint64  count;
    String* static_strings;
} StaticStringStorage;
global StaticStringStorage* g_static_string_storage;

internal void              string_storage_init(Arena* arena, uint64 capacity);
internal StaticStringIndex string_static_new(String str);
internal String            string_static_get(StaticStringIndex index);