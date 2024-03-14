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
    InputMouse   mouse;

    /* main loop */
    while (!glfwWindowShouldClose(window->glfw_window))
    {
        arena_reset(frame_arena);
        if (input_key_pressed(window, GLFW_KEY_RIGHT_BRACKET))
            break;

        mouse = input_mouse_get(window, g_renderer->camera, mouse);

        Vec2    point_a   = vec2(100, 100);
        Vec2    point_b   = vec2(100, -100);
        Vec2    target    = vec2(50, 50);
        float32 thickness = 2;

        draw_line(point_a, target, ColorWhite100A, thickness);
        draw_line(point_b, target, ColorWhite100A, thickness);

        Vec2 d_a            = sub_vec2(target, point_a);
        Vec2 n_a            = norm_vec2_safe(d_a);
        Vec2 normal_a       = vec2(-n_a.y, n_a.x);
        Vec2 point_a_right  = add_vec2(point_a, mul_vec2_f32(normal_a, 10));
        Vec2 point_a_left   = add_vec2(point_a, mul_vec2_f32(normal_a, -10));
        Vec2 target_a_right = add_vec2(target, mul_vec2_f32(normal_a, 10));
        Vec2 target_a_left  = add_vec2(target, mul_vec2_f32(normal_a, -10));
        draw_line(target_a_right, target_a_left, ColorRed500AA, thickness);
        Vec2 d_a_left  = sub_vec2(target_a_left, point_a_left);
        Vec2 d_a_right = sub_vec2(target_a_right, point_a_right);

        Vec2 n_b      = norm_vec2_safe(sub_vec2(target, point_b));
        Vec2 normal_b = vec2(-n_b.y, n_b.x);

        Vec2 point_b_right  = add_vec2(point_b, mul_vec2_f32(normal_b, 10));
        Vec2 point_b_left   = add_vec2(point_b, mul_vec2_f32(normal_b, -10));
        Vec2 target_b_right = add_vec2(target, mul_vec2_f32(normal_b, 10));
        Vec2 target_b_left  = add_vec2(target, mul_vec2_f32(normal_b, -10));
        Vec2 d_b_left       = sub_vec2(target_b_left, point_b_left);
        Vec2 d_b_right      = sub_vec2(target_b_right, point_b_right);
        draw_line(target_b_left, target_b_right, ColorGreen200A, thickness);

        draw_line(point_a_right, target_a_right, ColorWhite, thickness);
        draw_line(point_b_left, target_b_left, ColorWhite, thickness);

        Vec2 intersection_right = vec2_intersection_fast(point_b_left, norm_vec2_safe(d_b_left), point_a_right, norm_vec2_safe(d_a_right));
        Vec2 intersection_left  = vec2_intersection_fast(point_a_left, norm_vec2_safe(d_a_left), point_b_right, norm_vec2_safe(d_b_right));
        draw_line(point_a_right, intersection_right, ColorBlue500, 2);
        draw_line(point_b_left, intersection_right, ColorBlue500, 2);
        draw_line(point_a_left, intersection_left, ColorBlue500, 2);
        draw_line(point_b_right, intersection_left, ColorBlue500, 2);
        r_render(g_renderer, time.dt);
        window_update(window);
    }

    window_destroy(window);
    logger_flush();
    return 0;
}