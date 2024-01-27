#include "log.h"

global FILE* log_file;

internal void
log_output(LogLevel level, const char* message, ...)
{
    const char* level_strings[6] = {"[FATAL] ", "[ERROR] ", "[WARN] ", "[INFO] ", "[DEBUG] ", "[TRACE] "};
    bool32      is_error         = level <= LogLevelError;

    char out_buffer[LOG_MESSAGE_SIZE_LIMIT];
    memset(out_buffer, 0, sizeof(out_buffer));

    va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(out_buffer, LOG_MESSAGE_SIZE_LIMIT, message, arg_ptr);
    va_end(arg_ptr);

    char print_buffer[LOG_MESSAGE_SIZE_LIMIT];
    sprintf(print_buffer, "%-8s%s\n", level_strings[level], out_buffer);

#if LOG_TO_FILE == 1
    char     date_buffer[128];
    DateTime date = date_now();
    sprintf(date_buffer, "[%d/%02d/%02d|%02d:%02d:%02d]", 1900 + date.year, 1 + date.month, date.monthday, date.hour, date.min, date.sec);
    fprintf(log_file, "%s%s", date_buffer, print_buffer);
#endif
#if LOG_TO_CONSOLE == 1
    printf("%s", print_buffer);
#endif
}

internal bool32
logger_init()
{
#if LOG_TO_FILE == 1
    DateTime date = date_now();
    char     log_file_name_buffer[256];
    sprintf(log_file_name_buffer, "./opus_log_%d%02d%02d%02d%02d%02d.txt", 1900 + date.year, 1 + date.month, date.monthday, date.hour, date.min, date.sec);
    errno_t err = fopen_s(&log_file, log_file_name_buffer, "w");
    if (err != 0)
        return false;
#endif
    return true;
}

internal void
logger_flush()
{
#if LOG_TO_FILE == 1
    fflush(log_file);
#endif
}

internal void
log_assertion_failure(const char* message, const char* function, const char* file, int32 line)
{
    log_output(LogLevelFatal, "assertion failure, message: '%s', in function %s, in file %s, line: %d\n", message, function, file, line);
}

internal void
log_assertion_failure_expr(const char* expr, const char* message, const char* function, const char* file, int32 line)
{
    log_output(LogLevelFatal, "assertion failure: %s, message: '%s', in function %s, in file %s, line: %d\n", expr, message, function, file, line);
}