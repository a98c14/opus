#include "input.h"

internal Vec2
mouse_world_position(Vec2 raw_mouse_pos, Camera camera)
{
    Vec2 mouse_world = vec2(-raw_mouse_pos.x / camera.window_width, raw_mouse_pos.y / camera.window_height);
    mouse_world      = sub_vec2(vec2(0, 1), mouse_world);
    mouse_world      = add_vec2_f32(mouse_world, -0.5);
    mouse_world.x *= camera.world_width;
    mouse_world.y *= camera.world_height;
    return mouse_world;
}

internal InputMouse
input_mouse_get(Window* window, Camera camera, InputMouse prev_state)
{
    InputMouse result = {0};
    glfwGetCursorPos(window->glfw_window, &result.raw_x, &result.raw_y);
    result.prev_button_state = prev_state.button_state;
    result.button_state += (glfwGetMouseButton(window->glfw_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) << 0;
    result.button_state += (glfwGetMouseButton(window->glfw_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) << 1;
    result.button_state += (glfwGetMouseButton(window->glfw_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) << 2;

    Vec2 mouse_raw = vec2(result.raw_x, result.raw_y);
    result.world   = mouse_world_position(mouse_raw, camera);
    // TODO: calculate mouse screen position
    result.screen = vec2_zero();
    return result;
}

internal bool32
input_mouse_pressed(InputMouse mouse, MouseButtonState state)
{
    return (mouse.button_state & state) > 0;
}

internal bool32
input_mouse_released(InputMouse mouse, MouseButtonState state)
{
    return !input_mouse_pressed(mouse, state);
}

internal bool32
input_key_pressed(Window* window, uint16 key)
{
    return glfwGetKey(window->glfw_window, key) == GLFW_PRESS;
}