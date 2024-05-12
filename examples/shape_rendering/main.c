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

    Arena*       persistent_arena = make_arena_reserve(mb(128));
    Arena*       frame_arena      = make_arena_reserve(mb(128));
    Window*      window           = window_create(persistent_arena, WINDOW_WIDTH, WINDOW_HEIGHT, "Scratch Window", NULL);
    EngineTime   time             = engine_time_new();
    InputMouse   mouse            = {0};
    const uint32 font_size        = 30;
    font_cache_init(persistent_arena);

    RendererConfiguration* r_config = r_config_new(frame_arena);
    r_config_set_screen_size(r_config, WINDOW_WIDTH, WINDOW_HEIGHT);
    r_config_set_world_size(r_config, 0, WINDOW_HEIGHT);
    r_config_set_clear_color(r_config, ColorSlate900);
    renderer_init(persistent_arena, r_config);

    ArenaTemp    temp    = scratch_begin(0, 0);
    TextureIndex texture = texture_new_from_file(g_renderer, string(ASSET_PATH "\\textures\\game.png"), true, false);

    R_PipelineConfiguration* config       = r_pipeline_config_new(frame_arena);
    PassIndex                pass_default = r_pipeline_config_add_pass(config, FRAME_BUFFER_INDEX_DEFAULT);
    r_pipeline_init(config);

    d_context_init(persistent_arena);

    SpriteAtlas*  atlas       = sprite_atlas_new(temp.arena, texture, Animations, Sprites, 0, array_count(Animations), array_count(Sprites));
    FontFaceIndex ibx_mono    = font_load(string("ibx_mono"), string(ASSET_PATH "\\IBMPlexMono-Bold.ttf"), GlyphAtlasTypeFreeType);
    GlyphAtlas*   glyph_atlas = font_get_atlas(ibx_mono, font_size);

    scratch_end(temp);

    /* main loop */
    while (!glfwWindowShouldClose(window->glfw_window))
    {
        arena_reset(frame_arena);
        if (input_key_pressed_raw(window, GLFW_KEY_RIGHT_BRACKET))
            break;

        mouse = input_mouse_get(window, g_renderer->camera, mouse);
        time  = engine_get_time(time);

        /** new render pipelines */
        {
            FrameBuffer* frame_buffer = &g_renderer->frame_buffers[pass_default];
            r_frame_buffer_begin(frame_buffer);

            g_renderer->timer += time.dt;
            g_renderer->stat_draw_count   = 0;
            g_renderer->stat_object_count = 0;

            // /** setup batch */
            // RenderKey test_key = render_key_new_default(ViewTypeWorld, 5, pass_default, texture, d_context->material_sprite);
            // r_batch_scope(test_key)
            // {
            //     r_batch_sprite_push_sprite(atlas, SPRITE_GAME_SHIPS_RANGER, vec2(0, 0));
            //     r_batch_sprite_push_sprite(atlas, SPRITE_GAME_SHIPS_RANGER, vec2(300, 0));
            // }

            // r_batch_scope(test_key)
            // {
            //     r_batch_sprite_push_sprite(atlas, SPRITE_GAME_SHIPS_RANGER, vec2(-300, 0));
            // }

            // RenderKey font_key = render_key_new_default(ViewTypeWorld, 5, pass_default, glyph_atlas->texture, d_context->material_text);
            // r_batch_scope(font_key)
            // {
            //     r_batch_push_string(glyph_atlas, string(" !\"#$%&\\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), vec2(-900, 0), font_size);
            // }

            d_line(vec2(0, 0), vec2(100, 100), 3, ColorRed400);
            d_line(vec2(100, 100), vec2(200, 100), 3, ColorRed400);

            r_render(g_renderer, time.dt);
        }

        window_update(window);
    }

    window_destroy(window);
    logger_flush();
    return 0;
}