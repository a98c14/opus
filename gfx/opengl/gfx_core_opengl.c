#include "gfx_core_opengl.h"

global Arena*           _gfx_ogl_perm_arena  = 0;
global Arena*           _gfx_ogl_frame_arena = 0;
global GFX_OGL_Context* _gfx_ogl_ctx         = 0;

internal void
gfx_init(GFX_Configuration configuration)
{
    if (!os_window_is_ready())
    {
        log_error("Could initialize GFX layer, Main Window isn't ready. Call `os_window_create` before initializing GFX layer.");
        return;
    }

    log_info("Material Capacity: %d\n\tTexture Capacity: %d\n\tGeometry Capacity: %d\n\tPass Capacity: %d\n\tSorting Layer Capacity: %d", _GFX_OGL_MATERIAL_CAPACITY, _GFX_OGL_TEXTURE_CAPACITY, _GFX_OGL_GEOMETRY_CAPACITY, _GFX_OGL_PASS_CAPACITY, _GFX_OGL_SORTING_LAYER_CAPACITY);
    log_info("Render Key Total Bit Count: (%d/64)", RenderKeyTotalBitCount);

    IVec2 window_size = os_window_size();
    if (configuration.world_width == 0 && configuration.world_height == 0)
    {
        configuration.world_width  = (float32)window_size.x;
        configuration.world_height = (float32)window_size.y;
    }

    if (configuration.window_width == 0 && configuration.window_height == 0)
    {
        configuration.window_width  = window_size.x;
        configuration.window_height = window_size.y;
    }
#if BUILD_DEBUG
    gfx_enable_debug();
#endif

    _gfx_ogl_perm_arena  = arena_new_reserve(mb(64));
    _gfx_ogl_frame_arena = arena_new_reserve(mb(16));
    _gfx_ogl_ctx         = arena_push_struct_zero(_gfx_ogl_perm_arena, GFX_OGL_Context);
    _gfx_ogl_ctx->aspect = _gfx_ogl_ctx->window_width / (float)_gfx_ogl_ctx->window_height;

    if (configuration.window_width == 0)
        configuration.window_width = (int32)(configuration.window_height * _gfx_ogl_ctx->aspect);
    if (configuration.window_height == 0)
        configuration.window_height = (int32)(configuration.window_width / _gfx_ogl_ctx->aspect);

    _gfx_ogl_ctx->window_width  = configuration.window_width;
    _gfx_ogl_ctx->window_height = configuration.window_height;
    _gfx_ogl_ctx->frame_buffers = arena_push_array_zero(_gfx_ogl_perm_arena, GFX_OGL_FrameBuffer, _GFX_OGL_PASS_CAPACITY);
    _gfx_ogl_ctx->materials     = arena_push_array_zero(_gfx_ogl_perm_arena, GFX_OGL_Material, _GFX_OGL_MATERIAL_CAPACITY);
    _gfx_ogl_ctx->textures      = arena_push_array_zero(_gfx_ogl_perm_arena, GFX_OGL_Texture, _GFX_OGL_TEXTURE_CAPACITY);

    glViewport(0, 0, _gfx_ogl_ctx->window_width, _gfx_ogl_ctx->window_height);

    float32 world_height = configuration.world_height;
    float32 world_width  = configuration.world_width;

    if (configuration.world_width == 0)
        world_width = world_height * _gfx_ogl_ctx->aspect;
    if (configuration.world_height == 0)
        world_height = world_width / _gfx_ogl_ctx->aspect;

    _gfx_ogl_ctx->world_width  = world_width;
    _gfx_ogl_ctx->world_height = world_height;
    _gfx_ogl_ctx->ppu          = 1.0f / (_gfx_ogl_ctx->window_width / world_width);

    /** Create Default Camera */
    GFX_Camera camera       = gfx_camera_new(world_width, world_height, 100, -100, _gfx_ogl_ctx->window_width, _gfx_ogl_ctx->window_height);
    _gfx_ogl_ctx->camera[0] = camera;

    glEnable(GL_BLEND);
    Vec4 clear_color = color_v4(configuration.clear_color);
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);

    /* Create Global UBO */
    glGenBuffers(1, &_gfx_ogl_ctx->global_uniform_buffer_id);
    glBindBuffer(GL_UNIFORM_BUFFER, _gfx_ogl_ctx->global_uniform_buffer_id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GFX_OGL_GlobalUniformData), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, _GFX_OGL_BINDING_SLOT_GLOBAL, _gfx_ogl_ctx->global_uniform_buffer_id, 0, sizeof(GFX_OGL_GlobalUniformData));

    /* Create Texture UBO */
    glGenBuffers(1, &_gfx_ogl_ctx->texture_uniform_buffer_id);
    glBindBuffer(GL_UNIFORM_BUFFER, _gfx_ogl_ctx->texture_uniform_buffer_id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GFX_OGL_TextureUniformData), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, _GFX_OGL_BINDING_SLOT_TEXTURE, _gfx_ogl_ctx->texture_uniform_buffer_id, 0, sizeof(GFX_OGL_TextureUniformData));

    // Reserve the first slot for NULL texture
    _gfx_ogl_ctx->texture_count += 1;

    // Reserver the first slot for default frame buffer
    _gfx_ogl_ctx->frame_buffer_count += 1;

    GFX_OGL_FrameBuffer* frame_buffer = &_gfx_ogl_ctx->frame_buffers[_GFX_OGL_FRAME_BUFFER_INDEX_DEFAULT];
    frame_buffer->texture_index       = 0;
    frame_buffer->gl_buffer_id        = 0;
    frame_buffer->clear_color         = clear_color;
    frame_buffer->width               = configuration.window_width;
    frame_buffer->height              = configuration.window_height;
    frame_buffer->blend_src_rgb       = GL_SRC_ALPHA;
    frame_buffer->blend_dst_rgb       = GL_ONE_MINUS_SRC_ALPHA;
    frame_buffer->blend_src_alpha     = GL_ONE;
    frame_buffer->blend_dst_alpha     = GL_ONE;

    _gfx_ogl_ctx->material_count = 1;
    _gfx_ogl_ctx->pass_count     = 1;
    _gfx_ogl_ctx->passes         = arena_push_array_zero(_gfx_ogl_perm_arena, GFX_Pass, _GFX_OGL_SORTING_LAYER_CAPACITY);

    // default frame index
    _gfx_ogl_ctx->passes[0].batch_groups = arena_push_array_zero(_gfx_ogl_perm_arena, GFX_BatchGroup, _GFX_OGL_SORTING_LAYER_CAPACITY);

    // Init meshes
    ArenaTemp temp = scratch_begin(&_gfx_ogl_perm_arena, 1);

    GFX_VertexAttributeInfo* attr_info_color = gfx_attribute_info_new(temp.arena);
    gfx_attribute_info_add_vec2(attr_info_color); // layout(location = 0) in vec2 a_pos;
    gfx_attribute_info_add_vec2(attr_info_color); // layout(location = 1) in vec2 a_tex_coord;
    gfx_attribute_info_add_vec4(attr_info_color); // layout(location = 2) in vec4 a_color;

    _gfx_ogl_ctx->vao_quad     = _gfx_ogl_mesh_quad_create();
    _gfx_ogl_ctx->vao_triangle = _gfx_ogl_mesh_triangle_create();
    _gfx_ogl_ctx->vao_dynamic  = _gfx_ogl_mesh_buffer_create(attr_info_color);
    scratch_end(temp);
    log_debug("Renderer created.");
}

