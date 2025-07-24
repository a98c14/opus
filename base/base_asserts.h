#pragma once
#include "base_defines.h"
#include "base_log.h"

#if BUILD_DEBUG
#include <stdio.h>
#include <stdlib.h>
#define xassert_m(condition, message) statement(if (!(condition)) { log_assertion_failure_expr(#condition, message, this_function_name, __FILE__, __LINE__); assert_break(); })
#define xassert(condition)            statement(if (!(condition)) { assert_break(); })
#define not_implemented()             statement(log_assertion_failure("not_implemented", this_function_name, __FILE__, __LINE__); assert_break();)
#else
#define xassert_m(condition, message)
#define xassert(condition)
#define not_implemented()
#endif
