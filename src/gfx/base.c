#include "base.h"
#include "primitives.h"
#include <gfx/base.h>

internal RendererConfiguration*
r_config_new(Arena* temp_arena)
{
    RendererConfiguration* configuration = arena_push_struct_zero(temp_arena, RendererConfiguration);
    return configuration;
}

internal void
r_config_set_screen_size(RendererConfiguration* configuration, float32 width, float32 height)
{
    configuration->window_width  = width;
    configuration->window_height = height;
}

internal void
r_config_set_world_size(RendererConfiguration* configuration, float32 width, float32 height)
{
    configuration->world_width  = width;
    configuration->world_height = height;
}

internal void
r_config_set_clear_color(RendererConfiguration* configuration, Color color)
{
    configuration->clear_color = color;
}

internal R_PipelineConfiguration*
r_pipeline_config_new(Arena* temp_arena)
{
    R_PipelineConfiguration* configuration = arena_push_struct_zero(temp_arena, R_PipelineConfiguration);
    configuration->temp_arena              = temp_arena;
    return configuration;
}

internal PassIndex
r_pipeline_config_add_pass(R_PipelineConfiguration* config, FrameBufferIndex frame_buffer)
{
    R_PassConfigurationNode* node = arena_push_struct_zero(config->temp_arena, R_PassConfigurationNode);
    node->frame_buffer_index      = frame_buffer;
    PassIndex pass_index          = config->pass_count;
    config->pass_count++;
    queue_push(config->first_pass, config->last_pass, node);
    return pass_index;
}

internal void
renderer_init(Arena* arena, RendererConfiguration* configuration)
{
    g_renderer                = arena_push_struct_zero(arena, Renderer);
    g_renderer->arena         = arena;
    g_renderer->frame_arena   = make_arena_reserve(mb(128));
    g_renderer->window_width  = configuration->window_width;
    g_renderer->window_height = configuration->window_height;
    g_renderer->frame_buffers = arena_push_array_zero(arena, FrameBuffer, LAYER_CAPACITY);
    g_renderer->materials     = arena_push_array_zero(arena, Material, MATERIAL_CAPACITY);
    g_renderer->textures      = arena_push_array_zero(arena, Texture, TEXTURE_CAPACITY);
    g_renderer->geometries    = arena_push_array_zero(arena, Geometry, GEOMETRY_CAPACITY);

    xassert(configuration->world_width > 0 || configuration->world_height > 0, "at least one of world width or world height needs to have a value");
    glViewport(0, 0, g_renderer->window_width, g_renderer->window_height);

    g_renderer->aspect   = g_renderer->window_width / (float)g_renderer->window_height;
    float32 world_height = configuration->world_height;
    float32 world_width  = configuration->world_width;

    if (configuration->world_width == 0)
        world_width = world_height * g_renderer->aspect;
    if (configuration->world_height == 0)
        world_height = world_width / g_renderer->aspect;
    g_renderer->world_width  = world_width;
    g_renderer->world_height = world_height;
    g_renderer->ppu          = 1.0f / (g_renderer->window_width / world_width);
    g_renderer->camera       = camera_new(world_width, world_height, 100, -100, g_renderer->window_width, g_renderer->window_height);

    // TEMP: testing global variable solution out. Potentially dangerous?
    _pixel_per_unit = g_renderer->ppu;

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    Vec4 clear_color = color_to_vec4(configuration->clear_color);
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);

    /* Create Global UBO */
    glGenBuffers(1, &g_renderer->global_uniform_buffer_id);
    glBindBuffer(GL_UNIFORM_BUFFER, g_renderer->global_uniform_buffer_id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GlobalUniformData), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, BINDING_SLOT_GLOBAL, g_renderer->global_uniform_buffer_id, 0, sizeof(GlobalUniformData));

    /* Create Texture UBO */
    glGenBuffers(1, &g_renderer->texture_uniform_buffer_id);
    glBindBuffer(GL_UNIFORM_BUFFER, g_renderer->texture_uniform_buffer_id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(TextureUniformData), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, BINDING_SLOT_TEXTURE, g_renderer->texture_uniform_buffer_id, 0, sizeof(TextureUniformData));

    /* Create Camera UBO */
    glGenBuffers(1, &g_renderer->camera_uniform_buffer_id);
    glBindBuffer(GL_UNIFORM_BUFFER, g_renderer->camera_uniform_buffer_id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraUniformData), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, BINDING_SLOT_CAMERA, g_renderer->camera_uniform_buffer_id, 0, sizeof(CameraUniformData));

    /* Create MVP SSBO */
    glGenBuffers(1, &g_renderer->mvp_ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_renderer->mvp_ssbo_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Mat4) * MATERIAL_DRAW_BUFFER_ELEMENT_CAPACITY, 0, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_SLOT_SSBO_MODEL, g_renderer->mvp_ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    /* Draw State Setup */
    g_renderer->draw_state = renderer_draw_state_new(arena);

    // Reserve the first slot for NULL texture
    g_renderer->texture_count += 1;

    // Initialize default frame buffer
    g_renderer->frame_buffer_count += 1;
    FrameBuffer* frame_buffer   = &g_renderer->frame_buffers[FRAME_BUFFER_INDEX_DEFAULT];
    frame_buffer->texture_index = TEXTURE_INDEX_NULL;
    frame_buffer->buffer_id     = 0;
    frame_buffer->clear_color   = clear_color;
    frame_buffer->width         = configuration->window_width;
    frame_buffer->height        = configuration->window_height;

    /** frequently used primitives */
    g_renderer->quad     = geometry_quad_create(g_renderer);
    g_renderer->triangle = geometry_triangle_create(g_renderer);

    log_debug("renderer created");
}

