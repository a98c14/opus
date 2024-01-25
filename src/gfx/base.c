#include "base.h"
#include <gfx/base.h>

internal Renderer*
renderer_new(Arena* arena, RendererConfiguration* configuration)
{
    Renderer* renderer      = arena_push_struct_zero_aligned(arena, Renderer, 16);
    renderer->arena         = arena;
    renderer->window_width  = configuration->window_width;
    renderer->window_height = configuration->window_height;
    renderer->frame_buffers = arena_push_array_zero_aligned(arena, FrameBuffer, LAYER_CAPACITY, 16);
    renderer->materials     = arena_push_array_zero(arena, Material, MATERIAL_CAPACITY);
    renderer->textures      = arena_push_array_zero(arena, Texture, TEXTURE_CAPACITY);
    renderer->geometries    = arena_push_array_zero(arena, Geometry, GEOMETRY_CAPACITY);

    xassert(configuration->world_width > 0 || configuration->world_height > 0, "at least one of world width or world height needs to have a value");
    glViewport(0, 0, renderer->window_width, renderer->window_height);

    renderer->aspect     = renderer->window_width / (float)renderer->window_height;
    float32 world_height = configuration->world_height;
    float32 world_width  = configuration->world_width;

    if (configuration->world_width == 0)
        world_width = world_height * renderer->aspect;
    if (configuration->world_height == 0)
        world_height = world_width / renderer->aspect;
    renderer->world_width  = world_width;
    renderer->world_height = world_height;
    renderer->ppu          = 1.0f / (renderer->window_width / world_width);
    renderer->camera       = camera_new(world_width, world_height, 100, -100, renderer->window_width, renderer->window_height);

    // TEMP: testing global variable solution out. Potentially dangerous?
    _pixel_per_unit = renderer->ppu;

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    Vec4 clear_color = color_to_vec4(configuration->clear_color);
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);

    /* Create Global UBO */
    glGenBuffers(1, &renderer->global_uniform_buffer_id);
    glBindBuffer(GL_UNIFORM_BUFFER, renderer->global_uniform_buffer_id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GlobalUniformData), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, BINDING_SLOT_GLOBAL, renderer->global_uniform_buffer_id, 0, sizeof(GlobalUniformData));

    /* Create Texture UBO */
    glGenBuffers(1, &renderer->texture_uniform_buffer_id);
    glBindBuffer(GL_UNIFORM_BUFFER, renderer->texture_uniform_buffer_id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(TextureUniformData), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, BINDING_SLOT_TEXTURE, renderer->texture_uniform_buffer_id, 0, sizeof(TextureUniformData));

    /* Create Camera UBO */
    glGenBuffers(1, &renderer->camera_uniform_buffer_id);
    glBindBuffer(GL_UNIFORM_BUFFER, renderer->camera_uniform_buffer_id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraUniformData), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, BINDING_SLOT_CAMERA, renderer->camera_uniform_buffer_id, 0, sizeof(CameraUniformData));

    /* Create MVP SSBO */
    glGenBuffers(1, &renderer->mvp_ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderer->mvp_ssbo_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Mat4) * MATERIAL_DRAW_BUFFER_ELEMENT_CAPACITY, 0, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_SLOT_SSBO_MODEL, renderer->mvp_ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    /* Draw State Setup */
    renderer->draw_state = renderer_draw_state_new(arena);

    // Reserve the first slot for NULL texture
    renderer->texture_count += 1;

    // Initialize default frame buffer
    renderer->frame_buffer_count += 1;
    FrameBuffer* frame_buffer   = &renderer->frame_buffers[FRAME_BUFFER_INDEX_DEFAULT];
    frame_buffer->texture_index = TEXTURE_INDEX_NULL;
    frame_buffer->buffer_id     = 0;
    frame_buffer->clear_color   = clear_color;
    frame_buffer->width         = configuration->window_width;
    frame_buffer->height        = configuration->window_height;
    log_debug("renderer created");
    return renderer;
}

