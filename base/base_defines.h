#pragma once
#include <stdint.h>
#include "base_architecture.h"

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

#if COMPILER_CL || (COMPILER_CLANG && OS_WINDOWS)
#define bit_field __attribute__((packed))
#endif

/* Macros */
#define array_count(a)                           (sizeof(a) / sizeof((a)[0]))
#define array_count_from_type(type, array_field) array_count(field_pointer(type, array_field))
#define alignof(x)                               (usize) _Alignof(x)
#define countof(a)                               (sizeof(a) / sizeof(*(a)))
#define lengthof(s)                              (countof(s) - 1)
#define align_pow2(number, alignment)            ((number + (alignment) - 1) & ~((alignment) - 1))
#define align_down_pow2(number, alignment)       (((number) & ~(alignment - 1)))

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

#define thousand(n) ((n) * 1000)
#define million(n)  ((n) * 1000000)
#define billion(n)  ((n) * 1000000000LL)

#define to_kb(x) ((x) >> 10)
#define to_mb(x) ((x) >> 20)
#define to_gb(x) ((x) >> 30)
#define to_tb(x) ((x) >> 40)

/** Asserts */
#if _MSC_VER
#include <intrin.h>
#define assert_break() __debugbreak()
#else
#define assert_break() __builtin_trap()
#endif

#if BUILD_DEBUG
#define xstatic_assert(condition) statement(if (!(condition)) { assert_break(); })
#else
#define xstatic_assert(condition)
#endif

/** Address Sanitizer */
#if COMPILER_MSVC
#if defined(__SANITIZE_ADDRESS__)
#define ASAN_ENABLED 1
#define NO_ASAN      __declspec(no_sanitize_address)
#else
#define NO_ASAN
#endif
#elif COMPILER_CLANG
#if defined(__has_feature)
#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#define ASAN_ENABLED 1
#endif
#endif
#define NO_ASAN __attribute__((no_sanitize("address")))
#else
#define NO_ASAN
#endif

#if ASAN_ENABLED
// #pragma comment(lib, "clang_rt.asan-x86_64.lib")
#pragma comment(lib, "clang_rt.asan_dynamic-x86_64.lib")
void __asan_poison_memory_region(void const volatile* addr, size_t size);
void __asan_unpoison_memory_region(void const volatile* addr, size_t size);
#define asan_poison_memory_region(addr, size)   __asan_poison_memory_region((addr), (size))
#define asan_unpoison_memory_region(addr, size) __asan_unpoison_memory_region((addr), (size))
#else
#define asan_poison_memory_region(addr, size)   ((void)(addr), (void)(size))
#define asan_unpoison_memory_region(addr, size) ((void)(addr), (void)(size))
#endif

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

#define int_from_ptr(ptr) ((uint64)(ptr))
#define ptr_from_int(i)   (void*)((uint8*)0 + (i))

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

/** safe cast */
internal uint16 safe_cast_uint16(uint32 x);
internal uint32 safe_cast_uint32(uint64 x);
internal int32  safe_cast_int32(int64 x);

/** flags */
#define flag_set(n, f)        ((n) |= (f))
#define flag_sete(n, f)       ((n) |= (1 << (f))) // enum version
#define flag_clear(n, f)      ((n) &= ~(f))
#define flag_toggle(n, f)     ((n) ^= (f))
#define flag_is_set(n, f)     (((n) & (f)) == (f)) // Checks if all bits in 'f' are set in 'n'. Returns true if f == 0
#define flag_not_set(n, f)    (((n) & (f)) == 0)   // Checks if none of the bits in 'f' are set in 'n'.
#define flag_equals(n, f)     (((n) == (f)))       // Checks if 'n' is exactly equal to 'f'.
#define flag_intersects(n, f) (((n) & (f)) > 0)    // Checks if any bits in 'f' are set in 'n'.

/** Time */
#define ns_to_us(x)   ((uint32)((x + (thousand(1) - 1)) / thousand(1)))
#define ns_to_ms(x)   ((uint32)((x + (million(1) - 1)) / million(1)))
#define us_to_ms(x)   ((uint32)((x + (thousand(1) - 1)) / thousand(1)))
#define ns_to_us_f(x) ((float32)((x) / (float64)thousand(1)))
#define ns_to_ms_f(x) ((float32)((x) / (float64)million(1)))
#define us_to_ms_f(x) ((float32)((x) / (float64)thousand(1)))

