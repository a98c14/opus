#pragma once
#include <base.h>
#include <glad/gl.h>

#include "base_math.h"

/* Constants */
#define MATERIAL_CAPACITY      32
#define TEXTURE_CAPACITY       32
#define GEOMETRY_CAPACITY      32
#define LAYER_CAPACITY         16
#define PASS_CAPACITY          16
#define SORTING_LAYER_CAPACITY 16

#define MATERIAL_DRAW_BUFFER_CAPACITY_PER_SETTING (16)
#define MATERIAL_DRAW_BUFFER_CAPACITY             (1024)
#define MATERIAL_DRAW_BUFFER_ELEMENT_CAPACITY     (8192 * 2)

#define GFX_TRAIL_MAX_VERTEX_CAPACITY (256 * 3)

// TEMP: testing global variable solution out. Potentially dangerous?
global float32 _pixel_per_unit = 1;
#define DEFAULT_FONT_SIZE 18

// TODO(selim): Make `Renderer` global

enum
{
    ViewTypeWorld,
    ViewTypeScreen,
    ViewTypeCOUNT,
};

enum
{
    MeshTypeTriangle,
    MeshTypeQuad,
    MeshTypeDynamic,
    MeshTypeCOUNT,
};

typedef enum
{
    R_DrawTypeSingle       = 0,
    R_DrawTypeInstanced    = 1,
    R_DrawTypePackedBuffer = 2,
} DrawType;

typedef int64 RenderKey;
#define RenderKeyMaterialIndexBitCount  8
#define RenderKeyMeshTypeBitCount       3
#define RenderKeyTextureIndexBitCount   4
#define RenderKeyViewTypeBitCount       2
#define RenderKeyPassIndexBitCount      4
#define RenderKeySortLayerIndexBitCount 4

#define RenderKeyMaterialIndexBitStart  0
#define RenderKeyMeshTypeBitStart       (RenderKeyMaterialIndexBitStart + RenderKeyMaterialIndexBitCount)
#define RenderKeyTextureIndexBitStart   (RenderKeyMeshTypeBitStart + RenderKeyMeshTypeBitCount)
#define RenderKeyViewTypeBitStart       (RenderKeyTextureIndexBitStart + RenderKeyTextureIndexBitCount)
#define RenderKeyPassIndexBitStart      (RenderKeyViewTypeBitStart + RenderKeyViewTypeBitCount)
#define RenderKeySortLayerIndexBitStart (RenderKeyPassIndexBitStart + RenderKeyPassIndexBitCount)

typedef uint8 ViewType;
typedef uint8 MeshType;
typedef int8  FrameBufferIndex;
typedef int8  PassIndex;
typedef int8  TextureIndex;
typedef int8  MaterialIndex;
/** Objects are rendered with descending order (highest first) */
typedef uint8 SortLayerIndex;
typedef int16 MaterialDrawBufferIndex;

#define TEXTURE_INDEX_NULL             0
#define FRAME_BUFFER_INDEX_DEFAULT     0
#define MATERIAL_DRAW_BUFFER_EMPTY_KEY -1
#define MATERIAL_DRAW_BUFFER_MAX_PROBE 32

#define BINDING_SLOT_GLOBAL      0
#define BINDING_SLOT_TEXTURE     1
#define BINDING_SLOT_UBO_CUSTOM  2
#define BINDING_SLOT_SSBO_CUSTOM 2

typedef struct
{
    Mat4 projection;
    Mat4 view;
    Mat4 inverse_view;

    float32 window_width;
    float32 window_height;

    float32 world_height;
    float32 world_width;
} Camera;

typedef struct
{
    bool32   is_initialized;
    DrawType draw_type;

    uint32 gl_program_id;
    uint32 uniform_buffer_id;
    uint32 uniform_data_size;

    uint32 location_model;
    uint32 location_uniform;
    uint32 location_texture;
} Material;

