#include "os_core_win32.h"

/** Globals */
global Arena*           w32_perm_arena             = 0;
global uint64           w32_microsecond_resolution = 0;
global W32_Entity*      w32_entity_free            = 0;
global CRITICAL_SECTION w32_mutex                  = {0};

/** Windows Functions */
typedef PVOID   W32_VirtualAlloc2_Type(HANDLE Process, PVOID BaseAddress,
                                       SIZE_T Size, ULONG AllocationType,
                                       ULONG PageProtection,
                                       void* ExtendedParameters,
                                       ULONG ParameterCount);
typedef PVOID   W32_MapViewOfFile3_Type(HANDLE FileMapping, HANDLE Process,
                                        PVOID BaseAddress, ULONG64 Offset,
                                        SIZE_T ViewSize, ULONG AllocationType,
                                        ULONG PageProtection,
                                        void* ExtendedParameters,
                                        ULONG ParameterCount);
typedef HRESULT W32_SetThreadDescription_Type(HANDLE hThread,
                                              PCWSTR lpThreadDescription);

global W32_VirtualAlloc2_Type*        w32_virtual_alloc_func          = 0;
global W32_MapViewOfFile3_Type*       w32_map_view_of_file_func       = 0;
global W32_SetThreadDescription_Type* w32_set_thread_description_func = 0;

/** implementations */
internal void
os_init(void)
{
    // map special functions
    {
        HMODULE module = LoadLibraryA("kernel32.dll");
        if (module != 0)
        {
            w32_virtual_alloc_func          = (W32_VirtualAlloc2_Type*)GetProcAddress(module, "VirtualAlloc2");
            w32_map_view_of_file_func       = (W32_MapViewOfFile3_Type*)GetProcAddress(module, "MapViewOfFile3");
            w32_set_thread_description_func = (W32_SetThreadDescription_Type*)GetProcAddress(module, "SetThreadDescription");
            FreeLibrary(module);
        }
    }

    w32_perm_arena = arena_new_reserve(mb(4));

    LARGE_INTEGER large_int_resolution;
    if (QueryPerformanceFrequency(&large_int_resolution))
    {
        w32_microsecond_resolution = large_int_resolution.QuadPart;
    }
    else
    {
        w32_microsecond_resolution = 1;
    }
}

internal OS_Handle
os_thread_launch(OS_ThreadFunctionType* func, void* data, void* params)
{
    W32_Entity* entity     = w32_alloc_entity(W32_EntityKind_Thread);
    entity->thread.func    = func;
    entity->thread.data    = data;
    entity->thread.params  = params;
    entity->reference_mask = W32_PARENT_THREAD_MASK | W32_CHILD_THREAD_MASK;
    CreateThread(0, 0, w32_thread_base, data, 0, &entity->thread.id);
    OS_Handle handle = {int_from_ptr(entity)};

    return handle;
}

internal bool32
os_thread_wait(OS_Handle handle, uint64 time_us)
{
    (void)handle;
    (void)time_us;
    not_implemented();
}

internal void
os_thread_release(OS_Handle handle)
{
    W32_Entity* entity = (W32_Entity*)ptr_from_int(handle.v);
    LONG        result = InterlockedAnd((LONG*)&entity->reference_mask, ~W32_PARENT_THREAD_MASK);
    if ((result & W32_CHILD_THREAD_MASK) == 0)
    {
        w32_free_entity(entity);
    }
}

internal void
os_thread_name_set(String name)
{
    if (w32_set_thread_description_func)
    {
        w32_set_thread_description_func(GetCurrentThread(), (WCHAR*)name.value);
    }
}

/** mutexes */
internal OS_Handle
os_mutex_alloc(void)
{
    W32_Entity* entity = w32_alloc_entity(W32_EntityKind_Mutex);
    InitializeCriticalSection(&entity->mutex);

    OS_Handle result = {int_from_ptr(entity)};
    return (result);
}

internal void
os_mutex_release(OS_Handle mutex)
{
    W32_Entity* entity = (W32_Entity*)ptr_from_int(mutex.v);
    w32_free_entity(entity);
}

internal void
os_mutex_take(OS_Handle mutex)
{
    W32_Entity* entity = (W32_Entity*)ptr_from_int(mutex.v);
    EnterCriticalSection(&entity->mutex);
}

internal void
os_mutex_drop(OS_Handle mutex)
{
    W32_Entity* entity = (W32_Entity*)ptr_from_int(mutex.v);
    LeaveCriticalSection(&entity->mutex);
}

internal OS_Handle
os_rw_mutex_alloc(void)
{
    W32_Entity* entity = w32_alloc_entity(W32_EntityKind_RWMutex);
    InitializeSRWLock(&entity->rw_mutex);

    OS_Handle result = {int_from_ptr(entity)};
    return (result);
}

internal void
os_rw_mutex_release(OS_Handle rw_mutex)
{
    W32_Entity* entity = (W32_Entity*)ptr_from_int(rw_mutex.v);
    w32_free_entity(entity);
}

internal void
os_rw_mutex_take_r(OS_Handle rw_mutex)
{
    W32_Entity* entity = (W32_Entity*)ptr_from_int(rw_mutex.v);
    AcquireSRWLockShared(&entity->rw_mutex);
}

internal void
os_rw_mutex_drop_r(OS_Handle rw_mutex)
{
    W32_Entity* entity = (W32_Entity*)ptr_from_int(rw_mutex.v);
    ReleaseSRWLockShared(&entity->rw_mutex);
}

internal void
os_rw_mutex_take_w(OS_Handle rw_mutex)
{
    W32_Entity* entity = (W32_Entity*)ptr_from_int(rw_mutex.v);
    AcquireSRWLockExclusive(&entity->rw_mutex);
}

