#pragma once
#include "../base/base_inc.h"

typedef void OS_ThreadFunctionType(void* ptr);

typedef struct
{
    uint64 v;
} OS_Handle;

internal void      os_init(void);
internal OS_Handle os_thread_launch(OS_ThreadFunctionType* func, void* data, void* params);
internal bool32    os_thread_wait(OS_Handle thread_handle, uint64 time_us);
internal void      os_thread_name_set(String name);

internal bool32    os_handle_match(OS_Handle a, OS_Handle b);
internal OS_Handle os_handle_zero(void);
internal bool32    os_handle_is_zero(OS_Handle h);

/** mutexes */
internal OS_Handle os_mutex_alloc(void);
internal void      os_mutex_release(OS_Handle mutex);
internal void      os_mutex_take(OS_Handle mutex);
internal void      os_mutex_drop(OS_Handle mutex);
internal OS_Handle os_rw_mutex_alloc(void);
internal void      os_rw_mutex_release(OS_Handle rw_mutex);
internal void      os_rw_mutex_take_r(OS_Handle rw_mutex);
internal void      os_rw_mutex_drop_r(OS_Handle rw_mutex);
internal void      os_rw_mutex_take_w(OS_Handle rw_mutex);
internal void      os_rw_mutex_drop_w(OS_Handle rw_mutex);

// returns false on timeout, true on signal, (max_wait_ms = max_U64) -> no timeout
internal OS_Handle os_condition_variable_alloc(void);
internal void      os_condition_variable_release(OS_Handle cv);
internal bool32    os_condition_variable_wait(OS_Handle cv, OS_Handle mutex, uint64 endt_us);
internal bool32    os_condition_variable_wait_rw_r(OS_Handle cv, OS_Handle mutex_rw, uint64 endt_us);
internal bool32    os_condition_variable_wait_rw_w(OS_Handle cv, OS_Handle mutex_rw, uint64 endt_us);
internal void      os_condition_variable_signal(OS_Handle cv);
internal void      os_condition_variable_broadcast(OS_Handle cv);

#define os_mutex_scope(mutex)            defer_loop(os_mutex_take(mutex), os_mutex_drop(mutex))
#define os_mutex_scope_r(mutex)          defer_loop(os_rw_mutex_take_r(mutex), os_rw_mutex_drop_r(mutex))
#define os_mutex_scope_w(mutex)          defer_loop(os_rw_mutex_take_w(mutex), os_rw_mutex_drop_w(mutex))
#define os_mutex_scope_rw_promote(mutex) defer_loop((os_rw_mutex_drop_r(mutex), os_rw_mutex_take_w(mutex)), (os_rw_mutex_drop_w(mutex), os_rw_mutex_take_r(mutex)))

/** timers */
internal uint64 os_now_ms();
internal uint64 os_now_us();
internal uint64 os_now_ns();

/** utility */
internal IVec2 os_screen_resolution();

/** input */
typedef enum
{
    OS_KeyCode_Null = 0,
    // Mouse
    OS_KeyCode_MouseLeft,
    OS_KeyCode_MouseRight,
    OS_KeyCode_MouseMiddle,
    // Keys
    OS_KeyCode_Space,
    OS_KeyCode_BracketLeft,
    OS_KeyCode_BracketRight,
    OS_KeyCode_Backspace,
    OS_KeyCode_Tab,
    OS_KeyCode_Enter,
    OS_KeyCode_Escape,
    OS_KeyCode_Delete,

    // Arrow Keys
    OS_KeyCode_RightArrow,
    OS_KeyCode_LeftArrow,
    OS_KeyCode_DownArrow,
    OS_KeyCode_UpArrow,

    // Function Keys
    OS_KeyCode_F1,
    OS_KeyCode_F2,
    OS_KeyCode_F3,
    OS_KeyCode_F4,
    OS_KeyCode_F5,
    OS_KeyCode_F6,
    OS_KeyCode_F7,
    OS_KeyCode_F8,
    OS_KeyCode_F9,
    OS_KeyCode_F10,
    OS_KeyCode_F11,
    OS_KeyCode_F12,

    // Modifier Keys
    OS_KeyCode_LeftShift,
    OS_KeyCode_RightShift,
    OS_KeyCode_LeftControl,
    OS_KeyCode_RightControl,
    OS_KeyCode_LeftAlt,
    OS_KeyCode_RightAlt,

    // Numbers
    OS_KeyCode_0,
    OS_KeyCode_1,
    OS_KeyCode_2,
    OS_KeyCode_3,
    OS_KeyCode_4,
    OS_KeyCode_5,
    OS_KeyCode_6,
    OS_KeyCode_7,
    OS_KeyCode_8,
    OS_KeyCode_9,

    // Letters
    OS_KeyCode_A,
    OS_KeyCode_B,
    OS_KeyCode_C,
    OS_KeyCode_D,
    OS_KeyCode_E,
    OS_KeyCode_F,
    OS_KeyCode_G,
    OS_KeyCode_H,
    OS_KeyCode_I,
    OS_KeyCode_J,
    OS_KeyCode_K,
    OS_KeyCode_L,
    OS_KeyCode_M,
    OS_KeyCode_N,
    OS_KeyCode_O,
    OS_KeyCode_P,
    OS_KeyCode_Q,
    OS_KeyCode_R,
    OS_KeyCode_S,
    OS_KeyCode_T,
    OS_KeyCode_U,
    OS_KeyCode_V,
    OS_KeyCode_W,
    OS_KeyCode_X,
    OS_KeyCode_Y,
    OS_KeyCode_Z,

    OS_KeyCode_COUNT,
} OS_KeyCode;

typedef enum
{
    OS_KeyState_Null = 0,
    OS_KeyState_Pressed,
    OS_KeyState_Released,
} OS_KeyState;

// Returns window position. Bottom Left = (0,0), Top Left = (Window Width, Window Height);
internal Vec2        os_input_mouse_pos();
internal OS_KeyState os_input_key_state(OS_KeyCode code);