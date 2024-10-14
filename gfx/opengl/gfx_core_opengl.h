#pragma once
#include <glad/gl.h>
#include "../gfx_core.h"
#include "../gfx_utils.h"
#include "./gfx_primitives_opengl.h"

#define _GFX_OGL_MATERIAL_CAPACITY                         32
#define _GFX_OGL_TEXTURE_CAPACITY                          32
#define _GFX_OGL_GEOMETRY_CAPACITY                         32
#define _GFX_OGL_LAYER_CAPACITY                            16
#define _GFX_OGL_PASS_CAPACITY                             16
#define _GFX_OGL_SORTING_LAYER_CAPACITY                    16
#define _GFX_OGL_MATERIAL_DRAW_BUFFER_CAPACITY_PER_SETTING (16)
#define _GFX_OGL_MATERIAL_DRAW_BUFFER_CAPACITY             (1024)
#define _GFX_OGL_MATERIAL_DRAW_BUFFER_ELEMENT_CAPACITY     (8192 * 2)
#define _GFX_OGL_GFX_TRAIL_MAX_VERTEX_CAPACITY             (256 * 3)

#define _GFX_OGL_BINDING_SLOT_GLOBAL      0
#define _GFX_OGL_BINDING_SLOT_TEXTURE     1
#define _GFX_OGL_BINDING_SLOT_UBO_CUSTOM  2
#define _GFX_OGL_BINDING_SLOT_SSBO_CUSTOM 2

#define _GFX_OGL_TEXTURE_INDEX_NULL         0
#define _GFX_OGL_FRAME_BUFFER_INDEX_DEFAULT 0
#define _GFX_OGL_DEFAULT_FONT_SIZE          16.0f

typedef enum GFX_OGL_EntityKind
{
    GFX_OGL_EntityKind_Null,
} GFX_OGL_EntityKind;

typedef struct
{
    bool32       is_initialized;
    GFX_DrawType draw_type;

    uint32 gl_program_id;
    uint32 uniform_buffer_id;
    uint32 uniform_data_size;

    uint32 location_model;
    uint32 location_uniform;
    uint32 location_texture;
} GFX_OGL_Material;

typedef struct
{
    Vec4         clear_color;
    uint32       width;
    uint32       height;
    uint32       gl_buffer_id;
    TextureIndex texture_index;

    uint32 blend_src_rgb;
    uint32 blend_dst_rgb;
    uint32 blend_src_alpha;
    uint32 blend_dst_alpha;
} GFX_OGL_FrameBuffer;

typedef struct
{
    int32  width;
    int32  height;
    int32  channels;
    int32  format;
    int32  layer_count;
    uint32 gl_texture_id;
    uint32 gl_texture_type;
} GFX_OGL_Texture;

typedef struct
{
    Mat4    projection;
    Mat4    view;
    float32 time;
    Vec3    _;
} GFX_OGL_GlobalUniformData;

typedef struct
{
    Vec2    size;
    float32 layer_count;
    float32 _;
} GFX_OGL_TextureUniformData;

enum GFX_OGL_ShaderProgramType
{
    GFX_OGL_ShaderProgramTypeVertex   = GL_VERTEX_SHADER,
    GFX_OGL_ShaderProgramTypeFragment = GL_FRAGMENT_SHADER
};

typedef struct
{
    uint32  window_width;
    uint32  window_height;
    float32 world_width;
    float32 world_height;

    bool32  debug;
    float32 timer;
    float32 ppu;
    float32 aspect;

    /* draw state */
    RenderKey  active_key;
    GFX_Camera camera[8];

    uint32     pass_count;
    GFX_Pass*  passes;
    GFX_Batch* previous_batch;
    GFX_Batch* active_batch;

    /* resources */
    uint8                frame_buffer_count;
    GFX_OGL_FrameBuffer* frame_buffers;
    uint8                texture_count;
    GFX_OGL_Texture*     textures;
    uint8                material_count;
    GFX_OGL_Material*    materials;

    uint32 global_uniform_buffer_id;
    uint32 texture_uniform_buffer_id;
    uint32 vao_quad;
    uint32 vao_triangle;
    uint32 vao_dynamic;

    /* stats */
    int32   stat_draw_count;
    int32   stat_object_count;
    int16   stat_initialized_buffer_count;
    int32   stat_probe_count_max;
    int64   stat_probe_count;
    float32 stat_probe_count_sum;
} GFX_OGL_Context;

internal void _gfx_attribute_info_add(GFX_VertexAttributeInfo* info, usize component_size, uint32 component_count, GLenum type);

/** Frame Buffer Controls */
internal void _gfx_ogl_frame_buffer_begin(GFX_OGL_FrameBuffer* frame_buffer);

/** Shader */
internal uint32 _gfx_ogl_shader_load(String vertex_shader_str, String fragment_shader_str);
internal void   _gfx_ogl_shader_set_texture(GFX_OGL_Texture* texture);

/** Vertex Attribute */
internal void _gfx_ogl_attribute_info_add(GFX_VertexAttributeInfo* info, uint32 component_size, uint32 component_count, GLenum type);