/** Atomic */
#if OS_WINDOWS
#include <intrin.h>
#include <tmmintrin.h>
#include <windows.h>
#include <wmmintrin.h>
#if ARCH_X64
#define interlocked_eval_uint64(x)              InterlockedAdd64((volatile __int64*)(x), 0)
#define interlocked_inc_uint64(x)               InterlockedIncrement64((volatile __int64*)(x))
#define interlocked_dec_uint64(x)               InterlockedDecrement64((volatile __int64*)(x))
#define interlocked_assign_uint64(x, c)         InterlockedExchange64((volatile __int64*)(x), (c))
#define interlocked_add_uint64(x, c)            InterlockedAdd64((volatile __int64*)(x), c)
#define interlocked_cond_assign_uint64(x, k, c) InterlockedCompareExchange64((volatile __int64*)(x), (k), (c))
#define interlocked_eval_uint32(x, c)           InterlockedAdd((volatile LONG*)(x), 0)
#define interlocked_assign_uint32(x, c)         InterlockedExchange((volatile LONG*)(x), (c))
#define interlocked_cond_assign_uint32(x, k, c) InterlockedCompareExchange((volatile LONG*)(x), (k), (c))
#define interlocked_ptr_assign(x, c)            (void*)ins_atomic_uint64_eval_assign((volatile __int64*)(x), (__int64)(c))
#else
#error Atomic intrinsics not defined for this operating system / architecture combination.
#endif
#elif OS_LINUX
#if ARCH_X64
#define interlocked_inc_uint64(x) __sync_fetch_and_add((volatile uint64*)(x), 1)
#else
#error Atomic intrinsics not defined for this operating system / architecture combination.
#endif
#else
#error Atomic intrinsics not defined for this operating system.
#endif

/* Constants */
global float32 EPSILON_FLOAT32 = 1.1920929e-7f;
global float32 PI_FLOAT32      = 3.14159265359f;
global float32 TAU_FLOAT32     = 6.28318530718f;

global float32 ANGLE_TO_RAD_FLOAT32 = 3.14159265359f / 180.0f;
global float32 RAD_TO_ANGLE_FLOAT32 = 180.0f / 3.14159265359f;

// TODO: Find the actual min/max float values
global float32 FLOAT32_MIN = -1000000;
global float32 FLOAT32_MAX = 1000000;

global uint64 MAX_UINT64 = 0xffffffffffffffffull;
global uint32 MAX_UINT32 = 0xffffffff;
global uint16 MAX_UINT16 = 0xffff;
global uint8  MAX_UINT8  = 0xff;

global int64 MAX_INT64 = (int64)0x7fffffffffffffffull;
global int32 MAX_INT32 = (int32)0x7fffffff;
global int16 MAX_INT16 = (int16)0x7fff;
global int8  MAX_INT8  = (int8)0x7f;

global int64 MIN_INT64 = (int64)0xffffffffffffffffull;
global int32 MIN_INT32 = (int32)0xffffffff;
global int16 MIN_INT16 = (int16)0xff;
global int8  MIN_INT8  = (int8)0xf;

/** Bit Operations */
internal uint32 saturate_uint32_from_uint64(uint64 value);

/** Bit Mask */
global const uint32 bitmask1  = 0x00000001;
global const uint32 bitmask2  = 0x00000003;
global const uint32 bitmask3  = 0x00000007;
global const uint32 bitmask4  = 0x0000000f;
global const uint32 bitmask5  = 0x0000001f;
global const uint32 bitmask6  = 0x0000003f;
global const uint32 bitmask7  = 0x0000007f;
global const uint32 bitmask8  = 0x000000ff;
global const uint32 bitmask9  = 0x000001ff;
global const uint32 bitmask10 = 0x000003ff;
global const uint32 bitmask11 = 0x000007ff;
global const uint32 bitmask12 = 0x00000fff;
global const uint32 bitmask13 = 0x00001fff;
global const uint32 bitmask14 = 0x00003fff;
global const uint32 bitmask15 = 0x00007fff;
global const uint32 bitmask16 = 0x0000ffff;
global const uint32 bitmask17 = 0x0001ffff;
global const uint32 bitmask18 = 0x0003ffff;
global const uint32 bitmask19 = 0x0007ffff;
global const uint32 bitmask20 = 0x000fffff;
global const uint32 bitmask21 = 0x001fffff;
global const uint32 bitmask22 = 0x003fffff;
global const uint32 bitmask23 = 0x007fffff;
global const uint32 bitmask24 = 0x00ffffff;
global const uint32 bitmask25 = 0x01ffffff;
global const uint32 bitmask26 = 0x03ffffff;
global const uint32 bitmask27 = 0x07ffffff;
global const uint32 bitmask28 = 0x0fffffff;
global const uint32 bitmask29 = 0x1fffffff;
global const uint32 bitmask30 = 0x3fffffff;
global const uint32 bitmask31 = 0x7fffffff;
global const uint32 bitmask32 = 0xffffffff;

