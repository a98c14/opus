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

    /* main loop */
    while (!glfwWindowShouldClose(window->glfw_window))
    {
        arena_reset(frame_arena);
        if (input_key_pressed(window, GLFW_KEY_RIGHT_BRACKET))
            break;

        ShaderDataText shader_data    = {0};
        shader_data.color             = d_color_white;
        shader_data.thickness         = d_default_text_thickness;
        shader_data.softness          = d_default_text_softness;
        shader_data.outline_thickness = d_default_text_outline_thickness;

        Rect r = rect_at(vec2(-500, 0), vec2(600, 500), AlignmentLeft);
        draw_debug_rect_b(r);
        r = rect_shrink_f32(r, 10);

        draw_text(string("BottomLeft"), r, ANCHOR_BL_BL, font_size, ColorWhite100);
        draw_text(string("BottomRight"), r, ANCHOR_BR_BR, font_size, ColorWhite100);
        draw_text(string("TopLeft"), r, ANCHOR_TL_TL, font_size, ColorWhite100);
        draw_text(string("TopRight"), r, ANCHOR_TR_TR, font_size, ColorWhite100);

        Rect box = rect_at(vec2(-100, 0), vec2(600, 100), AlignmentBottomLeft);
        draw_debug_rect(box);
        draw_text(string("Here we go again"), rect_cut_top(&box, font_size), ANCHOR_TL_TL, font_size, ColorWhite100);
        draw_text(string("Second line"), rect_cut_top(&box, font_size), ANCHOR_TL_TL, font_size, ColorWhite100);
        draw_text(string("Third line"), rect_cut_top(&box, font_size), ANCHOR_TL_TL, font_size, ColorWhite100);
        // draw_text(string("Line 2"), rect_cut_top(&box, font_size), ANCHOR_BL_BL, font_size, ColorWhite100);
        // draw_text(string("Line 3"), rect_cut_top(&box, font_size), ANCHOR_BL_BL, font_size, ColorWhite100);
        // draw_text(string("Line 4"), rect_cut_top(&box, font_size), ANCHOR_BL_BL, font_size, ColorWhite100);

        r_render(g_renderer, time.dt);
        window_update(window);
    }

    window_destroy(window);
    logger_flush();
    return 0;
}