internal void
gfx_batch_commit(GFX_Batch batch)
{
    GFX_BatchNode* node = arena_push_struct_zero(_gfx_ogl_frame_arena, GFX_BatchNode);
    node->v             = batch;

    RenderKey       key              = node->v.key;
    PassIndex       pass_index       = gfx_render_key_mask(key, RenderKeyPassIndexBitStart, RenderKeyPassIndexBitCount);
    SortLayerIndex  sort_layer_index = gfx_render_key_mask(key, RenderKeySortLayerIndexBitStart, RenderKeySortLayerIndexBitCount);
    GFX_Pass*       pass             = &_gfx_ogl_ctx->passes[pass_index];
    GFX_BatchGroup* batch_group      = &pass->batch_groups[sort_layer_index];
    batch_group->batch_count++;
    queue_push(batch_group->first, batch_group->last, node);

    pass->is_empty = false;
}

/** Vertex Attribute Configuration */
internal void
_gfx_ogl_attribute_info_add(GFX_VertexAttributeInfo* info, uint32 component_size, uint32 component_count, GLenum type)
{
    GFX_VertexAttributeElementNode* n = arena_push_struct_zero(info->arena, GFX_VertexAttributeElementNode);
    n->v.component_count              = component_count;
    n->v.size                         = component_count * component_size;
    n->v.index                        = info->attribute_count;
    n->v.type                         = type;
    dll_push_back(info->first, info->last, n);
    info->attribute_count++;
}