internal void
r_pipeline_init(R_PipelineConfiguration* configuration)
{
    g_renderer->pass_count = configuration->pass_count;
    g_renderer->passes     = arena_push_array_zero(g_renderer->arena, R_Pass, g_renderer->pass_count);
    R_PassConfigurationNode* node;
    uint32                   pass_index = 0;
    for_each(node, configuration->first_pass)
    {
        g_renderer->passes[pass_index].frame_buffer = node->frame_buffer_index;
        g_renderer->passes[pass_index].batch_groups = arena_push_array_zero(g_renderer->arena, R_BatchGroup, SORTING_LAYER_CAPACITY);
        pass_index++;
    }

    g_renderer->active_render_key = render_key_new(0, 0, 0, TEXTURE_INDEX_NULL, GEOMETRY_CAPACITY - 1, MATERIAL_CAPACITY - 1); // TODO(selim): find a way to show invalid values properly;
}

internal RendererDrawState*
renderer_draw_state_new(Arena* arena)
{
    RendererDrawState* draw_state     = arena_push_struct_zero(arena, RendererDrawState);
    draw_state->material_draw_buffers = arena_push_array_zero(arena, MaterialDrawBuffer, MATERIAL_DRAW_BUFFER_CAPACITY);
    for (int i = 0; i < MATERIAL_DRAW_BUFFER_CAPACITY; i++)
    {
        draw_state->material_draw_buffers[i].key = MATERIAL_DRAW_BUFFER_EMPTY_KEY;
    }

    return draw_state;
}

internal Camera
camera_new(float32 width, float32 height, float32 near_plane, float32 far_plane, float32 window_width, float32 window_height)
{
    Camera result;
    result.projection    = mat4_ortho(width, height, near_plane, far_plane);
    result.view          = mat4_identity();
    result.inverse_view  = mat4_identity();
    result.world_width   = width;
    result.world_height  = height;
    result.window_width  = window_width;
    result.window_height = window_height;
    return result;
}