internal void
os_rw_mutex_drop_w(OS_Handle rw_mutex)
{
    W32_Entity* entity = (W32_Entity*)ptr_from_int(rw_mutex.v);
    ReleaseSRWLockExclusive(&entity->rw_mutex);
}

internal OS_Handle
os_condition_variable_alloc(void)
{
    W32_Entity* entity = w32_alloc_entity(W32_EntityKind_ConditionVariable);
    InitializeConditionVariable(&entity->cv);
    OS_Handle result = {int_from_ptr(entity)};
    return (result);
}

internal void
os_condition_variable_release(OS_Handle cv)
{
    W32_Entity* entity = (W32_Entity*)ptr_from_int(cv.v);
    w32_free_entity(entity);
}

internal bool32
os_condition_variable_wait(OS_Handle cv, OS_Handle mutex, uint64 endt_us)
{
    uint32 sleep_ms = w32_sleep_ms_from_endt_us(endt_us);
    BOOL   result   = 0;
    if (sleep_ms > 0)
    {
        W32_Entity* entity       = (W32_Entity*)ptr_from_int(cv.v);
        W32_Entity* mutex_entity = (W32_Entity*)ptr_from_int(mutex.v);
        result                   = SleepConditionVariableCS(&entity->cv, &mutex_entity->mutex, sleep_ms);
    }
    return (result);
}

internal bool32
os_condition_variable_wait_rw_r(OS_Handle cv, OS_Handle mutex_rw, uint64 endt_us)
{
    uint32 sleep_ms = w32_sleep_ms_from_endt_us(endt_us);
    BOOL   result   = 0;
    if (sleep_ms > 0)
    {
        W32_Entity* entity       = (W32_Entity*)ptr_from_int(cv.v);
        W32_Entity* mutex_entity = (W32_Entity*)ptr_from_int(mutex_rw.v);
        result                   = SleepConditionVariableSRW(&entity->cv, &mutex_entity->rw_mutex, sleep_ms, CONDITION_VARIABLE_LOCKMODE_SHARED);
    }
    return result;
}

internal bool32
os_condition_variable_wait_rw_w(OS_Handle cv, OS_Handle mutex_rw, uint64 endt_us)
{
    uint32 sleep_ms = w32_sleep_ms_from_endt_us(endt_us);
    BOOL   result   = 0;
    if (sleep_ms > 0)
    {
        W32_Entity* entity       = (W32_Entity*)ptr_from_int(cv.v);
        W32_Entity* mutex_entity = (W32_Entity*)ptr_from_int(mutex_rw.v);
        result                   = SleepConditionVariableSRW(&entity->cv, &mutex_entity->rw_mutex, sleep_ms, 0);
    }
    return result;
}

internal void
os_condition_variable_signal(OS_Handle cv)
{
    W32_Entity* entity = (W32_Entity*)ptr_from_int(cv.v);
    WakeConditionVariable(&entity->cv);
}

internal void
os_condition_variable_broadcast(OS_Handle cv)
{
    W32_Entity* entity = (W32_Entity*)ptr_from_int(cv.v);
    WakeAllConditionVariable(&entity->cv);
}

/** timers */
internal uint64
os_now_ms(void)
{
    uint64        result;
    LARGE_INTEGER counter;
    if (QueryPerformanceCounter(&counter))
    {
        result = counter.QuadPart * thousand(1) / w32_microsecond_resolution;
    }
    return result;
}

internal uint64
os_now_us(void)
{
    uint64        result;
    LARGE_INTEGER counter;
    if (QueryPerformanceCounter(&counter))
    {
        result = counter.QuadPart * million(1) / w32_microsecond_resolution;
    }
    return result;
}

internal uint64
os_now_ns(void)
{
    uint64        result;
    LARGE_INTEGER counter;
    if (QueryPerformanceCounter(&counter))
    {
        result = counter.QuadPart * billion(1) / w32_microsecond_resolution;
    }
    return result;
}

/** w32 unique */
internal DWORD
w32_thread_base(void* ptr)
{
    W32_Entity* entity = (W32_Entity*)ptr;

    ThreadContext tctx_;
    tctx_init_and_equip(&tctx_);
    entity->thread.func(entity->thread.data);
    tctx_release();

    LONG result = InterlockedAnd((LONG*)&entity->reference_mask, ~W32_CHILD_THREAD_MASK);
    if ((result & W32_PARENT_THREAD_MASK) == 0)
    {
        w32_free_entity(entity);
    }
    return 0;
}

internal W32_Entity*
w32_alloc_entity(W32_EntityKind kind)
{
    EnterCriticalSection(&w32_mutex);
    W32_Entity* result = w32_entity_free;
    if (result != 0)
    {
        stack_pop(w32_entity_free);
    }
    else
    {
        result = arena_push_struct(w32_perm_arena, W32_Entity);
    }
    memory_zero_struct(result);
    xstatic_assert(result != 0);
    LeaveCriticalSection(&w32_mutex);
    memory_zero_struct(result);
    result->kind = kind;
    return result;
}

internal void
w32_free_entity(W32_Entity* entity)
{
    entity->kind = W32_EntityKind_Null;
    EnterCriticalSection(&w32_mutex);
    stack_push(w32_entity_free, entity);
    LeaveCriticalSection(&w32_mutex);
}

internal uint32
w32_sleep_ms_from_endt_us(uint64 endt_us)
{
    uint32 sleep_ms = 0;
    if (endt_us == MAX_UINT64)
    {
        sleep_ms = INFINITE;
    }
    else
    {
        uint64 begint_us = os_now_us();
        if (begint_us < endt_us)
        {
            sleep_ms = us_to_ms(endt_us - begint_us);
        }
    }
    return (sleep_ms);
}