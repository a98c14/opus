#include "base_print.h"

internal void
printf_int32s(int32* arr, uint32 count)
{
    printf("[");
    for (uint32 i = 0; i < count; i++)
    {
        if (i < count - 1)
        {
            printf("%3d,", arr[i]);
        }
        else
        {
            printf("%3d", arr[i]);
        }
    }
    printf("]\n");
}