internal uint32
shader_load(String vertex_shader_text, String fragment_shader_text)
{
    GLuint vertex_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const char* const*)(&vertex_shader_text.value), NULL);
    glCompileShader(vertex_shader);

    GLuint fragment_shader;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const char* const*)(&fragment_shader_text.value), NULL);
    glCompileShader(fragment_shader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

internal GeometryIndex
geometry_new(Renderer* renderer, int32 index_count, int32 vertex_array_object)
{
    GeometryIndex geometry_index                             = renderer->geometry_count;
    renderer->geometries[geometry_index].index_count         = index_count;
    renderer->geometries[geometry_index].vertex_array_object = vertex_array_object;
    renderer->geometry_count++;
    return geometry_index;
}

internal MaterialIndex
material_new(Renderer* renderer, String vertex_shader_text, String fragment_shader_text, usize uniform_data_size, bool32 is_instanced)
{
    MaterialIndex material_index = renderer->material_count;
    renderer->material_count++;
    Material* result          = &renderer->materials[material_index];
    result->gl_program_id     = shader_load(vertex_shader_text, fragment_shader_text);
    result->location_texture  = glGetUniformLocation(result->gl_program_id, "u_main_texture");
    result->uniform_data_size = uniform_data_size;
    result->is_initialized    = 1;
    result->is_instanced      = is_instanced;

    // generate custom shader data UBO
    if (is_instanced)
    {
        result->location_model = -1;
        glGenBuffers(1, &result->uniform_buffer_id);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, result->uniform_buffer_id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, uniform_data_size * MATERIAL_DRAW_BUFFER_ELEMENT_CAPACITY, 0, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
    else
    {
        glGenBuffers(1, &result->uniform_buffer_id);
        glBindBuffer(GL_UNIFORM_BUFFER, result->uniform_buffer_id);
        glBufferData(GL_UNIFORM_BUFFER, uniform_data_size, NULL, GL_STREAM_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        result->location_model = glGetUniformLocation(result->gl_program_id, "u_model");
    }

    unsigned int global_ubo_index = glGetUniformBlockIndex(result->gl_program_id, "Global");
    glUniformBlockBinding(result->gl_program_id, global_ubo_index, BINDING_SLOT_GLOBAL);

    unsigned int texture_ubo_index = glGetUniformBlockIndex(result->gl_program_id, "Texture");
    glUniformBlockBinding(result->gl_program_id, texture_ubo_index, BINDING_SLOT_TEXTURE);

    return material_index;
}

internal TextureIndex
texture_new(Renderer* renderer, uint32 width, uint32 height, uint32 channels, uint32 filter, void* data)
{
    TextureIndex texture_index = renderer->texture_count;
    renderer->texture_count++;
    Texture* texture = &renderer->textures[texture_index];
    glGenTextures(1, &texture->gl_texture_id);
    glBindTexture(GL_TEXTURE_2D, texture->gl_texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    texture->gl_texture_type = GL_TEXTURE_2D;
    texture->width           = width;
    texture->height          = height;
    texture->channels        = channels;
    texture->layer_count     = 1;

    switch (channels)
    {
    case 4:
        texture->format = GL_RGBA;
        break;
    case 3:
        texture->format = GL_RGB;
        break;
    case 2:
        texture->format = GL_RED;
        break;
    case 1:
        texture->format = GL_RED;
        break;
    default:
        texture->format = GL_RGBA;
        break;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, texture->format, width, height, 0, texture->format, GL_UNSIGNED_BYTE, data);

    return texture_index;
}

internal TextureIndex
texture_array_new(Renderer* renderer, uint32 width, uint32 height, uint32 channels, uint32 filter, uint32 layer_count, TextureData* data)
{
    log_trace("loading texture array");
    TextureIndex texture_index = renderer->texture_count;
    Texture*     texture       = &renderer->textures[renderer->texture_count];

    texture->width           = width;
    texture->height          = height;
    texture->channels        = channels;
    texture->layer_count     = layer_count;
    texture->gl_texture_type = GL_TEXTURE_2D_ARRAY;

    glGenTextures(1, &texture->gl_texture_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture->gl_texture_id);
    // TODO: figure out if this is needed or not
    // glTexStorage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, texture->width, texture->height, layer_count);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, texture->width, texture->height, layer_count, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    for (int i = 0; i < layer_count; i++)
    {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, texture->width, texture->height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data[i].value);
    }
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, filter);
    renderer->texture_count++;
    return texture_index;
}

internal void
texture_update(Renderer* renderer, TextureIndex texture, void* data)
{
    Texture* texture_data = &renderer->textures[texture];
    glBindTexture(GL_TEXTURE_2D, texture_data->gl_texture_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_data->width, texture_data->height, texture_data->format, GL_UNSIGNED_BYTE, data);
}

internal RenderKey
render_key_new(ViewType view_type, SortLayerIndex sort_layer, PassIndex pass, TextureIndex texture, GeometryIndex geometry, MaterialIndex material_index)
{
    xassert(view_type < 4, "invalid view_type value provided");
    log_trace("render key new, sort: %2d, pass: %2d, view: %2d, texture: %2d, geometry: %2d, material: %2d", sort_layer, pass, view_type, texture, geometry, material_index);
    RenderKey result = ((uint64)sort_layer << RenderKeySortLayerIndexBitStart) +
                       ((uint64)pass << RenderKeyPassIndexBitStart) +
                       ((uint64)view_type << RenderKeyViewTypeBitStart) +
                       ((uint64)texture << RenderKeyTextureIndexBitStart) +
                       ((uint64)geometry << RenderKeyGeometryIndexBitStart) +
                       ((uint64)material_index << RenderKeyMaterialIndexBitStart);
    return result;
}

internal uint64
render_key_mask(RenderKey key, uint64 bit_start, uint64 bit_count)
{
    return (key >> bit_start) & ((1 << bit_count) - 1);
}

internal void
frame_buffer_begin(FrameBuffer* frame_buffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer->buffer_id);
    glViewport(0, 0, frame_buffer->width, frame_buffer->height);
    glClearColor(frame_buffer->clear_color.x, frame_buffer->clear_color.y, frame_buffer->clear_color.z, frame_buffer->clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

internal TextureIndex
frame_buffer_texture(Renderer* renderer, FrameBufferIndex frame_buffer_index)
{
    return renderer->frame_buffers[frame_buffer_index].texture_index;
}

internal FrameBufferIndex
renderer_frame_buffer_init(Renderer* renderer, uint32 width, uint32 height, uint32 filter, Color clear_color)
{
    TextureIndex texture_index = texture_new(renderer, width, height, 4, filter, NULL);

    uint32 fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->textures[texture_index].gl_texture_id, 0);

    // create render buffer
    uint32 rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        log_error("frame_buffer is not complete");
    }

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // init frame buffer
    FrameBufferIndex layer_index  = renderer->frame_buffer_count;
    FrameBuffer*     frame_buffer = &renderer->frame_buffers[layer_index];
    frame_buffer->buffer_id       = fbo;
    frame_buffer->texture_index   = texture_index;
    frame_buffer->width           = width;
    frame_buffer->height          = height;
    frame_buffer->clear_color     = color_to_vec4(clear_color);
    renderer->frame_buffer_count++;

    return layer_index;
}

internal Vec4
color_to_vec4(Color c)
{
    return (Vec4){.r = c.r / 255.0F, .g = c.g / 255.0F, .b = c.b / 255.0F, .a = c.a / 255.0F};
}

internal Color
vec4_to_color(Vec4 c)
{
    return (Color){.r = c.r * 255.0F, .g = c.g * 255.0F, .b = c.b * 255.0F, .a = c.a * 255.0F};
}

internal void
renderer_render(Renderer* renderer, float32 dt)
{
    Camera*            camera = &renderer->camera;
    RendererDrawState* state  = renderer->draw_state;
    renderer->timer += dt;
    renderer->stat_draw_count   = 0;
    renderer->stat_object_count = 0;

    /* setup global shader data */
    GlobalUniformData global_shader_data = {0};
    global_shader_data.time              = renderer->timer;
    glBindBuffer(GL_UNIFORM_BUFFER, renderer->global_uniform_buffer_id);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GlobalUniformData), &global_shader_data);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_SLOT_SSBO_MODEL, renderer->mvp_ssbo_id);

    /* Layer */
    for (uint8 layer_index = 0; layer_index < state->layer_count; layer_index++)
    {
        LayerDrawBuffer* layer_draw_buffer = &state->layer_draw_buffers[layer_index];
        FrameBuffer*     frame_buffer      = &renderer->frame_buffers[layer_draw_buffer->layer_index];
        frame_buffer_begin(frame_buffer);

        /* Sort Layer */
        for (int8 sort_layer_index = 0; sort_layer_index < SORTING_LAYER_CAPACITY; sort_layer_index++)
        {
            SortingLayerDrawBuffer* sort_layer_draw_buffer = &layer_draw_buffer->sorting_layer_draw_buffers[sort_layer_index];

            /* View */
            for (uint8 view_type_index = 0; view_type_index < ViewTypeCOUNT; view_type_index++)
            {
                ViewDrawBuffer* view_draw_buffer = &sort_layer_draw_buffer->view_buffers[view_type_index];
                Mat4            view_matrix      = view_draw_buffer->view_type == ViewTypeWorld ? camera->view : mat4_identity();

                CameraUniformData camera_data = {0};
                camera_data.view              = view_matrix;
                camera_data.projection        = camera->projection;
                glBindBuffer(GL_UNIFORM_BUFFER, renderer->camera_uniform_buffer_id);
                glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraUniformData), &camera_data);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_SLOT_CAMERA, renderer->camera_uniform_buffer_id);

                /* Texture */
                for (uint8 texture_index = 0; texture_index < view_draw_buffer->texture_count; texture_index++)
                {
                    TextureDrawBuffer* texture_draw_buffer = &view_draw_buffer->texture_draw_buffers[texture_index];
                    if (texture_draw_buffer->texture_index != TEXTURE_INDEX_NULL)
                    {
                        Texture* texture = &renderer->textures[texture_draw_buffer->texture_index];
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(texture->gl_texture_type, texture->gl_texture_id);
                        texture_shader_data_set(renderer, texture);
                    }

                    /* Geometry */
                    for (uint8 geometry_internal_index = 0; geometry_internal_index < texture_draw_buffer->geometry_count; geometry_internal_index++)
                    {
                        GeometryDrawBuffer* geometry_draw_buffer = &texture_draw_buffer->geometry_draw_buffers[geometry_internal_index];
                        Geometry            geometry             = renderer->geometries[geometry_draw_buffer->geometry_index];
                        // don't update state if not needed
                        if (state->active_geometry.vertex_array_object != geometry.vertex_array_object)
                        {
                            state->active_geometry = geometry;
                            glBindVertexArray(geometry.vertex_array_object);
                        }

                        /* Draw Buffers */
                        for (int internal_index = 0; internal_index < geometry_draw_buffer->material_count; internal_index++)
                        {
                            MaterialDrawBufferIndex material_draw_buffer_index = geometry_draw_buffer->material_buffer_indices[internal_index];
                            MaterialDrawBuffer*     material_draw_buffer       = &state->material_draw_buffers[material_draw_buffer_index];
                            Material*               material                   = &g_renderer->materials[material_draw_buffer->material_index];
                            Geometry*               geometry                   = &g_renderer->geometries[geometry_draw_buffer->geometry_index];

                            glUseProgram(material->gl_program_id);
                            glUniform1i(material->location_texture, 0);
                            r_draw_batch_internal(geometry, material, material_draw_buffer->element_count, material_draw_buffer->model_buffer, material_draw_buffer->shader_data_buffer);
                            material_draw_buffer->element_count = 0;
                        }
                    }
                }
            }
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

