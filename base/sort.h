#pragma once

#include "defines.h"
#include "memory.h"
#include <stdlib.h>

typedef struct
{
    void*   ptr;
    float32 v;
} SortKeyF32Ptr;

typedef struct
{
    uint64  idx;
    float32 v;
} SortKeyF32;

internal void sort_int32(int32* arr, uint32 count);
internal void sort_float32(float32* arr, uint32 count);
internal void sort_keys_f32(SortKeyF32Ptr* arr, uint32 count);

/** common qsort compare functions
 * Example: qsort(arr, arr_count, sizeof(int32), qsort_compare_int32_descending);
 */
internal int qsort_compare_int32_ascending(const void* p, const void* q);
internal int qsort_compare_int32_descending(const void* p, const void* q);
internal int qsort_compare_float32_ascending(const void* p, const void* q);
internal int qsort_compare_float32_descending(const void* p, const void* q);
internal int qsort_compare_sort_keys_ascending(const void* p, const void* q);
internal int qsort_compare_sort_keys_descending(const void* p, const void* q);