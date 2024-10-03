#pragma once
#include <opus/base/base_inc.h>
#include "gfx_math.h"

typedef struct
{
    uint64 index;
    uint64 generation;
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

typedef uint64 RenderKey;
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

typedef int32 FrameBufferIndex;
typedef int32 PassIndex;
typedef int32 TextureIndex;
typedef int32 MaterialIndex;
typedef int32 SortLayerIndex; // Objects are rendered with descending order (highest first)

#define MATERIAL_DRAW_BUFFER_EMPTY_KEY -1
#define MATERIAL_DRAW_BUFFER_MAX_PROBE 32

typedef struct
{
    Mat4 projection;
    Mat4 view;
    Mat4 inverse_view;

    uint32 window_width;
    uint32 window_height;

    float32 world_height;
    float32 world_width;
} GFX_Camera;

typedef struct
{
    void* value;
} TextureData;

typedef struct
{
    RenderKey params;

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
    uint32 size;
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

/** Handle Stuff (Generic) */
internal GFX_Handle gfx_handle_zero();

/** Core (Per Impl) */
internal void gfx_init(GFX_Configuration* configuration);
internal void gfx_batch_commit(GFX_Batch batch);
internal void gfx_render(float32 dt);

/** Renderer Configuration */
internal void gfx_config_set_screen_size(GFX_Configuration* configuration, float32 width, float32 height);
internal void gfx_config_set_world_size(GFX_Configuration* configuration, float32 width, float32 height);
internal void gfx_config_set_clear_color(GFX_Configuration* configuration, Color color);

/** Pipeline Configuration */
internal void       gfx_pipeline_init(GFX_Pipeline* pipeline);
internal GFX_Handle gfx_pipeline_add_pass(GFX_Pipeline* pipeline, GFX_Handle frame_buffer);

/** Vertex Attribute Configuration (Per Impl) */
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
internal GFX_Camera gfx_camera_new(float32 width, float32 height, float32 near_plane, float32 far_plane, uint32 window_width, uint32 window_height);
internal void       gfx_camera_move(GFX_Camera* camera, Vec2 position);
internal Vec3       gfx_camera_position(GFX_Camera camera);
internal Rect       gfx_camera_world_bounds(GFX_Camera camera);