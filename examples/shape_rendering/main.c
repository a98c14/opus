#include "main.h"

#include "sprites.h"

#define GLYPH_METRICS_CAPACITY 128
#define FREE_GLYPH_FONT_SIZE   16

int
main(void)
{
    /* engine initialization */
    ThreadContext tctx;
    tctx_init_and_equip(&tctx);

    Arena*     persistent_arena = make_arena_reserve(mb(128));
    Arena*     frame_arena      = make_arena_reserve(mb(128));
    Window*    window           = window_create(persistent_arena, WINDOW_WIDTH, WINDOW_HEIGHT, "Scratch Window", NULL, false);
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
    ArenaTemp temp          = scratch_begin(0, 0);
    String    ascii_charset = string(" !\"#$%&\\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");

    const float32 trail_rate  = 0.05f;
    float32       trail_clock = 0;
    Trail*        t           = d_trail_new(persistent_arena);
    d_trail_set_color(t, ColorWhite, ColorBlue400);
    scratch_end(temp);

    /* main loop */
    while (!glfwWindowShouldClose(window->glfw_window))
    {
        if (input_key_pressed_raw(window, GLFW_KEY_RIGHT_BRACKET))
            break;

        arena_reset(frame_arena);
        mouse = input_mouse_get(window, g_renderer->camera, mouse);
        time  = engine_get_time(time);

        d_circle(vec2(-800, 0), 128, 0.2, ColorRed500);
        d_circle(vec2(800, 0), 128, 0.2, ColorRed500);
        d_string_at(vec2(-800, -300), ascii_charset, 28, ColorWhite, AlignmentCenter);
        d_line(vec2(-800, -300), vec2(800, -300), 2, ColorGreen400);
        d_line(vec2(-800, 0), vec2(800, 0), 2, ColorGreen400);
        d_line(vec2(0, -500), vec2(0, 500), 2, ColorGreen400);
        d_string_at(mouse.world, string_pushf(frame_arena, "%.1f, %.1f", mouse.world.x, mouse.world.y), 15, ColorWhite, AlignmentCenter);

        d_rect(rect_at(vec2(100, 300), vec2(100, 100), AlignmentCenter), 0, ColorWhite);
        d_rect(rect_at(vec2(-100, 300), vec2(100, 100), AlignmentCenter), 2, ColorWhite);

        trail_clock -= time.dt;
        if (trail_clock <= 0)
        {
            d_trail_push_position(t, mouse.world);
            trail_clock = trail_rate;
        }
        d_trail_update(t, time.dt);
        d_trail_draw(t);

        r_render(g_renderer, time.dt);
        window_update(window);
    }

    window_destroy(window);
    return 0;
}