#pragma once
// TODO(selim): Move this to OS layer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base_defines.h"
#include "base_memory.h"
#include "base_strings.h"
#include "base_log.h"

#ifdef _WIN32
#include <direct.h>
#define get_current_directory_internal _getcwd
#else
#include <unistd.h>
#define get_current_directory_internal getcwd
#endif

internal String get_current_directory(Arena* arena);
internal String get_working_directory(Arena* arena);
internal String file_read_all_as_string(Arena* arena, String path);
