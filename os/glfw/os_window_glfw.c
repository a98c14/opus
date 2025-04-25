#include "./os_window_glfw.h"

/** Globals */
global Arena*       _os_glfw_perm_arena      = 0;
global OS_Handle    _os_glfw_main_window     = {0};
global const uint64 _os_glfw_perm_arena_size = mb(4);

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
        log_error("Could not initialize GLFW.");
        return os_handle_zero();
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, 0);
    GLFWwindow* glfw_window = glfwCreateWindow(width, height, name.value, NULL, NULL);
    if (!glfw_window)
    {
        log_error("Could not create window.");
        return os_handle_zero();
    }

    glfwSetKeyCallback(glfw_window, _os_window_glfw_key_callback_wrapper);
    glfwSetMouseButtonCallback(glfw_window, _os_window_glfw_mouse_callback_wrapper);
    glfwMakeContextCurrent(glfw_window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(0);
    log_debug("Window created successfully.");

    _OS_GLFW_Window* window = arena_push_struct_zero(_os_glfw_perm_arena, _OS_GLFW_Window);
    window->glfw_window     = glfw_window;
    window->key_callback    = key_callback;
    window->width           = width;
    window->height          = height;
    OS_Handle result        = {int_from_ptr(window)};
    if (os_handle_is_zero(_os_glfw_main_window))
    {
        _os_glfw_main_window = result;
    }

    /** Key Code Conversion */
    _os_set_key_conversion(GLFW_KEY_RIGHT_BRACKET, OS_KeyCode_BracketRight);
    _os_set_key_conversion(GLFW_KEY_LEFT_BRACKET, OS_KeyCode_BracketLeft);
    _os_set_key_conversion(GLFW_KEY_SPACE, OS_KeyCode_Space);
    _os_set_key_conversion(GLFW_KEY_BACKSPACE, OS_KeyCode_Backspace);
    _os_set_key_conversion(GLFW_KEY_TAB, OS_KeyCode_Tab);
    _os_set_key_conversion(GLFW_KEY_ENTER, OS_KeyCode_Enter);
    _os_set_key_conversion(GLFW_KEY_ESCAPE, OS_KeyCode_Escape);
    _os_set_key_conversion(GLFW_KEY_DELETE, OS_KeyCode_Delete);

    /* Arrow Keys */
    _os_set_key_conversion(GLFW_KEY_RIGHT, OS_KeyCode_RightArrow);
    _os_set_key_conversion(GLFW_KEY_LEFT, OS_KeyCode_LeftArrow);
    _os_set_key_conversion(GLFW_KEY_DOWN, OS_KeyCode_DownArrow);
    _os_set_key_conversion(GLFW_KEY_UP, OS_KeyCode_UpArrow);

    /* Function Keys */
    _os_set_key_conversion(GLFW_KEY_F1, OS_KeyCode_F1);
    _os_set_key_conversion(GLFW_KEY_F2, OS_KeyCode_F2);
    _os_set_key_conversion(GLFW_KEY_F3, OS_KeyCode_F3);
    _os_set_key_conversion(GLFW_KEY_F4, OS_KeyCode_F4);
    _os_set_key_conversion(GLFW_KEY_F5, OS_KeyCode_F5);
    _os_set_key_conversion(GLFW_KEY_F6, OS_KeyCode_F6);
    _os_set_key_conversion(GLFW_KEY_F7, OS_KeyCode_F7);
    _os_set_key_conversion(GLFW_KEY_F8, OS_KeyCode_F8);
    _os_set_key_conversion(GLFW_KEY_F9, OS_KeyCode_F9);
    _os_set_key_conversion(GLFW_KEY_F10, OS_KeyCode_F10);
    _os_set_key_conversion(GLFW_KEY_F11, OS_KeyCode_F11);
    _os_set_key_conversion(GLFW_KEY_F12, OS_KeyCode_F12);

    /* Modifier Keys */
    _os_set_key_conversion(GLFW_KEY_LEFT_SHIFT, OS_KeyCode_LeftShift);
    _os_set_key_conversion(GLFW_KEY_RIGHT_SHIFT, OS_KeyCode_RightShift);
    _os_set_key_conversion(GLFW_KEY_LEFT_CONTROL, OS_KeyCode_LeftControl);
    _os_set_key_conversion(GLFW_KEY_RIGHT_CONTROL, OS_KeyCode_RightControl);
    _os_set_key_conversion(GLFW_KEY_LEFT_ALT, OS_KeyCode_LeftAlt);
    _os_set_key_conversion(GLFW_KEY_RIGHT_ALT, OS_KeyCode_RightAlt);

    /* Numbers */
    _os_set_key_conversion(GLFW_KEY_0, OS_KeyCode_0);
    _os_set_key_conversion(GLFW_KEY_1, OS_KeyCode_1);
    _os_set_key_conversion(GLFW_KEY_2, OS_KeyCode_2);
    _os_set_key_conversion(GLFW_KEY_3, OS_KeyCode_3);
    _os_set_key_conversion(GLFW_KEY_4, OS_KeyCode_4);
    _os_set_key_conversion(GLFW_KEY_5, OS_KeyCode_5);
    _os_set_key_conversion(GLFW_KEY_6, OS_KeyCode_6);
    _os_set_key_conversion(GLFW_KEY_7, OS_KeyCode_7);
    _os_set_key_conversion(GLFW_KEY_8, OS_KeyCode_8);
    _os_set_key_conversion(GLFW_KEY_9, OS_KeyCode_9);

    /* Letters */
    _os_set_key_conversion(GLFW_KEY_A, OS_KeyCode_A);
    _os_set_key_conversion(GLFW_KEY_B, OS_KeyCode_B);
    _os_set_key_conversion(GLFW_KEY_C, OS_KeyCode_C);
    _os_set_key_conversion(GLFW_KEY_D, OS_KeyCode_D);
    _os_set_key_conversion(GLFW_KEY_E, OS_KeyCode_E);
    _os_set_key_conversion(GLFW_KEY_F, OS_KeyCode_F);
    _os_set_key_conversion(GLFW_KEY_G, OS_KeyCode_G);
    _os_set_key_conversion(GLFW_KEY_H, OS_KeyCode_H);
    _os_set_key_conversion(GLFW_KEY_I, OS_KeyCode_I);
    _os_set_key_conversion(GLFW_KEY_J, OS_KeyCode_J);
    _os_set_key_conversion(GLFW_KEY_K, OS_KeyCode_K);
    _os_set_key_conversion(GLFW_KEY_L, OS_KeyCode_L);
    _os_set_key_conversion(GLFW_KEY_M, OS_KeyCode_M);
    _os_set_key_conversion(GLFW_KEY_N, OS_KeyCode_N);
    _os_set_key_conversion(GLFW_KEY_O, OS_KeyCode_O);
    _os_set_key_conversion(GLFW_KEY_P, OS_KeyCode_P);
    _os_set_key_conversion(GLFW_KEY_Q, OS_KeyCode_Q);
    _os_set_key_conversion(GLFW_KEY_R, OS_KeyCode_R);
    _os_set_key_conversion(GLFW_KEY_S, OS_KeyCode_S);
    _os_set_key_conversion(GLFW_KEY_T, OS_KeyCode_T);
    _os_set_key_conversion(GLFW_KEY_U, OS_KeyCode_U);
    _os_set_key_conversion(GLFW_KEY_V, OS_KeyCode_V);
    _os_set_key_conversion(GLFW_KEY_W, OS_KeyCode_W);
    _os_set_key_conversion(GLFW_KEY_X, OS_KeyCode_X);
    _os_set_key_conversion(GLFW_KEY_Y, OS_KeyCode_Y);
    _os_set_key_conversion(GLFW_KEY_Z, OS_KeyCode_Z);

    // _os_set_key_conversion(GLFW_MOUSE_BUTTON_RIGHT, OS_KeyCode_MouseRight);
    // _os_set_key_conversion(GLFW_MOUSE_BUTTON_MIDDLE, OS_KeyCode_MouseMiddle);
    _glfw_mouse_conversion_table[GLFW_MOUSE_BUTTON_LEFT]   = OS_KeyCode_MouseLeft;
    _glfw_mouse_conversion_table[GLFW_MOUSE_BUTTON_RIGHT]  = OS_KeyCode_MouseRight;
    _glfw_mouse_conversion_table[GLFW_MOUSE_BUTTON_MIDDLE] = OS_KeyCode_MouseMiddle;
    return result;
}

