#include "base_datetime.h"

internal DateTime
date_now()
{
    DateTime  result = {0};
    struct tm new_time;
    int64     long_time;
    _time64(&long_time);
    errno_t err = localtime_s(&new_time, &long_time);
    xstatic_assert(err == 0);
    result.hour     = new_time.tm_hour;
    result.isdst    = new_time.tm_isdst;
    result.monthday = new_time.tm_mday;
    result.min      = new_time.tm_min;
    result.month    = new_time.tm_mon;
    result.year     = new_time.tm_year;
    result.sec      = new_time.tm_sec;
    result.weekday  = new_time.tm_wday;
    result.yearday  = new_time.tm_yday;
    return result;
}