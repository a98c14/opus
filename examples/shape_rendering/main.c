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

    ArenaTemp temp = scratch_begin(0, 0);

    MaterialIndex test_material = material_new(
        g_renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\test.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\test.frag")),
        sizeof(ShaderDataBasic),
        false);

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
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_SLOT_CAMERA, g_renderer->camera_uniform_buffer_id);

            FrameBuffer* frame_buffer = &g_renderer->frame_buffers[pass_default];
            frame_buffer_begin(frame_buffer);

            glBindVertexArray(g_renderer->geometry_empty);
            Material* material = &g_renderer->materials[test_material];
            glUseProgram(material->gl_program_id);
            glUniform1i(material->location_texture, 0);

            uint32 element_count = 1;
            Mat4*  model_data    = arena_push_array_zero(frame_arena, Mat4, element_count);
            model_data[0]        = transform_quad_aligned(vec2_zero(), vec2_one());

            VertexBuffer* v = vertex_buffer_new(frame_arena);
            vertex_buffer_push(v, vec2(0, 0));
            vertex_buffer_push(v, vec2(0, 100));
            vertex_buffer_push(v, vec2(100, 0));

            vertex_buffer_push(v, vec2(0, 100));
            vertex_buffer_push(v, vec2(100, 0));
            vertex_buffer_push(v, vec2(100, 100));

            glUniformMatrix4fv(material->location_model, 1, GL_FALSE, model_data[0].v);

            glBindBuffer(GL_UNIFORM_BUFFER, material->uniform_buffer_id);
            glBindBufferRange(GL_UNIFORM_BUFFER, BINDING_SLOT_UBO_CUSTOM, material->uniform_buffer_id, 0, material->uniform_data_size);

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_renderer->trail_ssbo_id);
            Vec4* temp_buffer = arena_push_array(frame_arena, Vec4, v->count);
            for (uint32 i = 0; i < v->count; i++)
            {
                temp_buffer[i] = vec4(v->v[i].x, v->v[i].y, 0, 1);
            }
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Vec2) * v->count, v->v);

            glDrawArrays(GL_TRIANGLES, 0, v->count);
        }

        window_update(window);
    }

    window_destroy(window);
    logger_flush();
    return 0;
}