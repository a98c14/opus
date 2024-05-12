#include "main.h"

#include "sprites.h"

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

    ArenaTemp    temp    = scratch_begin(0, 0);
    TextureIndex texture = texture_new_from_file(g_renderer, string(ASSET_PATH "\\textures\\game.png"), true, false);

    R_PipelineConfiguration* config       = r_pipeline_config_new(frame_arena);
    PassIndex                pass_default = r_pipeline_config_add_pass(config, FRAME_BUFFER_INDEX_DEFAULT);
    r_pipeline_init(config);

    font_cache_init(persistent_arena);
    draw_context_init(persistent_arena, frame_arena, g_renderer, pass_default);
    FontFaceIndex ibx_mono = font_load(string("ibx_mono"), string(ASSET_PATH "\\IBMPlexMono-Bold.ttf"), GlyphAtlasTypeFreeType);

    SpriteAtlas* atlas = sprite_atlas_new(temp.arena, texture, Animations, Sprites, 0, array_count(Animations), array_count(Sprites));
    draw_activate_font(ibx_mono);

    EngineTime time = engine_time_new();

    const uint32 font_size = 24;
    InputMouse   mouse     = {0};

    VertexAttributeInfo* attr_info = r_attribute_info_new(temp.arena);
    r_attribute_info_add_vec2(attr_info); // layout(location = 0) in vec2 a_pos;
    r_attribute_info_add_vec2(attr_info); // layout(location = 1) in vec2 a_tex_coord;
    // r_attribute_info_add_int(attr_info);  // layout(location = 2) in flat int a_instance_id;

    MaterialIndex test_material = r_material_create(
        g_renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\test.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\test.frag")),
        sizeof(ShaderDataBasic),
        false, attr_info);

    MaterialIndex font_material = r_material_create(
        g_renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\test_glyph.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\test_glyph.frag")),
        sizeof(ShaderDataBasic),
        false, attr_info);

    GlyphAtlas* glyph_atlas = font_get_atlas(d_state->ctx->font_face, 30);

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

            Camera* camera = &g_renderer->camera;
            g_renderer->timer += time.dt;
            g_renderer->stat_draw_count   = 0;
            g_renderer->stat_object_count = 0;

            /** setup batch */

            RenderKey test_key = render_key_new_default(ViewTypeWorld, 5, pass_default, texture, test_material);
            r_batch_scope(test_key)
            {
                r_batch_sprite_push_sprite(atlas, SPRITE_GAME_SHIPS_RANGER, vec2(0, 0));
                r_batch_sprite_push_sprite(atlas, SPRITE_GAME_SHIPS_RANGER, vec2(300, 0));
            }

            r_batch_scope(test_key)
            {
                r_batch_sprite_push_sprite(atlas, SPRITE_GAME_SHIPS_RANGER, vec2(-300, 0));
            }

            RenderKey font_key = render_key_new_default(ViewTypeWorld, 5, pass_default, glyph_atlas->texture, font_material);
            r_batch_scope(font_key)
            {
                r_batch_push_glyph(glyph_atlas, vec2(0, 0), 'A');
                r_batch_push_glyph(glyph_atlas, vec2(30, 0), 'b');
                r_batch_push_glyph(glyph_atlas, vec2(60, 0), 'c');
            }

            r_render(g_renderer, time.dt);
        }

        window_update(window);
    }

    window_destroy(window);
    logger_flush();
    return 0;
}