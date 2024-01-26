#pragma once

#include "asserts.h"
#include "datetime.h"
#include "defines.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <vadefs.h>

#if DEBUG == 0
#define LOG_TO_FILE 1
#else
#define LOG_TO_FILE 0
#endif
#define LOG_TO_CONSOLE         1
#define LOG_MESSAGE_SIZE_LIMIT 32000

#define LOG_WARN_ENABLED  1
#define LOG_INFO_ENABLED  1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 0 // Warning: Emits a lot of logs, best to enable while debugging

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

internal bool32 logger_init();

internal void logger_flush();

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

internal void log_assertion_failure(const char* expr, const char* message, const char* file, int32 line);