internal GFX_VertexAttributeInfo*
gfx_attribute_info_new(Arena* arena)
{
    GFX_VertexAttributeInfo* result = arena_push_struct_zero(arena, GFX_VertexAttributeInfo);
    result->arena                   = arena;
    return result;
}

internal void
gfx_attribute_info_add_vec2(GFX_VertexAttributeInfo* info)
{
    _gfx_ogl_attribute_info_add(info, sizeof(float32), 2, GL_FLOAT);
}

internal void
gfx_attribute_info_add_vec4(GFX_VertexAttributeInfo* info)
{
    _gfx_ogl_attribute_info_add(info, sizeof(float32), 4, GL_FLOAT);
}

internal void
gfx_attribute_info_add_int(GFX_VertexAttributeInfo* info)
{
    _gfx_ogl_attribute_info_add(info, sizeof(float32), 1, GL_INT);
}

internal void
gfx_attribute_info_add_uint(GFX_VertexAttributeInfo* info)
{
    _gfx_ogl_attribute_info_add(info, sizeof(float32), 1, GL_UNSIGNED_INT);
}

/** helpers */
internal float32
px(float32 u)
{
    return _gfx_ogl_ctx->ppu * u;
}

internal float32
unit(float32 u)
{
    return u / _gfx_ogl_ctx->ppu;
}

internal float32
em(float32 v)
{
    return px(v) * _GFX_OGL_DEFAULT_FONT_SIZE;
}

internal float32
screen_top(void)
{
    return _gfx_ogl_ctx->camera[0].world_height / 2.0f;
}

internal float32
screen_left(void)
{
    return -_gfx_ogl_ctx->camera[0].world_width / 2.0f;
}

internal float32
screen_right(void)
{
    return _gfx_ogl_ctx->camera[0].world_width / 2.0f;
}

internal float32
screen_bottom(void)
{
    return -_gfx_ogl_ctx->camera[0].world_height / 2.0f;
}

internal float32
screen_height(void)
{
    return _gfx_ogl_ctx->camera[0].world_height;
}

internal float32
screen_width(void)
{
    return _gfx_ogl_ctx->camera[0].world_width;
}

internal Rect
screen_rect(void)
{
    return rect_from_xy_wh(0, 0, screen_width(), screen_height());
}