typedef struct
{
    Vec4         clear_color;
    uint32       width;
    uint32       height;
    uint32       buffer_id;
    TextureIndex texture_index;

    uint32 blend_src_rgb;
    uint32 blend_dst_rgb;
    uint32 blend_src_alpha;
    uint32 blend_dst_alpha;
} FrameBuffer;

typedef struct
{
    int32  width;
    int32  height;
    int32  channels;
    int32  format;
    int32  layer_count;
    uint32 gl_texture_id;
    uint32 gl_texture_type;
} Texture;

typedef struct
{
    void* value;
} TextureData;

typedef struct
{
    Mat4    projection;
    Mat4    view;
    float32 time;
    Vec3    _;
} GlobalUniformData;

typedef struct
{
    Vec2    size;
    float32 layer_count;
    float32 _;
} TextureUniformData;

enum ShaderProgramType
{
    ShaderProgramTypeVertex   = GL_VERTEX_SHADER,
    ShaderProgramTypeFragment = GL_FRAGMENT_SHADER
};

typedef struct
{
    Vec2 pos;
    Vec2 tex_coord;
    Vec4 color;
} VertexAtrribute_TexturedColored;

typedef struct
{
    uint32 component_count;
    uint32 index;
    uint64 size;
    uint32 type;
} VertexAttributeElement;

typedef struct VertexAttributeElementNode VertexAttributeElementNode;
struct VertexAttributeElementNode
{
    VertexAttributeElement v;

    VertexAttributeElementNode* next;
    VertexAttributeElementNode* prev;
};

typedef struct
{
    Arena*                      arena;
    uint32                      attribute_count;
    VertexAttributeElementNode* first;
    VertexAttributeElementNode* last;
} VertexAttributeInfo;

typedef struct
{
    RenderKey key;

    uint32 element_count;
    void*  uniform_buffer;

    // only needs to be set when `MeshTypeDynamic` is used
    uint32 draw_instance_count;
    uint64 vertex_buffer_size;
    void*  vertex_buffer;
} R_Batch;

typedef struct R_BatchNode R_BatchNode;
struct R_BatchNode
{
    R_Batch      v;
    R_BatchNode* next;
};

typedef struct
{
    R_BatchNode* first;
    R_BatchNode* last;
    uint32       batch_count;
} R_BatchGroup;

typedef struct
{
    R_BatchGroup*    batch_groups;
    FrameBufferIndex frame_buffer;

    bool32 is_empty;
} R_Pass;

typedef struct
{
    Arena* arena;
    Arena* frame_arena;

    float32 window_width;
    float32 window_height;
    float32 world_width;
    float32 world_height;
    Camera  camera;

    uint32 global_uniform_buffer_id;
    uint32 texture_uniform_buffer_id;
    bool32 debug;

    /* state */
    RenderKey active_render_key;
    uint8     pass_count;
    R_Pass*   passes;
    R_Batch*  previous_batch;
    R_Batch*  active_batch;

    float32 timer;
    float32 ppu;
    float32 aspect;

    /* resources */
    uint8        frame_buffer_count;
    FrameBuffer* frame_buffers;

    uint8    texture_count;
    Texture* textures;

    uint8     material_count;
    Material* materials;

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
} Renderer;
global Renderer* g_renderer;

typedef struct R_PassConfigurationNode R_PassConfigurationNode;
struct R_PassConfigurationNode
{
    FrameBufferIndex frame_buffer_index;

    R_PassConfigurationNode* next;
};

typedef struct
{
    Arena*                   temp_arena;
    uint32                   pass_count;
    R_PassConfigurationNode* first_pass;
    R_PassConfigurationNode* last_pass;
} R_PipelineConfiguration;

typedef struct
{
    uint32 window_width;
    uint32 window_height;

    // Use 0 to auto calculate it using aspect ratio from window size (at least one of width or height still needs to be provided)
    float32 world_width;
    float32 world_height;
    Color   clear_color;
} RendererConfiguration;

internal RendererConfiguration* r_config_new(Arena* temp_arena);
internal void                   r_config_set_screen_size(RendererConfiguration* configuration, float32 width, float32 height);
internal void                   r_config_set_world_size(RendererConfiguration* configuration, float32 width, float32 height);
internal void                   r_config_set_clear_color(RendererConfiguration* configuration, Color color);

