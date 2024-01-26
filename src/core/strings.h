#pragma once

#include <core/strings.h>
#include <stdarg.h>
#include <stdio.h>
#include <vadefs.h>

#include "defines.h"
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

typedef struct
{
    char*  value;
    uint64 length;
} String;

/* Base */
internal String string_new(Arena* arena, uint64 length);
internal String string_create(char* buffer, uint32 size);
internal String string_null();
internal String string_pushfv(Arena* arena, const char* fmt, va_list args);
internal String string_pushf(Arena* arena, const char* fmt, ...);

/* Utility */
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