internal MaterialIndex
gfx_material_new(String vertex_shader_text, String fragment_shader_text, uint32 uniform_data_size, GFX_DrawType draw_type)
{
    GFX_OGL_Context* ctx            = _gfx_ogl_ctx;
    MaterialIndex    material_index = ctx->material_count;
    ctx->material_count++;

    GFX_OGL_Material* result  = &ctx->materials[material_index];
    result->gl_program_id     = _gfx_ogl_shader_load(vertex_shader_text, fragment_shader_text);
    result->location_texture  = glGetUniformLocation(result->gl_program_id, "u_main_texture");
    result->uniform_data_size = uniform_data_size;
    result->is_initialized    = 1;
    result->draw_type         = draw_type;

    // generate custom shader data UBO
    glGenBuffers(1, &result->uniform_buffer_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, result->uniform_buffer_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, uniform_data_size * _GFX_OGL_MATERIAL_DRAW_BUFFER_ELEMENT_CAPACITY, 0, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    result->location_model = glGetUniformLocation(result->gl_program_id, "u_model");

    unsigned int global_ubo_index = glGetUniformBlockIndex(result->gl_program_id, "Global");
    glUniformBlockBinding(result->gl_program_id, global_ubo_index, _GFX_OGL_BINDING_SLOT_GLOBAL);

    unsigned int texture_ubo_index = glGetUniformBlockIndex(result->gl_program_id, "Texture");
    glUniformBlockBinding(result->gl_program_id, texture_ubo_index, _GFX_OGL_BINDING_SLOT_TEXTURE);

    return material_index;
}

internal TextureIndex
gfx_texture_new(uint32 width, uint32 height, uint32 channels, uint32 filter, void* data)
{
    GFX_OGL_Context* ctx           = _gfx_ogl_ctx;
    TextureIndex     texture_index = ctx->texture_count;
    ctx->texture_count++;
    log_info("Texture Count: %d", texture_index);
    GFX_OGL_Texture* texture = &ctx->textures[texture_index];
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture->gl_texture_id);
    glBindTexture(GL_TEXTURE_2D, texture->gl_texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
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
        // TODO(selim): Why do we need to put this? Also should be enabled/disabled from function parameters
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        texture->format = GL_RED;
        break;
    default:
        texture->format = GL_RGBA;
        break;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, texture->format, width, height, 0, texture->format, GL_UNSIGNED_BYTE, data);
    return texture_index;
}

internal IVec2
gfx_texture_dims(TextureIndex texture)
{
    IVec2 result = {0};
    result.x     = _gfx_ogl_ctx->textures[texture].width;
    result.y     = _gfx_ogl_ctx->textures[texture].height;
    return result;
}

internal TextureIndex gfx_texture_array_new(uint32 width, uint32 height, uint32 channels, uint32 filter, uint32 layer_count, TextureData* data);
internal uint32       gfx_shader_load(String vertex_shader_text, String fragment_shader_text);

internal FrameBufferIndex
gfx_frame_buffer_new(uint32 width, uint32 height, uint32 filter, Color clear_color)
{
    TextureIndex texture_index = gfx_texture_new(width, height, 4, filter, NULL);

    uint32 fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _gfx_ogl_ctx->textures[texture_index].gl_texture_id, 0);

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
    FrameBufferIndex     layer_index  = _gfx_ogl_ctx->frame_buffer_count;
    GFX_OGL_FrameBuffer* frame_buffer = &_gfx_ogl_ctx->frame_buffers[layer_index];
    frame_buffer->gl_buffer_id        = fbo;
    frame_buffer->texture_index       = texture_index;
    frame_buffer->width               = width;
    frame_buffer->height              = height;
    frame_buffer->clear_color         = color_v4(clear_color);

    frame_buffer->blend_src_rgb   = GL_SRC_ALPHA;
    frame_buffer->blend_dst_rgb   = GL_ONE_MINUS_SRC_ALPHA;
    frame_buffer->blend_src_alpha = GL_ONE;
    frame_buffer->blend_dst_alpha = GL_ONE;
    _gfx_ogl_ctx->frame_buffer_count++;

    return layer_index;
}

internal uint64
gfx_material_uniform_data_size(MaterialIndex material)
{
    return _gfx_ogl_ctx->materials[material].uniform_data_size;
}

