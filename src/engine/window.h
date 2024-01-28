#pragma once
#include <base/asserts.h>
#include <base/defines.h>
#include <base/log.h>
#include <base/memory.h>
#include <gfx/base.h>
#include <gfx/debug.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

typedef struct
{
    GLFWwindow* glfw_window;
} Window;

typedef void WindowKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

internal void window_error_callback(int error, const char* description);

internal Window*
window_create(Arena* arena, uint32 width, uint32 height, char* name, WindowKeyCallback key_callback);

internal void window_update(Window* window);

internal void window_destroy(Window* window);