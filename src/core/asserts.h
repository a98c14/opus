#pragma once

#include "log.h"

#define ENABLE_ASSERT 1
#if ENABLE_ASSERT
#include <stdio.h>
#include <stdlib.h>
#if _MSC_VER
#include <intrin.h>
#define assert_break() __debugbreak()
#else
#define assert_break() __builtin_trap()
#endif
#define xassert(condition, message) statement(if (!(condition)) { log_assertion_failure(#condition, message, __FILE__, __LINE__); assert_break(); })
#define not_implemented()           statement(log_assertion_failure(this_function_name, "not implemented", __FILE__, __LINE__); assert_break();)
#else
#define xassert(condition, message)
#endif