internal void
renderer_v2_render(Renderer* renderer, float32 dt)
{
    xassert(renderer->pass_count > 0, "At least ONE render pass must be configured!");
    Camera* camera = &renderer->camera;
    renderer->timer += dt;
    renderer->stat_draw_count   = 0;
    renderer->stat_object_count = 0;

    /* setup global shader data */
    GlobalUniformData global_shader_data = {0};
    global_shader_data.time              = renderer->timer;
    glBindBuffer(GL_UNIFORM_BUFFER, renderer->global_uniform_buffer_id);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GlobalUniformData), &global_shader_data);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_SLOT_SSBO_MODEL, renderer->mvp_ssbo_id);

    /* setup global camera data */
    CameraUniformData camera_data = {0};
    camera_data.view              = camera->view;
    camera_data.projection        = camera->projection;
    glBindBuffer(GL_UNIFORM_BUFFER, renderer->camera_uniform_buffer_id);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraUniformData), &camera_data);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_SLOT_CAMERA, renderer->camera_uniform_buffer_id);

    for (uint32 i = 0; i < renderer->pass_count; i++)
    {
        R_Pass* pass = &renderer->passes[i];
        if (pass->total_draw_count <= 0)
            continue;

        FrameBuffer* frame_buffer = &renderer->frame_buffers[pass->frame_buffer];
        frame_buffer_begin(frame_buffer);

        for (uint32 j = 0; j < SORTING_LAYER_CAPACITY; j++)
        {
            R_BatchGroup* batch_list = &pass->batch_groups[j];
            if (batch_list->batch_count <= 0)
                continue;

            R_BatchNode* batch_node;
            for_each(batch_node, batch_list->first)
            {
                R_Batch batch                 = batch_node->v;
                uint64  diff                  = g_renderer->active_render_key ^ batch.key;
                g_renderer->active_render_key = batch.key;

                /** set texture */
                TextureIndex texture_index = render_key_mask(batch.key, RenderKeyTextureIndexBitStart, RenderKeyTextureIndexBitCount);
                if (render_key_mask(diff, RenderKeyTextureIndexBitStart, RenderKeyTextureIndexBitCount) > 0 && texture_index > 0)
                {
                    Texture* texture = &renderer->textures[texture_index];
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(texture->gl_texture_type, texture->gl_texture_id);
                    texture_shader_data_set(renderer, texture);
                }

                /** set view matrix */
                ViewType view_type = render_key_mask(batch.key, RenderKeyViewTypeBitStart, RenderKeyViewTypeBitCount);
                if (render_key_mask(diff, RenderKeyViewTypeBitStart, RenderKeyViewTypeBitCount) > 0)
                {
                    camera_data.view = view_type == ViewTypeWorld ? camera->view : mat4_identity();
                    glBindBuffer(GL_UNIFORM_BUFFER, renderer->camera_uniform_buffer_id);
                    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraUniformData), &camera_data);
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_SLOT_CAMERA, renderer->camera_uniform_buffer_id);
                }

                /** set material */
                MaterialIndex material_index = render_key_mask(batch.key, RenderKeyMaterialIndexBitStart, RenderKeyMaterialIndexBitCount);
                Material*     material       = &renderer->materials[material_index];
                if (render_key_mask(diff, RenderKeyMaterialIndexBitStart, RenderKeyMaterialIndexBitCount) > 0)
                {
                    glUseProgram(material->gl_program_id);
                    glUniform1i(material->location_texture, 0);
                }

                /** set geometry */
                GeometryIndex geometry_index = render_key_mask(batch.key, RenderKeyGeometryIndexBitStart, RenderKeyGeometryIndexBitCount);
                Geometry*     geometry       = &renderer->geometries[geometry_index];
                if (render_key_mask(diff, RenderKeyGeometryIndexBitStart, RenderKeyGeometryIndexBitCount) > 0)
                {
                    glBindVertexArray(geometry->vertex_array_object);
                }

                log_trace("rendering, sort: %2d, layer: %2d, view: %2d, texture: %2d, geometry: %2d, material: %2d", i, pass->frame_buffer, view_type, texture_index, geometry_index, material_index);

                r_draw_batch_internal(geometry, material, batch.element_count, batch.model_buffer, batch.uniform_buffer);
            }

            // NOTE(selim): if we don't zero the list it keeps the old references in the next frame
            memory_zero_struct(batch_list);
        }
    }

    arena_reset(renderer->frame_arena);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

