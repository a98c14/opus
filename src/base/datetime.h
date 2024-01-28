#pragma once
#include <time.h>
#include "defines.h"
#include "asserts.h"

typedef struct
{
    int32 sec;       // seconds after the minute - [0, 60] including leap second
    int32 min;       // minutes after the hour - [0, 59]
    int32 hour;      // hours since midnight - [0, 23]
    int32 monthday;  // day of the month - [1, 31]
    int32 month;     // months since January - [0, 11]
    int32 year;      // years since 1900
    int32 weekday;   // days since Sunday - [0, 6]
    int32 yearday;   // days since January 1 - [0, 365]
    int32 isdst;     // daylight savings time flag
} DateTime;

internal DateTime date_now();