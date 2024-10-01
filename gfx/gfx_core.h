#pragma once
#include <opus/base/base_inc.h>
#include "gfx_math.h"

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
#define GFX_TRAIL_MAX_VERTEX_CAPACITY             (256 * 3)
#define DEFAULT_FONT_SIZE                         18

global float32 _pixel_per_unit = 1;

typedef struct
{
    uint64 v;
} GFX_Handle;

typedef uint8 GFX_ViewType;
enum
{
    GFX_ViewTypeWorld,
    GFX_ViewTypeScreen,
    GFX_ViewTypeCOUNT,
};

typedef uint8 GFX_MeshType;
enum
{
    GFX_MeshTypeTriangle,
    GFX_MeshTypeQuad,
    GFX_MeshTypeDynamic,
    GFX_MeshTypeCOUNT,
};

typedef enum
{
    GFX_DrawTypeSingle       = 0,
    GFX_DrawTypeInstanced    = 1,
    GFX_DrawTypePackedBuffer = 2,
} GFX_DrawType;

typedef struct bit_field
{
    uint32 MaterialIndex : 8;
    uint32 PassIndex     : 4;
    uint32 MeshType      : 3;
    uint32 TextureIndex  : 4;
    uint32 ViewType      : 2;
    uint32 SortLayer     : 4;
} GFX_DrawParameters;

// typedef int64 RenderKey;
// #define RenderKeyMaterialIndexBitCount  8
// #define RenderKeyMeshTypeBitCount       3
// #define RenderKeyTextureIndexBitCount   4
// #define RenderKeyViewTypeBitCount       2
// #define RenderKeyPassIndexBitCount      4
// #define RenderKeySortLayerIndexBitCount 4
// #define RenderKeyMaterialIndexBitStart  0
// #define RenderKeyMeshTypeBitStart       (RenderKeyMaterialIndexBitStart + RenderKeyMaterialIndexBitCount)
// #define RenderKeyTextureIndexBitStart   (RenderKeyMeshTypeBitStart + RenderKeyMeshTypeBitCount)
// #define RenderKeyViewTypeBitStart       (RenderKeyTextureIndexBitStart + RenderKeyTextureIndexBitCount)
// #define RenderKeyPassIndexBitStart      (RenderKeyViewTypeBitStart + RenderKeyViewTypeBitCount)
// #define RenderKeySortLayerIndexBitStart (RenderKeyPassIndexBitStart + RenderKeyPassIndexBitCount)

// typedef int8 FrameBufferIndex;
// typedef int8 PassIndex;
// typedef int8 TextureIndex;
// typedef int8 MaterialIndex;
// /** Objects are rendered with descending order (highest first) */
// typedef uint8 SortLayerIndex;

// #define TEXTURE_INDEX_NULL             0
// #define FRAME_BUFFER_INDEX_DEFAULT     0

#define MATERIAL_DRAW_BUFFER_EMPTY_KEY -1
#define MATERIAL_DRAW_BUFFER_MAX_PROBE 32

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
    void* value;
} TextureData;

typedef struct
{
    GFX_DrawParameters params;

    uint32 element_count;
    void*  uniform_buffer;

    // only needs to be set when `MeshTypeDynamic` is used
    uint32 draw_instance_count;
    uint64 vertex_buffer_size;
    void*  vertex_buffer;
} GFX_Batch;

typedef struct GFX_BatchNode GFX_BatchNode;
struct GFX_BatchNode
{
    GFX_Batch      v;
    GFX_BatchNode* next;
};

typedef struct
{
    GFX_BatchNode* first;
    GFX_BatchNode* last;
    uint32         batch_count;
} GFX_BatchGroup;

typedef struct
{
    GFX_BatchGroup* batch_groups;
    GFX_Handle      frame_buffer_handle;
    bool32          is_empty;
} GFX_Pass;

typedef struct GFX_PipelinePass GFX_PipelinePass;
struct GFX_PipelinePass
{
    GFX_Handle        frame_buffer_handle;
    GFX_PipelinePass* next;
};

typedef struct
{
    uint32            pass_count;
    GFX_PipelinePass* first_pass;
    GFX_PipelinePass* last_pass;
} GFX_Pipeline;

