#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <vadefs.h>
#include "defines.h"
#include "memory.h"

#define string(s) (String){(char*)s, lengthof(s)}

typedef struct
{
    char* value;
    uint64 length;
} String;

/* Functions */
internal String 
string_new(Arena* arena, uint64 length);

internal String 
string_create(char* buffer, uint32 size);

internal String
string_null();

internal String 
string_pushfv(Arena* arena, const char* fmt, va_list args);

internal String 
string_pushf(Arena* arena, const char* fmt, ...);