internal void
_gfx_ogl_frame_buffer_begin(GFX_OGL_FrameBuffer* frame_buffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer->gl_buffer_id);
    glViewport(0, 0, frame_buffer->width, frame_buffer->height);
    glBlendFuncSeparate(frame_buffer->blend_src_rgb, frame_buffer->blend_dst_rgb, frame_buffer->blend_src_alpha, frame_buffer->blend_dst_alpha);
    glClearColor(frame_buffer->clear_color.x, frame_buffer->clear_color.y, frame_buffer->clear_color.z, frame_buffer->clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
};

internal uint32
_gfx_ogl_shader_load(String vertex_shader_str, String fragment_shader_str)
{
    GLuint vertex_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const char* const*)&vertex_shader_str.value, NULL);
    glCompileShader(vertex_shader);

    GLuint fragment_shader;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const char* const*)&fragment_shader_str.value, NULL);
    glCompileShader(fragment_shader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

internal void
_gfx_ogl_shader_set_texture(GFX_OGL_Texture* texture)
{
    GFX_OGL_TextureUniformData shader_data = {0};
    shader_data.layer_count                = (float32)texture->layer_count;
    shader_data.size                       = (Vec2){.x = (float32)texture->width, .y = (float32)texture->height};
    glBindBuffer(GL_UNIFORM_BUFFER, _gfx_ogl_ctx->texture_uniform_buffer_id);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GFX_OGL_TextureUniformData), &shader_data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

internal void
gfx_render(float32 dt)
{
    GFX_OGL_Context* ctx = _gfx_ogl_ctx;
    xassert(ctx->pass_count > 0, "At least ONE render pass must be configured!");
    GFX_Camera camera = ctx->camera[0];
    ctx->timer += dt;
    ctx->stat_draw_count   = 0;
    ctx->stat_object_count = 0;

    /* setup global shader data */
    Mat4 world_view  = mat4_mvp(mat4_identity(), camera.view, camera.projection);
    Mat4 screen_view = mat4_mvp(mat4_identity(), mat4_identity(), camera.projection);

    GFX_OGL_GlobalUniformData global_shader_data = {0};
    global_shader_data.time                      = ctx->timer;
    global_shader_data.view                      = camera.view;
    global_shader_data.projection                = camera.projection;
    glBindBuffer(GL_UNIFORM_BUFFER, ctx->global_uniform_buffer_id);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GFX_OGL_GlobalUniformData), &global_shader_data);

#if BUILD_DEBUG == 1
    if (ctx->debug)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
