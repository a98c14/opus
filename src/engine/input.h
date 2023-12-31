#pragma once

#include <core/defines.h>
#include <core/math.h>
#include <engine/window.h>
#include <gfx/base.h>

typedef enum
{
    MouseButtonStateLeft   = 1 << 0,
    MouseButtonStateRight  = 1 << 1,
    MouseButtonStateMiddle = 1 << 2,
} MouseButtonState;

typedef struct
{
    Vec2    world;
    Vec2    screen;
    float64 raw_x;
    float64 raw_y;
    uint8   prev_button_state;
    uint8   button_state;
} InputMouse;

internal Vec2       mouse_world_position(Vec2 raw_mouse_pos, Camera camera);
internal InputMouse input_mouse_get(Window* window, Camera camera, InputMouse prev_state);
internal bool32     input_mouse_pressed(InputMouse mouse, MouseButtonState state);
internal bool32     input_mouse_released(InputMouse mouse, MouseButtonState state);
internal bool32     input_key_pressed(Window* window, uint16 key);