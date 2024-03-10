#pragma once
#include <base.h>
#include <glad/gl.h>

#include "math.h"

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

typedef int64 RenderKey;
const uint64  RenderKeyMaterialIndexBitStart  = 0;
const uint64  RenderKeyMaterialIndexBitCount  = 8;
const uint64  RenderKeyGeometryIndexBitStart  = 8;
const uint64  RenderKeyGeometryIndexBitCount  = 8;
const uint64  RenderKeyTextureIndexBitStart   = 16;
const uint64  RenderKeyTextureIndexBitCount   = 8;
const uint64  RenderKeyViewTypeBitStart       = 24;
const uint64  RenderKeyViewTypeBitCount       = 2;
const uint64  RenderKeyPassIndexBitStart      = 26;
const uint64  RenderKeyPassIndexBitCount      = 8;
const uint64  RenderKeySortLayerIndexBitStart = 34;
const uint64  RenderKeySortLayerIndexBitCount = 8;

typedef uint8 ViewType;
typedef int8  FrameBufferIndex;
typedef int8  PassIndex;
typedef int8  TextureIndex;
typedef int8  MaterialIndex;
typedef uint8 GeometryIndex;
/** Objects are rendered with descending order (highest first) */
typedef uint8 SortLayerIndex;
typedef int16 MaterialDrawBufferIndex;

#define TEXTURE_INDEX_NULL             0
#define FRAME_BUFFER_INDEX_DEFAULT     0
#define MATERIAL_DRAW_BUFFER_EMPTY_KEY -1
#define MATERIAL_DRAW_BUFFER_MAX_PROBE 32

#define BINDING_SLOT_GLOBAL        0
#define BINDING_SLOT_TEXTURE       1
#define BINDING_SLOT_CAMERA        2
#define BINDING_SLOT_SSBO_MODEL    3
#define BINDING_SLOT_UBO_CUSTOM    4
#define BINDING_SLOT_SSBO_CUSTOM   4
#define BINDING_SLOT_SPRITE_BOUNDS 5

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

typedef uint32 Color;

typedef struct
{
    uint32 index_count;
    uint32 vertex_array_object;
} Geometry;

typedef struct
{
    bool32 is_initialized;
    bool32 is_instanced;

    uint32 gl_program_id;
    uint32 uniform_buffer_id;
    uint32 uniform_data_size;

    uint32 location_model;
    uint32 location_shader;
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
    float32 time;
    Vec3    _;
} GlobalUniformData;

typedef struct
{
    Vec2    size;
    float32 layer_count;
    float32 _;
} TextureUniformData;

typedef struct
{
    Mat4 projection;
    Mat4 view;
} CameraUniformData;

enum ShaderProgramType
{
    ShaderProgramTypeVertex   = GL_VERTEX_SHADER,
    ShaderProgramTypeFragment = GL_FRAGMENT_SHADER
};

typedef struct
{
    RenderKey key;
    uint32    element_count;
    uint32    uniform_data_size;
    Mat4*     model_buffer;
    void*     uniform_buffer;
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
    R_BatchGroup* batch_groups;
    uint32        total_draw_count;

    FrameBufferIndex frame_buffer;
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
    uint32 camera_uniform_buffer_id;
    uint32 sprites_ssbo_id;
    uint32 mvp_ssbo_id;

    /* state */
    RenderKey active_render_key;
    uint8     pass_count;
    R_Pass*   passes;

    float32 timer;
    float32 ppu;
    float32 aspect;

    /* resources */
    uint8        frame_buffer_count;
    FrameBuffer* frame_buffers;

    uint8    texture_count;
    Texture* textures;

    uint8     geometry_count;
    Geometry* geometries;

    uint8     material_count;
    Material* materials;

    GeometryIndex quad;
    GeometryIndex triangle;

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

internal void          renderer_init(Arena* arena, RendererConfiguration* configuration);
internal MaterialIndex material_new(Renderer* renderer, String vertex_shader_text, String fragment_shader_text, usize uniform_data_size, bool32 is_instanced);
internal GeometryIndex geometry_new(Renderer* renderer, int32 index_count, int32 vertex_array_object);
internal TextureIndex  texture_new(Renderer* renderer, uint32 width, uint32 height, uint32 channels, uint32 filter, void* data);
internal TextureIndex  texture_array_new(Renderer* renderer, uint32 width, uint32 height, uint32 channels, uint32 filter, uint32 layer_count, TextureData* data);
internal uint32        shader_load(String vertex_shader_text, String fragment_shader_text);

internal RenderKey render_key_new(ViewType view_type, SortLayerIndex sort_layer, PassIndex pass, TextureIndex texture, GeometryIndex geometry, MaterialIndex material_index);
internal uint64    render_key_mask(RenderKey key, uint64 bit_start, uint64 bit_count);

internal void             frame_buffer_begin(FrameBuffer* frame_buffer);
internal TextureIndex     frame_buffer_texture(Renderer* renderer, FrameBufferIndex frame_buffer_index);
internal FrameBufferIndex r_frame_buffer_new(Renderer* renderer, uint32 width, uint32 height, uint32 filter, Color clear_color);
internal void             r_frame_buffer_set_blend(FrameBufferIndex frame_buffer_index, uint32 blend_src_rgb, uint32 blend_dst_rgb, uint32 blend_src_alpha, uint32 blend_dst_alpha);
internal Vec4             color_v4(Color hex);
internal Color            vec4_to_color(Vec4 c);
internal void             r_render(Renderer* renderer, float32 dt);
internal void             texture_shader_data_set(Renderer* renderer, const Texture* texture);

internal R_BatchNode* r_batch_reserve(RenderKey key, uint64 element_count);
internal void         r_batch_commit(R_BatchNode* node);
internal R_Batch*     r_batch_from_key(RenderKey key, uint64 element_count);
internal void         r_draw_single(RenderKey key, Mat4 model, void* uniform_data);
internal void         r_draw_many(RenderKey key, uint64 count, Mat4* models, void* uniform_data);
internal void         r_draw_many_no_copy(RenderKey key, uint64 count, Mat4* models, void* uniform_data);
internal void         r_draw_pass(PassIndex source_index, PassIndex target_index, SortLayerIndex sort_layer, MaterialIndex material_index, void* uniform_data);
internal void         r_draw_batch_internal(Geometry* geometry, Material* material, uint64 element_count, Mat4* models, void* uniform_data);

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