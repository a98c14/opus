#include "window.h"

internal void
window_error_callback(int error, const char* description)
{
    log_error(description);
}

internal Window*
window_create(Arena* arena, uint32 width, uint32 height, char* name, WindowKeyCallback key_callback)
{
    Window* window = arena_push_struct_zero(arena, Window);

    glfwSetErrorCallback(window_error_callback);
    bool32 init_successfull = glfwInit();
    xassert(init_successfull, "could not initialize GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, 0);

    window->glfw_window = glfwCreateWindow(width, height, name, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window->glfw_window, key_callback);
    glfwMakeContextCurrent(window->glfw_window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);
    renderer_enable_debug();
    log_debug("window created");
    return window;
}

internal void
window_update(Window* window)
{
    glfwSwapBuffers(window->glfw_window);
    glfwPollEvents();
}

internal void
window_destroy(Window* window)
{
    glfwDestroyWindow(window->glfw_window);
    glfwTerminate();
}

internal bool32
window_should_close(Window* window)
{
    return glfwWindowShouldClose(window->glfw_window);
}