#pragma once
#include "../os_window.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

typedef struct
{
    int32              width;
    int32              height;
    GLFWwindow*        glfw_window;
    WindowKeyCallback* key_callback;
} _OS_GLFW_Window;

global read_only OS_KeyCode  _glfw_key_conversion_table[GLFW_KEY_LAST + 1]            = {0};
global read_only OS_KeyCode  _glfw_mouse_conversion_table[GLFW_MOUSE_BUTTON_LAST + 1] = {0};
global read_only uint32      _os_key_conversion_table[OS_KeyCode_COUNT]               = {0};
global read_only OS_KeyState _glfw_action_conversion_table[2]                         = {
    OS_KeyState_Released,
    OS_KeyState_Pressed,
};

typedef void _os_window_glfw_key_callback(GLFWwindow* glfw_window, int key, int scancode, int action, int mods);

internal void _os_window_glfw_error_callback(int error, const char* description);
internal void _os_window_glfw_key_callback_wrapper(GLFWwindow* glfw_window, int key, int scancode, int action, int mods);
internal void _os_window_glfw_mouse_callback_wrapper(GLFWwindow* glfw_window, int button, int action, int mods);
internal void _os_set_key_conversion(uint32 glfw_keycode, OS_KeyCode os_keycode);