#endif

    for (uint32 i = 0; i < ctx->pass_count; i++)
    {
        GFX_Pass* pass = &ctx->passes[i];
        if (pass->is_empty)
            continue;

        GFX_OGL_FrameBuffer* frame_buffer = &ctx->frame_buffers[pass->frame_buffer];
        _gfx_ogl_frame_buffer_begin(frame_buffer);

        for (uint32 j = 0; j < _GFX_OGL_SORTING_LAYER_CAPACITY; j++)
        {
            GFX_BatchGroup* batch_list = &pass->batch_groups[j];
            if (batch_list->batch_count <= 0)
                continue;

            GFX_BatchNode* batch_node;
            for_each(batch_node, batch_list->first)
            {
                GFX_Batch batch = batch_node->v;
                uint64    diff  = ctx->active_key ^ batch.key;
                ctx->active_key = batch.key;

                /** set texture */
                TextureIndex texture_index = (TextureIndex)gfx_render_key_mask(batch.key, RenderKeyTextureIndexBitStart, RenderKeyTextureIndexBitCount);
                if (gfx_render_key_mask(diff, RenderKeyTextureIndexBitStart, RenderKeyTextureIndexBitCount) > 0 && texture_index > 0)
                {
                    GFX_OGL_Texture* texture = &ctx->textures[texture_index];
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(texture->gl_texture_type, texture->gl_texture_id);
                    _gfx_ogl_shader_set_texture(texture);
                }

                /** set view matrix */
                GFX_ViewType view_type = (GFX_ViewType)gfx_render_key_mask(batch.key, RenderKeyViewTypeBitStart, RenderKeyViewTypeBitCount);
                Mat4         view      = view_type == GFX_ViewTypeWorld ? world_view : screen_view;

                /** set material */
                MaterialIndex     material_index = (MaterialIndex)gfx_render_key_mask(batch.key, RenderKeyMaterialIndexBitStart, RenderKeyMaterialIndexBitCount);
                GFX_OGL_Material* material       = &ctx->materials[material_index];
                if (gfx_render_key_mask(diff, RenderKeyMaterialIndexBitStart, RenderKeyMaterialIndexBitCount) > 0)
                {
                    glUseProgram(material->gl_program_id);
                    glUniform1i(material->location_texture, 0);
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, _GFX_OGL_BINDING_SLOT_SSBO_CUSTOM, material->uniform_buffer_id);
                }

                GFX_MeshType mesh_type = (GFX_MeshType)gfx_render_key_mask(batch.key, RenderKeyMeshTypeBitStart, RenderKeyMeshTypeBitCount);
                switch (mesh_type)
                {
                case GFX_MeshTypeQuad:
                {
                    batch.draw_instance_count = array_count(shape_indices_quad);
                    glBindVertexArray(ctx->vao_quad);
                    break;
                }
                case GFX_MeshTypeTriangle:
                {
                    batch.draw_instance_count = array_count(shape_indices_triangle);
                    glBindVertexArray(ctx->vao_triangle);
                    break;
                }
                case GFX_MeshTypeDynamic:
                {
                    glBindVertexArray(ctx->vao_dynamic);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, batch.vertex_buffer_size, batch.vertex_buffer);
                    break;
                }
                default:
                    not_implemented();
                }

                log_trace("rendering, sort: %2d, layer: %2d, view: %2d, texture: %2d, geometry: %2d, mesh type: %2d, material: %2d", i, pass->frame_buffer, view_type, texture_index, geometry_index, mesh_type, material_index);

                /** draw */
                glUniformMatrix4fv(material->location_model, 1, GL_FALSE, view.v);
                switch (material->draw_type)
                {
                case GFX_DrawTypePackedBuffer:
                {
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, material->uniform_buffer_id);
                    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, material->uniform_data_size * batch.element_count, batch.uniform_buffer);
                    glDrawArrays(GL_TRIANGLES, 0, batch.draw_instance_count);
                    ctx->stat_draw_count++;
                    ctx->stat_object_count += batch.element_count;
                    break;
                }
                case GFX_DrawTypeInstanced:
                {
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, material->uniform_buffer_id);
                    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, material->uniform_data_size * batch.element_count, batch.uniform_buffer);
                    glDrawElementsInstanced(GL_TRIANGLES, batch.draw_instance_count, GL_UNSIGNED_INT, 0, batch.element_count);

                    // TODO(selim): Use persistently mapped buffers instead of synchronizing manually like this.
                    // reference:
                    // https://www.khronos.org/opengl/wiki/Synchronization#Implicit_synchronization
                    // https://www.youtube.com/watch?v=-bCeNzgiJ8I
                    glFlush();
                    ctx->stat_draw_count++;
                    ctx->stat_object_count += batch.element_count;
                    break;
                }
                case GFX_DrawTypeSingle:
                {
                    glBindBuffer(GL_UNIFORM_BUFFER, material->uniform_buffer_id);
                    glBindBufferRange(GL_UNIFORM_BUFFER, _GFX_OGL_BINDING_SLOT_UBO_CUSTOM, material->uniform_buffer_id, 0, material->uniform_data_size);
                    for (uint32 element_index = 0; element_index < batch.element_count; element_index++)
                    {
                        glBufferSubData(GL_UNIFORM_BUFFER, 0, material->uniform_data_size, ((uint8*)batch.uniform_buffer + element_index * material->uniform_data_size));
                        glDrawElements(GL_TRIANGLES, batch.draw_instance_count, GL_UNSIGNED_INT, 0);
                        ctx->stat_draw_count++;
                    }
                    ctx->stat_object_count += 1;
                    break;
                }
                default:
                    not_implemented();
                }
            }

            // NOTE(selim): if we don't zero the list it keeps the old references in the next frame
            memory_zero_struct(batch_list);
        }
    }

    arena_reset(_gfx_ogl_frame_arena);
    ctx->previous_batch = 0;
    ctx->active_batch   = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