internal void
_os_set_key_conversion(uint32 glfw_keycode, OS_KeyCode os_keycode)
{
    _glfw_key_conversion_table[glfw_keycode] = os_keycode;
    _os_key_conversion_table[os_keycode]     = glfw_keycode;
}

internal IVec2
os_window_size()
{
    IVec2 result = {0};
    if (!os_window_is_ready())
        return result;

    _OS_GLFW_Window* window = (_OS_GLFW_Window*)ptr_from_int(_os_glfw_main_window.v);
    glfwGetWindowSize(window->glfw_window, &result.x, &result.y);
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

internal bool32
os_window_is_ready(void)
{
    return !os_handle_is_zero(_os_glfw_main_window);
}

internal void
_os_window_glfw_error_callback(int error, const char* description)
{
    log_error("Window Error, Code: %d, Desc: %s.", error, description);
}

internal void
_os_window_glfw_key_callback_wrapper(GLFWwindow* glfw_window, int key, int scancode, int action, int mods)
{
    (void)glfw_window;
    (void)scancode;
    (void)mods;
    _OS_GLFW_Window* window = (_OS_GLFW_Window*)ptr_from_int(_os_glfw_main_window.v);
    window->key_callback(_glfw_key_conversion_table[key], _glfw_action_conversion_table[action]);
}

internal void
_os_window_glfw_mouse_callback_wrapper(GLFWwindow* glfw_window, int button, int action, int mods)
{
    (void)glfw_window;
    (void)mods;
    _OS_GLFW_Window* window = (_OS_GLFW_Window*)ptr_from_int(_os_glfw_main_window.v);
    window->key_callback(_glfw_mouse_conversion_table[button], _glfw_action_conversion_table[action]);
}

/** Input */
internal Vec2
os_input_mouse_pos()
{
    _OS_GLFW_Window* window = (_OS_GLFW_Window*)ptr_from_int(_os_glfw_main_window.v);
    Vec2             result = {0};
    float64          x, y;
    glfwGetCursorPos(window->glfw_window, &x, &y);
    result.x = (float32)x;
    result.y = window->height - (float32)y;
    return result;
}

internal OS_KeyState
os_input_key_state(OS_KeyCode code)
{
    _OS_GLFW_Window* window    = (_OS_GLFW_Window*)ptr_from_int(_os_glfw_main_window.v);
    uint32           glfw_code = _os_key_conversion_table[code];
    OS_KeyState      result    = 0;
    uint32           state     = glfwGetMouseButton(window->glfw_window, glfw_code);
    if (state == GLFW_PRESS)
        flag_set(result, OS_KeyState_Pressed);
    if (state == GLFW_RELEASE)
        flag_set(result, OS_KeyState_Released);
    return result;
}