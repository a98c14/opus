#include "./os_window_glfw.h"

/** Globals */
global Arena*          _os_glfw_perm_arena      = 0;
global const uint64    _os_glfw_perm_arena_size = mb(4);
global const OS_Handle _os_glfw_main_window     = {0};

internal OS_Handle
os_window_create(int32 width, int32 height, String name, WindowKeyCallback key_callback)
{
    if (_os_glfw_perm_arena == 0)
    {
        _os_glfw_perm_arena = arena_new_reserve(_os_glfw_perm_arena_size);
    }

    glfwSetErrorCallback(_os_window_glfw_error_callback);
    bool32 init_successfull = glfwInit();
    if (!init_successfull)
    {
        log_error("could not initialize GLFW");
        return os_handle_zero();
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, 0);
    GLFWwindow* glfw_window = glfwCreateWindow(width, height, name.value, NULL, NULL);
    if (!glfw_window)
    {
        log_error("could not create window");
        return os_handle_zero();
    }

    glfwSetKeyCallback(glfw_window, _os_window_glfw_key_callback_wrapper);
    glfwMakeContextCurrent(glfw_window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(0);
    log_debug("window created successfully");

    _OS_GLFW_Window* window = arena_push_struct_zero(_os_glfw_perm_arena, _OS_GLFW_Window);
    window->glfw_window     = glfw_window;
    window->key_callback    = key_callback;
    window->width           = width;
    window->height          = height;
    OS_Handle result        = {int_from_ptr(window)};
    return result;
}

internal void
os_window_update(OS_Handle window_handle)
{
    _OS_GLFW_Window* window = (_OS_GLFW_Window*)ptr_from_int(window_handle.v);
    glfwSwapBuffers(window->glfw_window);
    glfwPollEvents();
}

internal void
os_window_destroy(OS_Handle window_handle)
{
    _OS_GLFW_Window* window = (_OS_GLFW_Window*)ptr_from_int(window_handle.v);
    glfwDestroyWindow(window->glfw_window);
    glfwTerminate();
}

internal bool32
os_window_should_close(OS_Handle window_handle)
{
    _OS_GLFW_Window* window = (_OS_GLFW_Window*)ptr_from_int(window_handle.v);
    return glfwWindowShouldClose(window->glfw_window);
}

internal void
_os_window_glfw_error_callback(int error, const char* description)
{
    log_error("Window Error, Code: %d, Desc: %s ", error, description);
}

internal void
_os_window_glfw_key_callback_wrapper(GLFWwindow* glfw_window, int key, int scancode, int action, int mods)
{
    (void)glfw_window;
    _OS_GLFW_Window* window = (_OS_GLFW_Window*)ptr_from_int(_os_glfw_main_window.v);
    window->key_callback(_os_glfw_main_window, key, scancode, action, mods);
}