internal PassIndex
gfx_pipeline_add_pass(FrameBufferIndex frame_buffer)
{
    uint32 pass_index                             = _gfx_ogl_ctx->pass_count;
    _gfx_ogl_ctx->passes[pass_index].frame_buffer = frame_buffer;
    _gfx_ogl_ctx->passes[pass_index].batch_groups = arena_push_array_zero(_gfx_ogl_perm_arena, GFX_BatchGroup, _GFX_OGL_SORTING_LAYER_CAPACITY);
    _gfx_ogl_ctx->pass_count++;
}

/** Utility */
internal Vec2
gfx_window_to_screen_position(Vec2 p)
{
    Vec2    result = {0};
    float32 x      = (p.x / _gfx_ogl_ctx->window_width) - 0.5f;
    float32 y      = (p.y / _gfx_ogl_ctx->window_height) - 0.5f;
    x *= _gfx_ogl_ctx->world_width;
    y *= _gfx_ogl_ctx->world_height;

    result.x = x;
    result.y = y;
    return result;
}

internal Vec2
gfx_window_to_world_position(Vec2 p)
{
    Vec2    result = {0};
    float32 x      = (p.x / _gfx_ogl_ctx->window_width) - 0.5f;
    float32 y      = (p.y / _gfx_ogl_ctx->window_height) - 0.5f;
    x *= _gfx_ogl_ctx->world_width;
    y *= _gfx_ogl_ctx->world_height;

    // TODO(selim): This is a temporary solution. We need to use the camera's view matrix to convert the screen position to world position.
    // GFX_Camera camera     = _gfx_ogl_ctx->camera[0];
    // Mat4       world_view = mat4_mvp(mat4_identity(), camera.view, camera.projection);
    Vec4 world_pos = vec4(x, y, 0, 0);
    // world_pos             = mul_mat4_vec4(world_view, world_pos);

    result.x = world_pos.x;
    result.y = world_pos.y;
    return result;
}

/** Debug */
internal void
gfx_enable_debug(void)
{
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(_gfx_ogl_debug_message_callback, 0);
}

internal void
_gfx_ogl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
{
    (void)id;
    (void)length;
    (void)user_param;
    char* source_str;
    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        source_str = "API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        source_str = "WINDOW SYSTEM";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        source_str = "SHADER COMPILER";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        source_str = "THIRD PARTY";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        source_str = "APPLICATION";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        source_str = "OTHER";
        break;
    default:
        source_str = "NO_SOURCE";
        break;
    }

    char* type_str;
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        type_str = "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        type_str = "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        type_str = "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        type_str = "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        type_str = "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_MARKER:
        type_str = "MARKER";
        break;
    case GL_DEBUG_TYPE_OTHER:
        type_str = "OTHER";
        break;
    default:
        type_str = "NO_TYPE";
        break;
    }

    char* severity_str;
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        return; // severity_str = "NOTIFICATION"; break;
    case GL_DEBUG_SEVERITY_LOW:
        severity_str = "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        severity_str = "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        severity_str = "HIGH";
        break;
    default:
        severity_str = "NO_SEVERITY";
        break;
    }

    printf("[GL_%s][%s|%s] %s\n", severity_str, source_str, type_str, message);
}