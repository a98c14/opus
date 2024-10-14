#pragma once

#include "base_defines.h"
#include "base_datetime.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <vadefs.h>

#define LOG_TO_CONSOLE         1
#define LOG_MESSAGE_SIZE_LIMIT 32000

#ifndef LOG_WARN_ENABLED
#define LOG_WARN_ENABLED 1
#endif

#ifndef LOG_INFO_ENABLED
#define LOG_INFO_ENABLED 1
#endif

#ifndef LOG_DEBUG_ENABLED
#define LOG_DEBUG_ENABLED 1
#endif

#ifndef LOG_TRACE_ENABLED
#define LOG_TRACE_ENABLED 0 // Warning: Emits a lot of logs, best to enable while debugging
#endif

typedef enum
{
    LogLevelFatal,
    LogLevelError,
    LogLevelWarn,
    LogLevelInfo,
    LogLevelDebug,
    LogLevelTrace,
} LogLevel;

internal void
log_output(LogLevel level, const char* message, ...);

#ifndef log_fatal
#define log_fatal(message, ...) log_output(LogLevelFatal, message, ##__VA_ARGS__)
#endif

#ifndef log_error
#define log_error(message, ...) log_output(LogLevelError, message, ##__VA_ARGS__)
#endif

#if LOG_WARN_ENABLED == 1
#define log_warn(message, ...) log_output(LogLevelWarn, message, ##__VA_ARGS__)
#else
#define log_warn(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
#define log_info(message, ...) log_output(LogLevelInfo, message, ##__VA_ARGS__)
#else
#define log_info(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
#define log_debug(message, ...) log_output(LogLevelDebug, message, ##__VA_ARGS__)
#else
#define log_debug(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
#define log_trace(message, ...) log_output(LogLevelTrace, message, ##__VA_ARGS__)
#else
#define log_trace(message, ...)
#endif

internal void log_assertion_failure(const char* message, const char* function, const char* file, int32 line);
internal void log_assertion_failure_expr(const char* expr, const char* message, const char* function, const char* file, int32 line);