internal void
texture_shader_data_set(Renderer* renderer, const Texture* texture)
{
    TextureUniformData shader_data = {0};
    shader_data.layer_count        = texture->layer_count;
    shader_data.size               = (Vec2){.x = (float32)texture->width, .y = (float32)texture->height};
    glBindBuffer(GL_UNIFORM_BUFFER, renderer->texture_uniform_buffer_id);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(TextureUniformData), &shader_data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

/** V2 */
internal R_Batch*
r_batch_from_key(RenderKey key, uint64 element_count)
{
    MaterialIndex material_index    = render_key_mask(key, RenderKeyMaterialIndexBitStart, RenderKeyMaterialIndexBitCount);
    uint64        uniform_data_size = g_renderer->materials[material_index].uniform_data_size;

    PassIndex      pass_index       = render_key_mask(key, RenderKeyPassIndexBitStart, RenderKeyPassIndexBitCount);
    SortLayerIndex sort_layer_index = render_key_mask(key, RenderKeySortLayerIndexBitStart, RenderKeySortLayerIndexBitCount);
    R_Pass*        pass             = &g_renderer->passes[pass_index];
    pass->total_draw_count += element_count;

    R_BatchGroup* batch_group = &pass->batch_groups[sort_layer_index];
    batch_group->batch_count++;

    R_BatchNode* batch_node         = arena_push_struct_zero(g_renderer->frame_arena, R_BatchNode);
    batch_node->v.key               = key;
    batch_node->v.element_count     = element_count;
    batch_node->v.uniform_data_size = uniform_data_size;
    batch_node->v.model_buffer      = arena_push_array(g_renderer->frame_arena, Mat4, element_count);
    batch_node->v.uniform_buffer    = arena_push(g_renderer->frame_arena, uniform_data_size * element_count);
    queue_push(batch_group->first, batch_group->last, batch_node);
    return &batch_node->v;
}

internal void
r_draw_single(RenderKey key, Mat4 model, void* uniform_data)
{
    r_draw_many(key, 1, &model, uniform_data);
}

internal void
r_draw_many(RenderKey key, uint64 count, Mat4* models, void* uniform_data)
{
    R_Batch* batch = r_batch_from_key(key, count);
    memcpy(batch->model_buffer, models, sizeof(Mat4) * count);
    memcpy((uint8*)batch->uniform_buffer, uniform_data, batch->uniform_data_size * count);
}

internal void
r_draw_many_no_copy(RenderKey key, uint64 count, Mat4* models, void* uniform_data)
{
    R_BatchNode* batch_node      = arena_push_struct_zero(g_renderer->frame_arena, R_BatchNode);
    batch_node->v.element_count  = count;
    batch_node->v.model_buffer   = models;
    batch_node->v.uniform_buffer = uniform_data;

    PassIndex      pass_index       = render_key_mask(key, RenderKeyPassIndexBitStart, RenderKeyPassIndexBitCount);
    SortLayerIndex sort_layer_index = render_key_mask(key, RenderKeySortLayerIndexBitStart, RenderKeySortLayerIndexBitCount);

    R_BatchGroup* batch_group = &g_renderer->passes[pass_index].batch_groups[sort_layer_index];
    queue_push(batch_group->first, batch_group->last, batch_node);
}

internal void
r_draw_pass(PassIndex source_index, PassIndex target_index, SortLayerIndex sort_layer, MaterialIndex material_index, void* uniform_data)
{
    R_Pass*      source_pass         = &g_renderer->passes[source_index];
    FrameBuffer* source_frame_buffer = &g_renderer->frame_buffers[source_pass->frame_buffer];

    Mat4      model = transform_quad_aligned(vec3_zero(), vec2(g_renderer->world_width, g_renderer->world_height));
    RenderKey key   = render_key_new(ViewTypeScreen, sort_layer, target_index, source_frame_buffer->texture_index, g_renderer->quad, material_index);
    r_draw_single(key, model, uniform_data);
}

internal void
r_draw_batch_internal(Geometry* geometry, Material* material, uint64 element_count, Mat4* models, void* uniform_data)
{
    if (material->is_instanced)
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_SLOT_SSBO_CUSTOM, material->uniform_buffer_id);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_renderer->mvp_ssbo_id);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Mat4) * element_count, models);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, material->uniform_buffer_id);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, material->uniform_data_size * element_count, uniform_data);
        glDrawElementsInstanced(GL_TRIANGLES, geometry->index_count, GL_UNSIGNED_INT, 0, element_count);
        g_renderer->stat_draw_count++;
        g_renderer->stat_object_count += element_count;
    }
    else
    {
        glBindBuffer(GL_UNIFORM_BUFFER, material->uniform_buffer_id);
        glBindBufferRange(GL_UNIFORM_BUFFER, BINDING_SLOT_UBO_CUSTOM, material->uniform_buffer_id, 0, material->uniform_data_size);
        for (uint32 element_index = 0; element_index < element_count; element_index++)
        {
            Mat4  model       = models[element_index];
            void* shader_data = ((uint8*)uniform_data + element_index * material->uniform_data_size);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, material->uniform_data_size, shader_data);
            glUniformMatrix4fv(material->location_model, 1, GL_FALSE, model.v);
            glDrawElements(GL_TRIANGLES, geometry->index_count, GL_UNSIGNED_INT, 0);
            g_renderer->stat_draw_count++;
        }
        g_renderer->stat_object_count += element_count;
    }
}

