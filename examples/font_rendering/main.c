#include "main.h"

#define GLYPH_METRICS_CAPACITY 128
#define FREE_GLYPH_FONT_SIZE   16

// typedef struct
// {
//     float ax; // advance.x
//     float ay; // advance.y

//     float bw; // bitmap.width;
//     float bh; // bitmap.rows;

//     float bl; // bitmap_left;
//     float bt; // bitmap_top;

//     float tx; // x offset of glyph in texture coordinates
// } FreeGlyphMetric;

int
main(void)
{
    /*  initialization */
    ThreadContext tctx;
    tctx_init_and_equip(&tctx);
    logger_init();
    Arena*                 persistent_arena = make_arena_reserve(mb(128));
    Arena*                 frame_arena      = make_arena_reserve(mb(128));
    Window*                window           = window_create(persistent_arena, WINDOW_WIDTH, WINDOW_HEIGHT, "Scratch Window", NULL);
    RendererConfiguration* r_config         = r_config_new(frame_arena);
    r_config_set_screen_size(r_config, WINDOW_WIDTH, WINDOW_HEIGHT);
    r_config_set_world_size(r_config, 0, WINDOW_HEIGHT);
    r_config_set_clear_color(r_config, ColorSlate900);
    renderer_init(persistent_arena, r_config);

    R_PipelineConfiguration* config       = r_pipeline_config_new(frame_arena);
    PassIndex                pass_default = r_pipeline_config_add_pass(config, FRAME_BUFFER_INDEX_DEFAULT);
    r_pipeline_init(config);
    font_cache_init(persistent_arena);
    draw_context_init(persistent_arena, frame_arena, g_renderer, pass_default);
    EngineTime time = engine_time_new();

    /* main loop */
    while (!glfwWindowShouldClose(window->glfw_window))
    {
        arena_reset(frame_arena);
        if (input_key_pressed(window, GLFW_KEY_RIGHT_BRACKET))
            break;

        // draw_circle_filled(vec2_zero(), 100, ColorWhite);
        // draw_rect(rect_from_wh(200, 200), ColorWhite);

        String test_string = string("Do small fonts look ok? ggg");
        String english     = string("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890~!@#$%^&*()-_+=[{]}\\|;:'\",<.>/?");

        ShaderDataText shader_data    = {0};
        shader_data.color             = d_color_white;
        shader_data.thickness         = d_default_text_thickness;
        shader_data.softness          = d_default_text_softness;
        shader_data.outline_thickness = d_default_text_outline_thickness;

        Vec2 pos = vec2(-200, 50);
        draw_line(pos, vec2(200, pos.y), ColorRed500, 3);
        draw_text(english, rect_at(pos, vec2(200, 0), AlignmentBottomLeft), ANCHOR_BL_BL, 20, ColorWhite100);
        r_render(g_renderer, time.dt);
        window_update(window);
    }

    window_destroy(window);
    logger_flush();
    return 0;
}