typedef struct
{
    Vec2 pos;
    Vec2 tex_coord;
    Vec4 color;
} GFX_VertexAtrribute_TexturedColored;

typedef struct
{
    uint32 component_count;
    uint32 index;
    uint64 size;
    uint32 type;
} GFX_VertexAttributeElement;

typedef struct GFX_VertexAttributeElementNode GFX_VertexAttributeElementNode;
struct GFX_VertexAttributeElementNode
{
    GFX_VertexAttributeElement v;

    GFX_VertexAttributeElementNode* next;
    GFX_VertexAttributeElementNode* prev;
};

typedef struct
{
    Arena*                          arena;
    uint32                          attribute_count;
    GFX_VertexAttributeElementNode* first;
    GFX_VertexAttributeElementNode* last;
} GFX_VertexAttributeInfo;

typedef struct
{
    uint32 window_width;
    uint32 window_height;

    // Use 0 to auto calculate it using aspect ratio from window size (at least one of width or height still needs to be provided)
    float32 world_width;
    float32 world_height;
    Color   clear_color;
} GFX_Configuration;

/** Core (Per Impl) */
internal void gfx_init(GFX_Configuration* configuration);
internal void gfx_render(float32 dt);

/** Draw Management (Generic) */
internal void gfx_batch_commit(GFX_Batch batch);
internal void gfx_draw_single(GFX_DrawParameters params, Mat4 model, void* uniform_data);
internal void gfx_draw_many(GFX_DrawParameters params, uint64 count, Mat4* models, void* uniform_data);
internal void gfx_draw_many_no_copy(GFX_DrawParameters params, uint64 count, Mat4* models, void* uniform_data);
internal void gfx_draw_pass(GFX_Handle source_pass, GFX_Handle target_pass, uint8 sort_layer, GFX_Handle material, void* uniform_data);

/** Renderer Configuration */
internal void gfx_config_set_screen_size(GFX_Configuration* configuration, float32 width, float32 height);
internal void gfx_config_set_world_size(GFX_Configuration* configuration, float32 width, float32 height);
internal void gfx_config_set_clear_color(GFX_Configuration* configuration, Color color);

/** Pipeline Configuration */
internal void       gfx_pipeline_init(GFX_Pipeline* pipeline);
internal GFX_Handle gfx_pipeline_add_pass(GFX_Pipeline* pipeline, GFX_Handle frame_buffer);

/** Vertex Attribute Configuration */
internal GFX_VertexAttributeInfo* gfx_attribute_info_new(Arena* arena);
internal void                     gfx_attribute_info_add_vec2(GFX_VertexAttributeInfo* info);
internal void                     gfx_attribute_info_add_vec4(GFX_VertexAttributeInfo* info);
internal void                     gfx_attribute_info_add_int(GFX_VertexAttributeInfo* info);
internal void                     gfx_attribute_info_add_uint(GFX_VertexAttributeInfo* info);

/** Material Configuration */
internal GFX_Handle gfx_material_new(String vertex_shader_text, String fragment_shader_text, usize uniform_data_size, GFX_DrawType draw_type);
internal GFX_Handle gfx_texture_new(uint32 width, uint32 height, uint32 channels, uint32 filter, void* data);
internal GFX_Handle gfx_texture_array_new(uint32 width, uint32 height, uint32 channels, uint32 filter, uint32 layer_count, TextureData* data);
internal uint32     gfx_shader_load(String vertex_shader_text, String fragment_shader_text);

/** FrameBuffer Controls */
internal GFX_Handle gfx_frame_buffer_new(uint32 width, uint32 height, uint32 filter, Color clear_color);
internal void       gfx_frame_buffer_set_blend(GFX_Handle frame_buffer, uint32 blend_src_rgb, uint32 blend_dst_rgb, uint32 blend_src_alpha, uint32 blend_dst_alpha);
internal GFX_Handle gfx_frame_buffer_texture(GFX_Handle frame_buffer);

/** Camera Controls */
internal GFX_Handle gfx_camera_new(float32 width, float32 height, float32 near_plane, float32 far_plane, float32 window_width, float32 window_height);
internal void       gfx_camera_move(GFX_Handle camera_handle, Vec2 position);
internal Vec3       gfx_camera_position(GFX_Handle camera_handle);
internal Rect       gfx_camera_world_bounds(GFX_Handle camera_handle);