internal void
camera_move(Renderer* renderer, Vec2 position)
{
    renderer->camera.inverse_view = mat4_translation(vec3_xy_z(position, 0));
    renderer->camera.view         = mat4_inverse(renderer->camera.inverse_view);
}

internal Vec3
camera_position(Renderer* renderer)
{
    return renderer->camera.inverse_view.columns[3].xyz;
}

internal Rect
camera_world_bounds(Camera camera)
{
    Vec2 camera_position = camera.inverse_view.columns[3].xy;
    return rect(camera_position.x, camera_position.y, camera.world_width, camera.world_height);
}

internal float32
px(float32 u)
{
    return _pixel_per_unit * u;
}

internal float32
em(float32 v)
{
    return px(v) * DEFAULT_FONT_SIZE;
}

/** helpers */
internal float32
screen_top()
{
    return g_renderer->camera.world_height / 2.0f;
}

internal float32
screen_left()
{
    return -g_renderer->camera.world_width / 2.0f;
}

internal float32
screen_right()
{
    return g_renderer->camera.world_width / 2.0f;
}

internal float32
screen_bottom()
{
    return -g_renderer->camera.world_height / 2.0f;
}

internal float32
screen_height()
{
    return g_renderer->camera.world_height;
}

internal float32
screen_width()
{
    return g_renderer->camera.world_width;
}

internal Rect
screen_rect()
{
    return rect(0, 0, screen_width(), screen_height());
}