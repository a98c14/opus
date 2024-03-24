#include "main.h"

#define GLYPH_METRICS_CAPACITY 128
#define FREE_GLYPH_FONT_SIZE   16

int
main(void)
{
    /*  initialization */
    ThreadContext tctx;
    tctx_init_and_equip(&tctx);
    logger_init();
    Arena*  persistent_arena = make_arena_reserve(mb(128));
    Arena*  frame_arena      = make_arena_reserve(mb(128));
    Window* window           = window_create(persistent_arena, WINDOW_WIDTH, WINDOW_HEIGHT, "Scratch Window", NULL);

    RendererConfiguration* r_config = r_config_new(frame_arena);
    r_config_set_screen_size(r_config, WINDOW_WIDTH, WINDOW_HEIGHT);
    r_config_set_world_size(r_config, 0, WINDOW_HEIGHT);
    r_config_set_clear_color(r_config, ColorSlate900);
    renderer_init(persistent_arena, r_config);

    R_PipelineConfiguration* config       = r_pipeline_config_new(frame_arena);
    PassIndex                pass_default = r_pipeline_config_add_pass(config, FRAME_BUFFER_INDEX_DEFAULT);
    r_pipeline_init(config);
    font_cache_init(persistent_arena);
    draw_context_init(persistent_arena, frame_arena, g_renderer, pass_default);
    FontFaceIndex ibx_mono = font_load(string("ibx_mono"), string(ASSET_PATH "\\IBMPlexMono-Bold.ttf"), GlyphAtlasTypeFreeType);
    draw_activate_font(ibx_mono);

    EngineTime time = engine_time_new();

    const uint32 font_size = 24;
    InputMouse   mouse     = {0};

    Trail* mouse_trail = trail_new(persistent_arena);

    /* main loop */
    while (!glfwWindowShouldClose(window->glfw_window))
    {
        arena_reset(frame_arena);
        if (input_key_pressed_raw(window, GLFW_KEY_RIGHT_BRACKET))
            break;
        mouse = input_mouse_get(window, g_renderer->camera, mouse);
        time  = engine_get_time(time);

        trail_push_position(mouse_trail, mouse.world);
        trail_update(mouse_trail, time.dt);
        draw_scope(10, ViewTypeWorld, pass_default)
        {
            draw_rect(rect_at(vec2(-200, -200), vec2(100, 100), AlignmentBottom), ColorRed200);
            trail_draw(mouse_trail);
        }

        r_render(g_renderer, time.dt);
        window_update(window);
    }

    window_destroy(window);
    logger_flush();
    return 0;
}