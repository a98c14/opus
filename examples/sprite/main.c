#include <corecrt_math.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(lib,"user32.lib")

#include <opus.h>
#include <opus.c>

#include "editor.h"
#include "app.h"

#include "editor.c"
#include "app.c"

int main(void)
{
    Engine* e = engine_init();
    DrawContext* dc = e->draw_context;
    Theme* t = e->theme; 

    /* app initialization */
    float32 dt_scale = 1;

    float32 world_width = e->renderer->world_width;
    float32 world_height = e->renderer->world_height;
    float32 world_half_width = world_width / 2.0f;
    float32 world_half_height = world_height / 2.0f;


    /* main loop */
    while (!glfwWindowShouldClose(e->window->glfw_window))
    {
        engine_frame_start(e);
        float32 dt = e->time.dt * dt_scale;

        Rect r = rect(100, 0, 100, 100);
        draw_rect(dc, r, 0, 10, e->theme->rect_default);

        Rect r2 = rect_from_wh(100, 20);
        draw_rect(dc, rect_shrink(rect_anchor(r2, r, ANCHOR_T_BL), vec2(4, 4)), 0, 10, e->theme->rect_debug);
        draw_rect(dc, rect_cut_top(&r, 20), 0, 10, e->theme->rect_debug);
        draw_rect(dc, rect_cut_top(&r, 20), 0, 10, e->theme->rect_debug);
        Rect header = rect_cut_top(&r, 20);
        draw_rect(dc, rect_cut_left(&header, 20), 0, 10, e->theme->rect_debug);
        draw_rect(dc, rect_cut_left(&header, 20), 0, 10, e->theme->rect_debug);
        draw_rect(dc, rect_cut_left(&header, 20), 0, 10, e->theme->rect_debug);
        draw_rect(dc, rect_cut_bottom(&r, 20), 0, 10, e->theme->rect_debug);
        draw_rect(dc, r, 0, 10, e->theme->rect_debug2);

        /** Control Panel */
        engine_render(e);
        editor_draw_stats(e);
    }

    window_destroy(e->window);
    logger_flush();
    return 0;
}