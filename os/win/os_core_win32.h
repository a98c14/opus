#pragma once
// clang-format off
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <timeapi.h>
#include <tlhelp32.h>
#include <Shlobj.h>
#include <processthreadsapi.h>
// clang-format on

#include "../os_core.h"

#define W32_PARENT_THREAD_MASK 0x1
#define W32_CHILD_THREAD_MASK  0x2

typedef enum W32_EntityKind
{
    W32_EntityKind_Null,
    W32_EntityKind_Thread,
    W32_EntityKind_Mutex,
    W32_EntityKind_RWMutex,
    W32_EntityKind_ConditionVariable,
} W32_EntityKind;

typedef struct W32_Entity W32_Entity;
struct W32_Entity
{
    W32_Entity*     next;
    W32_EntityKind  kind;
    volatile uint32 reference_mask;
    union
    {
        struct
        {
            DWORD                  id;
            OS_ThreadFunctionType* func;
            void*                  data;
            void*                  params;
        } thread;
        CRITICAL_SECTION   mutex;
        SRWLOCK            rw_mutex;
        CONDITION_VARIABLE cv;
    };
};

internal DWORD       w32_thread_base(void* ptr);
internal W32_Entity* w32_alloc_entity(W32_EntityKind kind);
internal void        w32_free_entity(W32_Entity* entity);

/** helpers */
internal uint32 w32_sleep_ms_from_endt_us(uint64 endt_us);
