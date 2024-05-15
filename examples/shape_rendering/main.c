#include "main.h"

#include "sprites.h"

#define GLYPH_METRICS_CAPACITY 128
#define FREE_GLYPH_FONT_SIZE   16

int
main(void)
{
    /* initialization */
    ThreadContext tctx;
    tctx_init_and_equip(&tctx);
    logger_init();

    Arena*     persistent_arena = make_arena_reserve(mb(128));
    Arena*     frame_arena      = make_arena_reserve(mb(128));
    Window*    window           = window_create(persistent_arena, WINDOW_WIDTH, WINDOW_HEIGHT, "Scratch Window", NULL);
    EngineTime time             = engine_time_new();
    InputMouse mouse            = {0};
    font_cache_init(persistent_arena);

    RendererConfiguration* r_config = r_config_new(frame_arena);
    r_config_set_screen_size(r_config, WINDOW_WIDTH, WINDOW_HEIGHT);
    r_config_set_world_size(r_config, 0, WINDOW_HEIGHT);
    r_config_set_clear_color(r_config, ColorSlate900);
    renderer_init(persistent_arena, r_config);

    ArenaTemp    temp          = scratch_begin(0, 0);
    TextureIndex texture       = texture_new_from_file(g_renderer, string(ASSET_PATH "\\textures\\game.png"), true, false);
    SpriteAtlas* atlas         = sprite_atlas_new(temp.arena, texture, Animations, Sprites, 0, array_count(Animations), array_count(Sprites));
    String       ascii_charset = string(" !\"#$%&\\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");

    R_PipelineConfiguration* config = r_pipeline_config_new(frame_arena);
    r_pipeline_config_add_pass(config, FRAME_BUFFER_INDEX_DEFAULT);
    r_pipeline_init(config);

    d_context_init(persistent_arena, frame_arena, AssetPath);

    scratch_end(temp);

    /* main loop */
    while (!glfwWindowShouldClose(window->glfw_window))
    {
        arena_reset(frame_arena);
        if (input_key_pressed_raw(window, GLFW_KEY_RIGHT_BRACKET))
            break;

        mouse = input_mouse_get(window, g_renderer->camera, mouse);
        time  = engine_get_time(time);

        g_renderer->timer += time.dt;
        g_renderer->stat_draw_count   = 0;
        g_renderer->stat_object_count = 0;

        d_circle(vec2(0, 0), 128, 0.2, ColorRed500);
        d_circle(vec2(800, 0), 128, 0.2, ColorRed500);
        d_string(vec2(-800, 0), ascii_charset, 28, ColorWhite);
        d_line(vec2(-800, 0), vec2(800, 0), 2, ColorGreen400);
        d_line(vec2(0, -500), vec2(0, 500), 2, ColorGreen400);

        d_sprite(atlas, SPRITE_GAME_CELESTIAL_OBJECTS_NEBULA_1, vec2(0, 0), vec2_one());
        d_string(mouse.world, string_pushf(frame_arena, "%.1f, %.1f", mouse.world.x, mouse.world.y), 28, ColorWhite);

        r_render(g_renderer, time.dt);

        window_update(window);
    }

    window_destroy(window);
    logger_flush();
    return 0;
}