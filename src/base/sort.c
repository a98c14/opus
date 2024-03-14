#include "sort.h"

internal void
sort_int32(int32* arr, uint32 count)
{
    for (uint32 i = 1; i < count; i++)
    {
        for (uint32 j = i; j > 0 && arr[j - 1] > arr[j]; j--)
        {
            int32 temp = arr[j - 1];
            arr[j - 1] = arr[j];
            arr[j]     = temp;
        }
    }
}

internal void
sort_float32(float32* arr, uint32 count)
{
    for (uint32 i = 1; i < count; i++)
    {
        for (uint32 j = i; j > 0 && arr[j - 1] > arr[j]; j--)
        {
            int32 temp = arr[j - 1];
            arr[j - 1] = arr[j];
            arr[j]     = temp;
        }
    }
}

internal void
sort_keys_f32(SortKeyF32Ptr* arr, uint32 count)
{
    for (uint32 i = 1; i < count; i++)
    {
        for (uint32 j = i; j > 0 && arr[j - 1].v > arr[j].v; j--)
        {
            SortKeyF32Ptr temp = arr[j - 1];
            arr[j - 1]         = arr[j];
            arr[j]             = temp;
        }
    }
}

internal int
qsort_compare_int32_ascending(const void* p, const void* q)
{
    int x = *(const int*)p;
    int y = *(const int*)q;
    return (x < y)   ? -1
           : (x > y) ? 1
                     : 0;
}

internal int
qsort_compare_int32_descending(const void* p, const void* q)
{
    int32 x = *(const int32*)p;
    int32 y = *(const int32*)q;
    return (x < y)   ? 1
           : (x > y) ? -1
                     : 0;
}

internal int
qsort_compare_float32_ascending(const void* p, const void* q)
{
    float32 x = *(const float32*)p;
    float32 y = *(const float32*)q;
    return (x < y)   ? -1
           : (x > y) ? 1
                     : 0;
}

internal int
qsort_compare_float32_descending(const void* p, const void* q)
{
    float32 x = *(const float32*)p;
    float32 y = *(const float32*)q;
    return (x < y)   ? 1
           : (x > y) ? -1
                     : 0;
}

internal int
qsort_compare_sort_keys_ascending(const void* p, const void* q)
{
    float32 x = (*(const SortKeyF32*)p).v;
    float32 y = (*(const SortKeyF32*)q).v;
    return (x < y)   ? -1
           : (x > y) ? 1
                     : 0;
}

internal int
qsort_compare_sort_keys_descending(const void* p, const void* q)
{
    float32 x = (*(const SortKeyF32*)p).v;
    float32 y = (*(const SortKeyF32*)q).v;
    return (x < y)   ? 1
           : (x > y) ? -1
                     : 0;
}