internal RendererDrawState*
renderer_draw_state_new(Arena* arena)
{
    RendererDrawState* draw_state     = arena_push_struct_zero(arena, RendererDrawState);
    draw_state->material_draw_buffers = arena_push_array_zero_aligned(arena, MaterialDrawBuffer, MATERIAL_DRAW_BUFFER_CAPACITY, 16);
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
render_key_new(ViewType view_type, SortLayerIndex sort_layer, FrameBufferIndex layer, TextureIndex texture, GeometryIndex geometry, MaterialIndex material_index)
{
    xassert(view_type < 4, "invalid view_type value provided");
    RenderKey result = ((uint64)sort_layer << RenderKeySortLayerIndexBitStart) +
                       ((uint64)layer << RenderKeyFrameBufferIndexBitStart) +
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

internal MaterialDrawBuffer*
renderer_get_material_buffer(Renderer* renderer, RenderKey key, uint32 available_space)
{
    MaterialDrawBufferIndex initial_index = hash_uint64(key) % MATERIAL_DRAW_BUFFER_CAPACITY;
    MaterialDrawBuffer*     buffer        = NULL;
    for (int16 draw_buffer_index = initial_index; draw_buffer_index < initial_index + MATERIAL_DRAW_BUFFER_MAX_PROBE; draw_buffer_index++)
    {
        int32 probe_count = draw_buffer_index - initial_index;
        buffer            = &renderer->draw_state->material_draw_buffers[draw_buffer_index % MATERIAL_DRAW_BUFFER_CAPACITY];

        // if the buffer doesn't have enough space for the request, skip it
        if (buffer->key == key && buffer->element_count + available_space >= MATERIAL_DRAW_BUFFER_ELEMENT_CAPACITY)
            continue;

        // if target buffer is found, exit
        if (buffer->key == key)
        {
            renderer->stat_probe_count_max = max(renderer->stat_probe_count_max, probe_count);
            renderer->stat_probe_count_sum += probe_count;
            renderer->stat_probe_count++;
            break;
        }

        // if the slot is empty and we are still in probe range, initialize the buffer
        if (buffer->key == MATERIAL_DRAW_BUFFER_EMPTY_KEY)
        {
            renderer->stat_probe_count_max = max(renderer->stat_probe_count_max, probe_count);
            renderer->stat_probe_count_sum += probe_count;
            renderer->stat_probe_count++;

            MaterialIndex    material_index = render_key_mask(key, RenderKeyMaterialIndexBitStart, RenderKeyMaterialIndexBitCount);
            FrameBufferIndex layer          = render_key_mask(key, RenderKeyFrameBufferIndexBitStart, RenderKeyFrameBufferIndexBitCount);
            ViewType         view_type      = render_key_mask(key, RenderKeyViewTypeBitStart, RenderKeyViewTypeBitCount);
            SortLayerIndex   sort_layer     = render_key_mask(key, RenderKeySortLayerIndexBitStart, RenderKeySortLayerIndexBitCount);
            TextureIndex     texture        = render_key_mask(key, RenderKeyTextureIndexBitStart, RenderKeyTextureIndexBitCount);
            GeometryIndex    geometry       = render_key_mask(key, RenderKeyGeometryIndexBitStart, RenderKeyGeometryIndexBitCount);

            log_debug("initializing material buffer, sort:%d layer:%d view:%d texture:%d geometry:%d material:%d buffer_index:%03d instanced:%d", sort_layer, layer, view_type, texture, geometry, material_index, draw_buffer_index, renderer->materials[material_index].is_instanced);
            const Material* material = &renderer->materials[material_index];
            xassert(material->is_initialized, "material isn't initialized");
            buffer->index              = draw_buffer_index;
            buffer->key                = key;
            buffer->element_count      = 0;
            buffer->material_index     = material_index;
            buffer->model_buffer       = arena_push_array_zero_aligned(renderer->arena, Mat4, MATERIAL_DRAW_BUFFER_ELEMENT_CAPACITY, 16);
            buffer->shader_data_buffer = arena_push_zero_aligned(renderer->arena, MATERIAL_DRAW_BUFFER_ELEMENT_CAPACITY * material->uniform_data_size, 16);

            // find layer buffer
            int32 layer_buffer_index = -1;
            for (uint32 j = 0; j < renderer->draw_state->layer_count; j++)
            {
                FrameBufferIndex layer_index = renderer->draw_state->layer_draw_buffers[j].layer_index;
                if (layer_index == layer)
                {
                    layer_buffer_index = j;
                    break;
                }
            }

            if (layer_buffer_index == -1)
            {
                layer_buffer_index                                                       = renderer->draw_state->layer_count;
                renderer->draw_state->layer_draw_buffers[layer_buffer_index].layer_index = layer;
                renderer->draw_state->layer_count++;
                xassert(renderer->draw_state->layer_count < LAYER_CAPACITY, "[ERROR] Draw layer buffer index exceeded capacity!");
            }

            xassert(layer_buffer_index != -1, "[ERROR] LayerDrawBuffer could not be found");
            LayerDrawBuffer* layer_draw_buffer = &renderer->draw_state->layer_draw_buffers[layer_buffer_index];

            // find sort layer
            SortingLayerDrawBuffer* sort_layer_draw_buffer = &layer_draw_buffer->sorting_layer_draw_buffers[sort_layer];

            // find view buffer
            int32 view_buffer_index = -1;
            for (int j = 0; j < sort_layer_draw_buffer->view_count; j++)
            {
                ViewType current_view_type = sort_layer_draw_buffer->view_buffers[j].view_type;
                if (current_view_type == view_type)
                {
                    view_buffer_index = j;
                    break;
                }
            }

            if (view_buffer_index == -1)
            {
                view_buffer_index                                                 = sort_layer_draw_buffer->view_count;
                sort_layer_draw_buffer->view_buffers[view_buffer_index].view_type = view_type;
                sort_layer_draw_buffer->view_count++;
                xassert(sort_layer_draw_buffer->view_count <= ViewTypeCOUNT, "draw view buffer index exceeded capacity!");
            }

            ViewDrawBuffer* view_draw_buffer = &sort_layer_draw_buffer->view_buffers[view_buffer_index];
            xassert(view_buffer_index != -1, "`ViewDrawBuffer` could not be found");

            // find texture buffer
            int32 texture_buffer_index = -1;
            for (int j = 0; j < view_draw_buffer->texture_count; j++)
            {
                TextureIndex current_texture_index = view_draw_buffer->texture_draw_buffers[j].texture_index;
                if (current_texture_index == texture)
                {
                    texture_buffer_index = j;
                    break;
                }
            }

            if (texture_buffer_index == -1)
            {
                texture_buffer_index                                                       = view_draw_buffer->texture_count;
                view_draw_buffer->texture_draw_buffers[texture_buffer_index].texture_index = texture;
                view_draw_buffer->texture_count++;
                xassert(view_draw_buffer->texture_count < TEXTURE_CAPACITY, "draw texture buffer index exceeded capacity!");
            }

            TextureDrawBuffer* texture_draw_buffer = &view_draw_buffer->texture_draw_buffers[texture_buffer_index];

            // find geometry buffer
            int32 geometry_buffer_index = -1;
            for (int j = 0; j < texture_draw_buffer->geometry_count; j++)
            {
                GeometryIndex current_geometry_index = texture_draw_buffer->geometry_draw_buffers[j].geometry_index;
                if (current_geometry_index == geometry)
                {
                    geometry_buffer_index = j;
                    break;
                }
            }

            if (geometry_buffer_index == -1)
            {
                geometry_buffer_index                                                            = texture_draw_buffer->geometry_count;
                texture_draw_buffer->geometry_draw_buffers[geometry_buffer_index].geometry_index = geometry;
                texture_draw_buffer->geometry_count++;
                xassert(texture_draw_buffer->geometry_count < GEOMETRY_CAPACITY, "draw geometry buffer index exceeded capacity!");
            }

            GeometryDrawBuffer* geometry_draw_buffer = &texture_draw_buffer->geometry_draw_buffers[geometry_buffer_index];

            // add draw buffer index
            int32 setting_internal_index = geometry_draw_buffer->material_count;
            log_trace("draw buffer internal_index: %d", setting_internal_index);
            xassert(setting_internal_index < MATERIAL_DRAW_BUFFER_CAPACITY_PER_SETTING, "material internal index exceeded MATERIAL_CAPACITY");
            geometry_draw_buffer->material_buffer_indices[setting_internal_index];
            geometry_draw_buffer->material_buffer_indices[setting_internal_index] = draw_buffer_index;
            geometry_draw_buffer->material_count++;
            renderer->stat_initialized_buffer_count++;
            break;
        }
    }

    xassert(buffer && buffer->key == key, "`MaterialDrawBuffer` could not be found");
    return buffer;
}

/* queues N elements from underlying draw buffer and returns the addresses. If N is larger than `MATERIAL_DRAW_BUFFER_ELEMENT_CAPACITY` use `renderer_buffer_request_batched` instead */
internal DrawBuffer
renderer_buffer_request(Renderer* renderer, RenderKey key, uint32 count)
{
    xassert(count > 0, "requested render buffer can not have 0 size");
    xassert(count < MATERIAL_DRAW_BUFFER_ELEMENT_CAPACITY, "requested render buffer can not be larger than max element capacity, use `renderer_buffer_request_batched` instead");

    MaterialDrawBuffer* buffer         = renderer_get_material_buffer(renderer, key, count);
    MaterialIndex       material_index = render_key_mask(key, RenderKeyMaterialIndexBitStart, RenderKeyMaterialIndexBitCount);
    const Material*     material       = &renderer->materials[material_index];
    DrawBuffer          result         = {0};
    result.capacity                    = min(count, MATERIAL_DRAW_BUFFER_ELEMENT_CAPACITY - buffer->element_count);
    result.model_buffer                = buffer->model_buffer + buffer->element_count;
    result.uniform_data_buffer         = (uint8*)buffer->shader_data_buffer + (buffer->element_count * material->uniform_data_size);
    result.uniform_data_size           = material->uniform_data_size;
    buffer->element_count += count;
    xassert(result.capacity > 0, "material draw buffer is at maximum capacity, can not reserve more elements");
    return result;
}

/* queues N elements from underyling draw buffers in batches and returns the addresses. */
internal DrawBufferArray*
renderer_buffer_request_batched(Arena* arena, Renderer* renderer, RenderKey key, uint32 count)
{
    uint32           capacity = 1 + (count / MATERIAL_DRAW_BUFFER_ELEMENT_CAPACITY);
    DrawBufferArray* result   = arena_push_struct_zero(arena, DrawBufferArray);
    result->elements          = arena_push_array_zero_aligned(arena, DrawBuffer, capacity, 16);
    result->count             = 0;
    result->index             = 0;

    uint32 remaining = count;
    while (remaining > 0)
    {
        uint32 batch_size               = min(MATERIAL_DRAW_BUFFER_ELEMENT_CAPACITY - 1, remaining);
        result->elements[result->count] = renderer_buffer_request(renderer, key, count);
        remaining -= batch_size;
        result->count++;
    }
    return result;
}

internal void
renderer_buffer_queue_single(Renderer* renderer, RenderKey key, Vec3 position, Vec2 scale, void* uniform_data)
{
    DrawBuffer draw_buffer = renderer_buffer_request(renderer, key, 1);
    draw_buffer_insert(&draw_buffer, transform_quad_aligned(position, scale), &uniform_data);
}

internal bool32
draw_buffer_insert(DrawBuffer* draw_buffer, Mat4 model, void* uniform_data)
{
    draw_buffer->model_buffer[draw_buffer->index] = model;
    memcpy(((uint8*)draw_buffer->uniform_data_buffer) + draw_buffer->uniform_data_size * draw_buffer->index, uniform_data, draw_buffer->uniform_data_size);
    draw_buffer->index++;
    draw_buffer->capacity--;
    return draw_buffer->capacity > 0;
}

internal void
draw_buffer_array_insert(DrawBufferArray* draw_buffer_array, Mat4 model, void* uniform_data)
{
    DrawBuffer* draw_buffer = &draw_buffer_array->elements[draw_buffer_array->index];
    bool32      has_space   = draw_buffer_insert(draw_buffer, model, uniform_data);
    if (!has_space)
        draw_buffer_array->index++;
    xassert(draw_buffer_array->index <= draw_buffer_array->count, "draw buffer array insert exceeded available space");
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
                            Material*               material                   = &renderer->materials[material_draw_buffer->material_index];

                            glUseProgram(material->gl_program_id);
                            glUniform1i(material->location_texture, 0);
                            log_trace("rendering, sort: %2d, layer: %2d, view: %2d, texture: %2d, geometry: %2d, material: %2d, internal: %2d, draw_buffer: %03d", sort_layer_index, layer_draw_buffer->layer_index, view_draw_buffer->view_type, texture_draw_buffer->texture_index, geometry_draw_buffer->geometry_index, material_draw_buffer->material_index, internal_index, material_draw_buffer_index);

                            if (material->is_instanced)
                            {
                                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_SLOT_SSBO_CUSTOM, material->uniform_buffer_id);
                                glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderer->mvp_ssbo_id);
                                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Mat4) * material_draw_buffer->element_count, material_draw_buffer->model_buffer);
                                glBindBuffer(GL_SHADER_STORAGE_BUFFER, material->uniform_buffer_id);
                                glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, material->uniform_data_size * material_draw_buffer->element_count, material_draw_buffer->shader_data_buffer);
                                glDrawElementsInstanced(GL_TRIANGLES, geometry.index_count, GL_UNSIGNED_INT, 0, material_draw_buffer->element_count);
                                renderer->stat_draw_count++;
                                renderer->stat_object_count += material_draw_buffer->element_count;
                            }
                            else
                            {
                                glBindBuffer(GL_UNIFORM_BUFFER, material->uniform_buffer_id);
                                glBindBufferRange(GL_UNIFORM_BUFFER, BINDING_SLOT_UBO_CUSTOM, material->uniform_buffer_id, 0, material->uniform_data_size);
                                for (int element_index = 0; element_index < material_draw_buffer->element_count; element_index++)
                                {
                                    Mat4  model       = material_draw_buffer->model_buffer[element_index];
                                    void* shader_data = ((uint8*)material_draw_buffer->shader_data_buffer + element_index * material->uniform_data_size);
                                    glBufferSubData(GL_UNIFORM_BUFFER, 0, material->uniform_data_size, shader_data);
                                    glUniformMatrix4fv(material->location_model, 1, GL_FALSE, model.v);
                                    glDrawElements(GL_TRIANGLES, geometry.index_count, GL_UNSIGNED_INT, 0);
                                    renderer->stat_draw_count++;
                                }
                                renderer->stat_object_count += material_draw_buffer->element_count;
                            }
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
texture_shader_data_set(Renderer* renderer, const Texture* texture)
{
    TextureUniformData shader_data = {0};
    shader_data.layer_count        = texture->layer_count;
    shader_data.size               = (Vec2){.x = (float32)texture->width, .y = (float32)texture->height};
    glBindBuffer(GL_UNIFORM_BUFFER, renderer->texture_uniform_buffer_id);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(TextureUniformData), &shader_data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
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