internal R_PipelineConfiguration* r_pipeline_config_new(Arena* temp_arena);
internal PassIndex                r_pipeline_config_add_pass(R_PipelineConfiguration* config, FrameBufferIndex frame_buffer);
internal void                     r_pipeline_init(R_PipelineConfiguration* configuration);

internal void renderer_init(Arena* arena, RendererConfiguration* configuration);

/** vertex attributes */
internal VertexAttributeInfo* r_attribute_info_new(Arena* arena);
internal void                 r_attribute_info_add(VertexAttributeInfo* info, usize component_size, uint32 component_count, GLenum type);
internal void                 r_attribute_info_add_vec2(VertexAttributeInfo* info);
internal void                 r_attribute_info_add_vec4(VertexAttributeInfo* info);
internal void                 r_attribute_info_add_int(VertexAttributeInfo* info);
internal void                 r_attribute_info_add_uint(VertexAttributeInfo* info);

/** DEPRECATED: use `r_material_create` instead */
internal MaterialIndex material_new_deprecated(Renderer* renderer, String vertex_shader_text, String fragment_shader_text, usize uniform_data_size, bool32 is_instanced);
internal MaterialIndex r_material_create(Renderer* renderer, String vertex_shader_text, String fragment_shader_text, usize uniform_data_size, DrawType draw_type);
internal TextureIndex  texture_new(Renderer* renderer, uint32 width, uint32 height, uint32 channels, uint32 filter, void* data);
internal TextureIndex  texture_array_new(Renderer* renderer, uint32 width, uint32 height, uint32 channels, uint32 filter, uint32 layer_count, TextureData* data);
internal uint32        shader_load(String vertex_shader_text, String fragment_shader_text);

internal RenderKey render_key_new(ViewType view_type, SortLayerIndex sort_layer, PassIndex pass, TextureIndex texture, MeshType mesh_type, MaterialIndex material_index);
internal uint64    render_key_mask(RenderKey key, uint64 bit_start, uint64 bit_count);

internal void             r_frame_buffer_begin(FrameBuffer* frame_buffer);
internal TextureIndex     frame_buffer_texture(Renderer* renderer, FrameBufferIndex frame_buffer_index);
internal FrameBufferIndex r_frame_buffer_new(Renderer* renderer, uint32 width, uint32 height, uint32 filter, Color clear_color);
internal void             r_frame_buffer_set_blend(FrameBufferIndex frame_buffer_index, uint32 blend_src_rgb, uint32 blend_dst_rgb, uint32 blend_src_alpha, uint32 blend_dst_alpha);
internal void             r_render(Renderer* renderer, float32 dt);
internal void             r_render_v2(float32 dt);
internal void             texture_shader_data_set(Renderer* renderer, const Texture* texture);

internal void r_batch_commit(R_Batch batch);

internal void r_draw_single(RenderKey key, Mat4 model, void* uniform_data);
internal void r_draw_many(RenderKey key, uint64 count, Mat4* models, void* uniform_data);
internal void r_draw_many_no_copy(RenderKey key, uint64 count, Mat4* models, void* uniform_data);
internal void r_draw_pass(PassIndex source_index, PassIndex target_index, SortLayerIndex sort_layer, MaterialIndex material_index, void* uniform_data);

/** camera */
internal Camera camera_new(float32 width, float32 height, float32 near_plane, float32 far_plane, float32 window_width, float32 window_height);
internal void   camera_move(Renderer* renderer, Vec2 position);
internal Vec3   camera_position(Renderer* renderer);
internal Rect   camera_world_bounds(Camera camera);

/* converts the unit value to actual screen pixel*/
internal float32 px(float32 u);
internal float32 em(float32 v);

/** helpers */
internal float32 screen_top();
internal float32 screen_left();
internal float32 screen_right();
internal float32 screen_bottom();
internal float32 screen_height();
internal float32 screen_width();
internal Rect    screen_rect();