global const uint64 bitmask33 = 0x00000001ffffffffull;
global const uint64 bitmask34 = 0x00000003ffffffffull;
global const uint64 bitmask35 = 0x00000007ffffffffull;
global const uint64 bitmask36 = 0x0000000fffffffffull;
global const uint64 bitmask37 = 0x0000001fffffffffull;
global const uint64 bitmask38 = 0x0000003fffffffffull;
global const uint64 bitmask39 = 0x0000007fffffffffull;
global const uint64 bitmask40 = 0x000000ffffffffffull;
global const uint64 bitmask41 = 0x000001ffffffffffull;
global const uint64 bitmask42 = 0x000003ffffffffffull;
global const uint64 bitmask43 = 0x000007ffffffffffull;
global const uint64 bitmask44 = 0x00000fffffffffffull;
global const uint64 bitmask45 = 0x00001fffffffffffull;
global const uint64 bitmask46 = 0x00003fffffffffffull;
global const uint64 bitmask47 = 0x00007fffffffffffull;
global const uint64 bitmask48 = 0x0000ffffffffffffull;
global const uint64 bitmask49 = 0x0001ffffffffffffull;
global const uint64 bitmask50 = 0x0003ffffffffffffull;
global const uint64 bitmask51 = 0x0007ffffffffffffull;
global const uint64 bitmask52 = 0x000fffffffffffffull;
global const uint64 bitmask53 = 0x001fffffffffffffull;
global const uint64 bitmask54 = 0x003fffffffffffffull;
global const uint64 bitmask55 = 0x007fffffffffffffull;
global const uint64 bitmask56 = 0x00ffffffffffffffull;
global const uint64 bitmask57 = 0x01ffffffffffffffull;
global const uint64 bitmask58 = 0x03ffffffffffffffull;
global const uint64 bitmask59 = 0x07ffffffffffffffull;
global const uint64 bitmask60 = 0x0fffffffffffffffull;
global const uint64 bitmask61 = 0x1fffffffffffffffull;
global const uint64 bitmask62 = 0x3fffffffffffffffull;
global const uint64 bitmask63 = 0x7fffffffffffffffull;
global const uint64 bitmask64 = 0xffffffffffffffffull;

global const uint32 bit1  = (1 << 0);
global const uint32 bit2  = (1 << 1);
global const uint32 bit3  = (1 << 2);
global const uint32 bit4  = (1 << 3);
global const uint32 bit5  = (1 << 4);
global const uint32 bit6  = (1 << 5);
global const uint32 bit7  = (1 << 6);
global const uint32 bit8  = (1 << 7);
global const uint32 bit9  = (1 << 8);
global const uint32 bit10 = (1 << 9);
global const uint32 bit11 = (1 << 10);
global const uint32 bit12 = (1 << 11);
global const uint32 bit13 = (1 << 12);
global const uint32 bit14 = (1 << 13);
global const uint32 bit15 = (1 << 14);
global const uint32 bit16 = (1 << 15);
global const uint32 bit17 = (1 << 16);
global const uint32 bit18 = (1 << 17);
global const uint32 bit19 = (1 << 18);
global const uint32 bit20 = (1 << 19);
global const uint32 bit21 = (1 << 20);
global const uint32 bit22 = (1 << 21);
global const uint32 bit23 = (1 << 22);
global const uint32 bit24 = (1 << 23);
global const uint32 bit25 = (1 << 24);
global const uint32 bit26 = (1 << 25);
global const uint32 bit27 = (1 << 26);
global const uint32 bit28 = (1 << 27);
global const uint32 bit29 = (1 << 28);
global const uint32 bit30 = (1 << 29);
global const uint32 bit31 = (1 << 30);
global const uint32 bit32 = (uint32)(1 << 31);

global const uint64 bit33 = (1ull << 32);
global const uint64 bit34 = (1ull << 33);
global const uint64 bit35 = (1ull << 34);
global const uint64 bit36 = (1ull << 35);
global const uint64 bit37 = (1ull << 36);
global const uint64 bit38 = (1ull << 37);
global const uint64 bit39 = (1ull << 38);
global const uint64 bit40 = (1ull << 39);
global const uint64 bit41 = (1ull << 40);
global const uint64 bit42 = (1ull << 41);
global const uint64 bit43 = (1ull << 42);
global const uint64 bit44 = (1ull << 43);
global const uint64 bit45 = (1ull << 44);
global const uint64 bit46 = (1ull << 45);
global const uint64 bit47 = (1ull << 46);
global const uint64 bit48 = (1ull << 47);
global const uint64 bit49 = (1ull << 48);
global const uint64 bit50 = (1ull << 49);
global const uint64 bit51 = (1ull << 50);
global const uint64 bit52 = (1ull << 51);
global const uint64 bit53 = (1ull << 52);
global const uint64 bit54 = (1ull << 53);
global const uint64 bit55 = (1ull << 54);
global const uint64 bit56 = (1ull << 55);
global const uint64 bit57 = (1ull << 56);
global const uint64 bit58 = (1ull << 57);
global const uint64 bit59 = (1ull << 58);
global const uint64 bit60 = (1ull << 59);
global const uint64 bit61 = (1ull << 60);
global const uint64 bit62 = (1ull << 61);
global const uint64 bit63 = (1ull << 62);
global const uint64 bit64 = (uint64)(1ull << 63);
