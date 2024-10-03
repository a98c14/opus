#include "gfx_core_opengl.h"

global Arena*           _gfx_ogl_perm_arena  = 0;
global Arena*           _gfx_ogl_frame_arena = 0;
global GFX_OGL_Context* _gfx_ogl_ctx         = 0;

internal void
gfx_init(GFX_Configuration* configuration)
{
    xassert(configuration->world_width > 0 || configuration->world_height > 0, "at least one of world width or world height needs to have a value");

    _gfx_ogl_perm_arena  = arena_new_reserve(mb(64));
    _gfx_ogl_frame_arena = arena_new_reserve(mb(16));
    _gfx_ogl_ctx         = arena_push_struct_zero(_gfx_ogl_perm_arena, GFX_OGL_Context);

    _gfx_ogl_ctx->window_width  = configuration->window_width;
    _gfx_ogl_ctx->window_height = configuration->window_height;

    _gfx_ogl_ctx->frame_buffers = arena_push_array_zero(_gfx_ogl_perm_arena, GFX_OGL_FrameBuffer, _GFX_OGL_LAYER_CAPACITY);
    _gfx_ogl_ctx->materials     = arena_push_array_zero(_gfx_ogl_perm_arena, GFX_OGL_Material, _GFX_OGL_MATERIAL_CAPACITY);
    _gfx_ogl_ctx->textures      = arena_push_array_zero(_gfx_ogl_perm_arena, GFX_OGL_Texture, _GFX_OGL_TEXTURE_CAPACITY);

    glViewport(0, 0, _gfx_ogl_ctx->window_width, _gfx_ogl_ctx->window_height);

    _gfx_ogl_ctx->aspect = _gfx_ogl_ctx->window_width / (float)_gfx_ogl_ctx->window_height;
    float32 world_height = configuration->world_height;
    float32 world_width  = configuration->world_width;

    if (configuration->world_width == 0)
        world_width = world_height * _gfx_ogl_ctx->aspect;
    if (configuration->world_height == 0)
        world_height = world_width / _gfx_ogl_ctx->aspect;

    _gfx_ogl_ctx->world_width  = world_width;
    _gfx_ogl_ctx->world_height = world_height;
    _gfx_ogl_ctx->ppu          = 1.0f / (_gfx_ogl_ctx->window_width / world_width);

    /** Create Default Camera */
    GFX_Camera camera       = gfx_camera_new(world_width, world_height, 100, -100, _gfx_ogl_ctx->window_width, _gfx_ogl_ctx->window_height);
    _gfx_ogl_ctx->camera[0] = camera;

    glEnable(GL_BLEND);
    Vec4 clear_color = color_v4(configuration->clear_color);
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
    frame_buffer->texture_handle      = gfx_handle_zero();
    frame_buffer->gl_buffer_id        = 0;
    frame_buffer->clear_color         = clear_color;
    frame_buffer->width               = configuration->window_width;
    frame_buffer->height              = configuration->window_height;
    frame_buffer->blend_src_rgb       = GL_SRC_ALPHA;
    frame_buffer->blend_dst_rgb       = GL_ONE_MINUS_SRC_ALPHA;
    frame_buffer->blend_src_alpha     = GL_ONE;
    frame_buffer->blend_dst_alpha     = GL_ONE;

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
    log_debug("renderer created");
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
