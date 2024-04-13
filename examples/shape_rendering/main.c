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

    ArenaTemp temp = scratch_begin(0, 0);

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
    r_attribute_info_add_int(attr_info);  // layout(location = 2) in flat int a_instance_id;

    MaterialIndex test_material = r_material_create(
        g_renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\test.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\test.frag")),
        sizeof(ShaderDataBasic),
        false, attr_info);
    scratch_end(temp);

    uint32 vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float32) * 4096, 0, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_SLOT_CAMERA, g_renderer->camera_uniform_buffer_id);

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
            Camera* camera = &g_renderer->camera;
            g_renderer->timer += time.dt;
            g_renderer->stat_draw_count   = 0;
            g_renderer->stat_object_count = 0;

            /* setup global shader data */
            GlobalUniformData global_shader_data = {0};
            global_shader_data.time              = g_renderer->timer;
            glBindBuffer(GL_UNIFORM_BUFFER, g_renderer->global_uniform_buffer_id);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GlobalUniformData), &global_shader_data);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_SLOT_SSBO_MODEL, g_renderer->mvp_ssbo_id);

            /* setup global camera data */
            CameraUniformData camera_data = {0};
            camera_data.view              = camera->view;
            camera_data.projection        = camera->projection;
            glBindBuffer(GL_UNIFORM_BUFFER, g_renderer->camera_uniform_buffer_id);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraUniformData), &camera_data);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(g_renderer->textures[texture].gl_texture_type, g_renderer->textures[texture].gl_texture_id);

            FrameBuffer* frame_buffer = &g_renderer->frame_buffers[pass_default];
            r_frame_buffer_begin(frame_buffer);

            /** setup batch */
            RenderKey test_key = render_key_new_default(ViewTypeWorld, 5, pass_default, texture, 0, test_material);

            R_BatchTexturedQuad* batch = r_batch_textured_quad_begin(test_key, 1);
            r_batch_textured_quad_push_sprite(batch, atlas, SPRITE_GAME_SHIPS_RANGER, vec2(200, 0));

            Mat4* model_data = arena_push_array_zero(frame_arena, Mat4, batch->element_count);
            model_data[0]    = transform_quad_aligned(vec2_zero(), vec2_one());

            Material* material = &g_renderer->materials[test_material];
            glUseProgram(material->gl_program_id);

            /** vertex data */
            glBindVertexArray(material->vertex_array_object);
            glBufferSubData(GL_ARRAY_BUFFER, 0, batch->vertex_count * material->vertex_size, batch->vertex_data);

            /** uniform data */
            glUniform1i(material->location_texture, 0);
            glUniformMatrix4fv(material->location_model, 1, GL_FALSE, model_data[0].v);
            glBindBuffer(GL_UNIFORM_BUFFER, material->uniform_buffer_id);
            glBindBufferRange(GL_UNIFORM_BUFFER, BINDING_SLOT_UBO_CUSTOM, material->uniform_buffer_id, 0, material->uniform_data_size);
            glDrawArrays(GL_TRIANGLES, 0, batch->element_count);
        }

        window_update(window);
    }

    window_destroy(window);
    logger_flush();
    return 0;
}