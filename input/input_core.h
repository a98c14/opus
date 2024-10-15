#pragma once
#include "../base/base_inc.h"
#include "../os/os_inc.h"
#include "../gfx/gfx_inc.h"

#define _INPUT_ACTION_MAP_TABLE_SIZE  1024
#define _INPUT_MAX_KEY_REGISTER_COUNT 256
#define _INPUT_EVENT_BUFFER_SIZE      128

typedef uint16 Input_KeyState;
enum
{
    Input_KeyStateNone        = 0,
    Input_KeyStateReleased    = 1 << 0,
    Input_KeyStatePressed     = 1 << 1,
    Input_KeyStateNew         = 1 << 2, // set if the state switched this frame
    Input_KeyStatePressedNew  = Input_KeyStatePressed | Input_KeyStateNew,
    Input_KeyStateReleasedNew = Input_KeyStateReleased | Input_KeyStateNew,
};

typedef struct
{
    OS_KeyCode  key_code;
    OS_KeyState state;
} Input_Event;

typedef struct
{
    String         name;
    bool32         is_initialized;
    Input_KeyState state;

    float64 t_press;
    float64 t_release;
} Input_Key;

typedef struct Input_KeyNode Input_KeyNode;

struct Input_KeyNode
{
    Input_KeyNode* next;

    uint64     hash;
    OS_KeyCode key_code;
};

typedef struct
{
    uint32 count;

    Input_KeyNode* first;
    Input_KeyNode* last;
} Input_KeyBucket;

typedef struct
{
    Vec2 raw;
    Vec2 screen;
    Vec2 world;
} Input_MouseInfo;

typedef struct
{
    Arena* persistent_arena;

    float64 t_frame;

    uint64      input_event_buffer_end;
    uint64      input_event_buffer_start;
    Input_Event input_events[_INPUT_EVENT_BUFFER_SIZE];

    Input_MouseInfo cached_mouse_info;

    uint32          key_count;
    Input_KeyBucket key_index_map[_INPUT_ACTION_MAP_TABLE_SIZE];
    Input_Key       keys[OS_KeyCode_COUNT];
    OS_KeyCode      registered_key_codes[_INPUT_MAX_KEY_REGISTER_COUNT];
} Input_Context;

global Input_Context* _input_context;

internal void input_key_callback(OS_KeyCode key_code, OS_KeyState state);
internal void input_init(Arena* arena);
// call this at the start of your game loop
internal void input_update(float32 dt);
internal void input_register_key(String action_name, OS_KeyCode code);

internal Input_MouseInfo input_mouse_info(void);
internal bool32          input_is_pressed(String action_name);
internal bool32          input_is_held(String action_name);

/** Private Functions */
internal Input_Key* _input_key_from_action(String action_name);
