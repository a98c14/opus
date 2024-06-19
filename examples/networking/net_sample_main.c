// clang-format off
#pragma once
#include "ft2build.h"
#include FT_FREETYPE_H
#include <corecrt_math.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(lib, "user32.lib")

#include <opus.h>
#include <opus.c>

#include "net_sample_server.c"
#include "net_sample_client.c"
// clang-format on

#define WINDOW_WIDTH           1920
#define WINDOW_HEIGHT          1080
#define GLYPH_METRICS_CAPACITY 128
#define FREE_GLYPH_FONT_SIZE   16

global read_only String AssetPath = string_comp("..\\assets");

typedef enum
{
    AppKey_Mutex,
} AppKey;

int
main(void)
{
    ThreadContext tctx;
    tctx_init_and_equip(&tctx);
    os_init();
    net_init();

    Arena*     persistent_arena = make_arena_reserve(mb(128));
    Arena*     frame_arena      = make_arena_reserve(mb(128));
    Window*    window           = window_create(persistent_arena, WINDOW_WIDTH, WINDOW_HEIGHT, "Networking Demo", NULL, true);
    EngineTime time             = engine_time_new();

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

    InputMouse mouse = {0};
    input_manager_init(persistent_arena, window);
    input_manager_register_action(string("mutex"), AppKey_Mutex, GLFW_KEY_L);

    /** demo state */
    uint64 start = os_now_us();

    ui_state_init(persistent_arena);

    /* main loop */
    ArenaTemp temp = scratch_begin(0, 0);
    while (!glfwWindowShouldClose(window->glfw_window))
    {
        scratch_end(temp);
        if (input_key_pressed_raw(window, GLFW_KEY_RIGHT_BRACKET))
            break;

        input_manager_update(time);

        uint64 frame_duration_us = os_now_us() - start;
        start                    = os_now_us();

        arena_reset(frame_arena);
        mouse = input_mouse_get(window, g_renderer->camera, mouse);
        time  = engine_get_time(time);

        ui_state_update(time, mouse);
        ui_create_fixed(screen_rect())
        {
            ui_shrink(28, 28);
            d_string(ui_cut_top(20), string("Networking Demo"), 20, ColorWhite, ANCHOR_TL_TL);
            d_string(ui_cut_top(20), string_pushf(temp.arena, "Frame: %.2fms", us_to_ms_f(frame_duration_us)), 20, ColorWhite, ANCHOR_TL_TL);

            ui_create(CutSideLeft, 300)
            {
                d_rect(ui_rect(), 0, ColorWhite100AA); // background
                ui_shrink(4, 4);
                if (ui_button(ui_key_cstr("server_button"), string("Server")))
                {
                    ServerOptions options = {.name = string("test_server"), .port = string("5055")};
                    os_thread_launch(server_entry_point, &options, 0);
                    log_info("server thread launched");
                }

                ui_cut_top(4);
                if (ui_button(ui_key_cstr("client_button"), string("Client")))
                {
                    ClientOptions options = {.name = string("test_client"), .address = string("127.0.0.1"), .port = string("5055")};
                    os_thread_launch(client_entry_point, &options, 0);
                    log_info("client thread launched");
                }
            }
        }

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
    return 0;
}