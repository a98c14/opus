#include "base_datetime.h"
#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

internal DateTime
date_now(void)
{
    DateTime  result = {0};
    struct tm new_time;
    int64     long_time;
    int32     ms = 0;

#ifdef _WIN32
    struct _timeb time_buffer;
    _ftime64_s(&time_buffer);
    long_time   = time_buffer.time;
    ms          = time_buffer.millitm;
    errno_t err = localtime_s(&new_time, &long_time);
    xstatic_assert(err == 0);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long_time    = tv.tv_sec;
    milliseconds = tv.tv_usec / 1000;
    localtime_r(&long_time, &new_time);
#endif

    result.hour     = new_time.tm_hour;
    result.isdst    = new_time.tm_isdst;
    result.monthday = new_time.tm_mday;
    result.min      = new_time.tm_min;
    result.month    = new_time.tm_mon + 1;
    result.year     = 1900 + new_time.tm_year;
    result.sec      = new_time.tm_sec;
    result.weekday  = new_time.tm_wday;
    result.yearday  = new_time.tm_yday;
    result.ms       = ms;
    return result;
}