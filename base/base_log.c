#include "base_log.h"

internal void
log_output(LogLevel level, const char* message, ...)
{
    const char* level_strings[6] = {"[FATAL] ", "[ERROR] ", "[WARN] ", "[INFO] ", "[DEBUG] ", "[TRACE] "};

    char out_buffer[LOG_MESSAGE_SIZE_LIMIT];
    memset(out_buffer, 0, sizeof(out_buffer));

    va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(out_buffer, LOG_MESSAGE_SIZE_LIMIT, message, arg_ptr);
    va_end(arg_ptr);

    char print_buffer[LOG_MESSAGE_SIZE_LIMIT];
    sprintf_s(print_buffer, LOG_MESSAGE_SIZE_LIMIT, "%-8s%s\n", level_strings[level], out_buffer);

#if LOG_TO_CONSOLE == 1
    printf("%s", print_buffer);
#endif
}

internal void
log_assertion_failure(const char* message, const char* function, const char* file, int32 line)
{
    log_output(LogLevelFatal, "Assertion failure, message: '%s', in function %s, in file %s, line: %d\n", message, function, file, line);
}

internal void
log_assertion_failure_expr(const char* expr, const char* message, const char* function, const char* file, int32 line)
{
    log_output(LogLevelFatal, "Assertion failure: %s, message: '%s', in function %s, in file %s, line: %d\n", expr, message, function, file, line);
}