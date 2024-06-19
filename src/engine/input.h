#pragma once

#include <base/defines.h>
#include <base/math.h>
#include <engine/time.h>
#include <engine/window.h>
#include <gfx/base.h>

#define INPUT_MANAGER_MAX_ACTION_ID 256

typedef enum
{
    MouseButtonStateLeft   = 1 << 0,
    MouseButtonStateRight  = 1 << 1,
    MouseButtonStateMiddle = 1 << 2,
} MouseButtonState;

typedef uint16 InputKeyState;
enum
{
    InputKeyStateNone       = 0,
    InputKeyStateReleased   = 1 << 0,
    InputKeyStatePressed    = 1 << 1,
    InputKeyStateNew        = 1 << 2, // set if the state switched this frame
    InputKeyStatePressedNew = InputKeyStatePressed | InputKeyStateNew,
};

typedef struct
{
    Vec2    world;
    Vec2    screen;
    float64 raw_x;
    float64 raw_y;
    uint8   prev_button_state;
    uint8   button_state;
} InputMouse;

typedef struct
{
    InputKeyState key_state;
    uint16        key;
    float32       t_press;
    float32       t_release;
} InputState;

typedef struct
{
    Window*    window;
    uint32     key_count;
    InputState key_states[INPUT_MANAGER_MAX_ACTION_ID];
    String     key_names[INPUT_MANAGER_MAX_ACTION_ID];
} InputManager;
global InputManager* g_input_manager;

/** raw */
internal bool32 input_key_pressed_raw(Window* window, uint16 key);
internal bool32 input_mouse_pressed_raw(Window* window, uint16 key);

/** `action_id` should be a value of an enum, there shouldn't be any gaps. */
internal void   input_manager_init(Arena* arena, Window* window);
internal void   input_manager_register_action(String action_name, uint64 action_id, uint16 key);
internal void   input_manager_update(EngineTime time);
internal bool32 input_action_pressed(uint64 action_id);

/** mouse */
internal Vec2       input_mouse_world_position(Vec2 raw_mouse_pos, Camera camera);
internal InputMouse input_mouse_get(Window* window, Camera camera, InputMouse prev_state);
internal bool32     input_mouse_pressed(InputMouse mouse, MouseButtonState state);
internal bool32     input_mouse_held(InputMouse mouse, MouseButtonState state);
internal bool32     input_mouse_released(InputMouse mouse, MouseButtonState state);