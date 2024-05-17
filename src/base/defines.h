#pragma once
#include <stdint.h>

/* Defines */
#define global        static
#define local_persist static
#define internal      static
#define true          1
#define false         0

#if COMPILER_MSVC
#define per_thread __declspec(thread)
#elif COMPILER_CLANG
#define per_thread __thread
#elif COMPILER_GCC
#define per_thread __thread
#else
#define per_thread
#endif

#if COMPILER_CL || (COMPILER_CLANG && OS_WINDOWS)
#pragma section(".rdata$", read)
#define read_only __declspec(allocate(".rdata$"))
#elif (COMPILER_CLANG && OS_LINUX)
#define read_only __attribute__((section(".rodata")))
#else
#define read_only
#endif

/* Macros */
#define array_count(a)                           (sizeof(a) / sizeof((a)[0]))
#define array_count_from_type(type, array_field) array_count(field_pointer(type, array_field))
#define alignof(x)                               (usize) _Alignof(x)
#define countof(a)                               (sizeof(a) / sizeof(*(a)))
#define lengthof(s)                              (countof(s) - 1)
#define align_pow2(number, alignment)            ((number + alignment - 1) & ~(alignment - 1))

#define defer_loop(begin, end)         for (int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))
#define defer_loop_checked(begin, end) for (int _i_ = 2 * !(begin); (_i_ == 2 ? ((end), 0) : !_i_); _i_ += 1, (end))

#define memory_set(dst, byte, size) memset((dst), (byte), (size))
#define memory_compare(a, b, size)  memcmp((a), (b), (size))

#define memory_zero(s, z)       memset((s), 0, (z))
#define memory_zero_struct(s)   memory_zero((s), sizeof(*(s)))
#define memory_zero_array(a)    memory_zero((a), sizeof(a))
#define memory_zero_typed(m, c) memory_zero((m), sizeof(*(m)) * (c))

#define memory_copy(dst, src, size) memmove((dst), (src), (size))
#define memory_copy_struct(d, s)    memory_copy((d), (s), sizeof(*(d)))
#define memory_copy_array(d, s)     memory_copy((d), (s), sizeof(d))
#define memory_copy_typed(d, s, c)  memory_copy((d), (s), sizeof(*(d)) * (c))

#if COMPILER_MSVC && COMPILER_MSVC_YEAR < 2015
#define this_function_name "unknown"
#else
#define this_function_name __func__
#endif

#define stringify(value) #value
#define statement(statement) \
    do                       \
    {                        \
        statement            \
    } while (0)

#define kb(x) ((x) << 10)
#define mb(x) ((x) << 20)
#define gb(x) ((x) << 30)
#define tb(x) ((x) << 40)

#define thousand(n) ((n)*1000)
#define million(n)  ((n)*1000000)
#define billion(n)  ((n)*1000000000LL)

#define to_kb(x) ((x) >> 10)
#define to_mb(x) ((x) >> 20)
#define to_gb(x) ((x) >> 30)
#define to_tb(x) ((x) >> 40)

/* Primitive Types */
typedef uint8_t   uint8;
typedef uint16_t  uint16;
typedef uint32_t  uint32;
typedef uint64_t  uint64;
typedef int8_t    int8;
typedef int16_t   int16;
typedef int32_t   int32;
typedef int64_t   int64;
typedef int32_t   bool32;
typedef int16_t   bool16;
typedef int8_t    bool8;
typedef float     float32;
typedef double    float64;
typedef uintptr_t uptr;
typedef size_t    usize;

/* List Helpers */
#define check_null(p) ((p) == 0)
#define set_null(p)   ((p) = 0)

#define queue_push_nz(f, l, n, next, zchk, zset)       (zchk(f) ? (((f) = (l) = (n)), zset((n)->next)) : ((l)->next = (n), (l) = (n), zset((n)->next)))
#define queue_push_front_nz(f, l, n, next, zchk, zset) (zchk(f) ? (((f) = (l) = (n)), zset((n)->next)) : ((n)->next = (f)), ((f) = (n)))
#define queue_pop_nz(f, l, next, zset)                 ((f) == (l) ? (zset(f), zset(l)) : ((f) = (f)->next))
#define stack_push_n(f, n, next)                       ((n)->next = (f), (f) = (n))
#define stack_pop_nz(f, next, zchk)                    (zchk(f) ? 0 : ((f) = (f)->next))
#define stack_pop_n(f, next)                           ((f) = (f)->next)

#define dll_insert_npz(f, l, p, n, next, prev, zchk, zset)                                                                                                          \
    (zchk(f) ? (((f) = (l) = (n)), zset((n)->next), zset((n)->prev)) : zchk(p) ? (zset((n)->prev), (n)->next = (f), (zchk(f) ? (0) : ((f)->prev = (n))), (f) = (n)) \
                                                                               : ((zchk((p)->next) ? (0) : (((p)->next->prev) = (n))), (n)->next = (p)->next, (n)->prev = (p), (p)->next = (n), ((p) == (l) ? (l) = (n) : (0))))
#define dll_push_back_npz(f, l, n, next, prev, zchk, zset) dll_insert_npz(f, l, l, n, next, prev, zchk, zset)
#define dll_remove_npz(f, l, n, next, prev, zchk, zset)    (((f) == (n)) ? ((f) = (f)->next, (zchk(f) ? (zset(l)) : zset((f)->prev))) : ((l) == (n)) ? ((l) = (l)->prev, (zchk(l) ? (zset(f)) : zset((l)->next))) \
                                                                                                                                                     : ((zchk((n)->next) ? (0) : ((n)->next->prev = (n)->prev)), (zchk((n)->prev) ? (0) : ((n)->prev->next = (n)->next))))

#define queue_push(f, l, n)       queue_push_nz(f, l, n, next, check_null, set_null)
#define queue_push_front(f, l, n) queue_push_front_nz(f, l, n, next, check_null, set_null)
#define queue_pop(f, l)           queue_pop_nz(f, l, next, set_null)
#define stack_push(f, n)          stack_push_n(f, n, next)
#define stack_pop(f)              stack_pop_nz(f, next, check_null)
#define dll_push_back(f, l, n)    dll_push_back_npz(f, l, n, next, prev, check_null, set_null)
#define dll_push_front(f, l, n)   dll_push_back_npz(l, f, n, prev, next, check_null, set_null)
#define dll_insert(f, l, p, n)    dll_insert_npz(f, l, p, n, next, prev, check_null, set_null)
#define dll_remove(f, l, n)       dll_remove_npz(f, l, n, next, prev, check_null, set_null)

#define for_each(n, f) for ((n = f); (n != 0); (n = n->next))

/** branch predictor hints */
#if defined(__clang__)
#define expect(expr, val) __builtin_expect((expr), (val))
#else
#define expect(expr, val) (expr)
#endif

#define likely(expr)   expect(expr, 1)
#define unlikely(expr) expect(expr, 0)