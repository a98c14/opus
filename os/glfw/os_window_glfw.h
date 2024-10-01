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

typedef void _os_window_glfw_key_callback(GLFWwindow* glfw_window, int key, int scancode, int action, int mods);

internal void _os_window_glfw_error_callback(int error, const char* description);
internal void _os_window_glfw_key_callback_wrapper(GLFWwindow* glfw_window, int key, int scancode, int action, int mods);