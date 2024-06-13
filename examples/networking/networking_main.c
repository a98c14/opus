#pragma once
#include "ft2build.h"
#include FT_FREETYPE_H
#include <corecrt_math.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(lib, "user32.lib")

#include <opus.h>

#include <net/net_inc.h>

#include <net/win/net_impl_win.c>
#include <opus.c>

#define WINDOW_WIDTH           1920
#define WINDOW_HEIGHT          1080
#define GLYPH_METRICS_CAPACITY 128
#define FREE_GLYPH_FONT_SIZE   16

global read_only String AssetPath = string_comp("..\\assets");

int
main(void)
{
    /* engine initialization */
    ThreadContext tctx;
    tctx_init_and_equip(&tctx);
    logger_init();

    Arena*     persistent_arena = make_arena_reserve(mb(128));
    Arena*     frame_arena      = make_arena_reserve(mb(128));
    Window*    window           = window_create(persistent_arena, WINDOW_WIDTH, WINDOW_HEIGHT, "Networking Demo", NULL, false);
    EngineTime time             = engine_time_new();
    InputMouse mouse            = {0};
    font_cache_init(persistent_arena);

    RendererConfiguration* r_config = r_config_new(frame_arena);
    r_config_set_screen_size(r_config, WINDOW_WIDTH, WINDOW_HEIGHT);
    r_config_set_world_size(r_config, 0, WINDOW_HEIGHT);
    r_config_set_clear_color(r_config, ColorSlate900);
    renderer_init(persistent_arena, r_config);

    R_PipelineConfiguration* config = r_pipeline_config_new(frame_arena);
    r_pipeline_config_add_pass(config, FRAME_BUFFER_INDEX_DEFAULT);
    r_pipeline_init(config);
    d_context_init(persistent_arena, frame_arena, AssetPath);

    /** demo state */

    /* main loop */
    while (!glfwWindowShouldClose(window->glfw_window))
    {
        if (input_key_pressed_raw(window, GLFW_KEY_RIGHT_BRACKET))
            break;

        arena_reset(frame_arena);
        mouse = input_mouse_get(window, g_renderer->camera, mouse);
        time  = engine_get_time(time);

        Rect ui = rect_shrink_f32(screen_rect(), 28);
        d_string(rect_cut_top(&ui, 20), string("Networking Demo"), 20, ColorWhite, ANCHOR_TL_TL);

        // random shapes
        d_circle(vec2(-800, 0), 128, 0.2, ColorRed500);
        d_circle(vec2(800, 0), 128, 0.2, ColorRed500);
        d_rect(rect_at(vec2(100, 300), vec2(100, 100), AlignmentCenter), 0, ColorWhite);
        d_rect(rect_at(vec2(-100, 300), vec2(100, 100), AlignmentCenter), 2, ColorWhite);
        d_string_at(mouse.world, string_pushf(frame_arena, "%.1f, %.1f", mouse.world.x, mouse.world.y), 15, ColorWhite, AlignmentCenter);

        r_render(g_renderer, time.dt);
        window_update(window);
    }

    window_destroy(window);
    logger_flush();
    return 0;
}