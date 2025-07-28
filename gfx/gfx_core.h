#pragma once
#include "../base/base_inc.h"
#include "../os/os_inc.h"
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
#define RenderKeyTextureIndexBitCount   7
#define RenderKeyViewTypeBitCount       2
#define RenderKeyPassIndexBitCount      4
#define RenderKeySortLayerIndexBitCount 4
#define RenderKeyMaterialIndexBitStart  0
#define RenderKeyMeshTypeBitStart       (RenderKeyMaterialIndexBitStart + RenderKeyMaterialIndexBitCount)
#define RenderKeyTextureIndexBitStart   (RenderKeyMeshTypeBitStart + RenderKeyMeshTypeBitCount)
#define RenderKeyViewTypeBitStart       (RenderKeyTextureIndexBitStart + RenderKeyTextureIndexBitCount)
#define RenderKeyPassIndexBitStart      (RenderKeyViewTypeBitStart + RenderKeyViewTypeBitCount)
#define RenderKeySortLayerIndexBitStart (RenderKeyPassIndexBitStart + RenderKeyPassIndexBitCount)
#define RenderKeyTotalBitCount          (RenderKeySortLayerIndexBitStart + RenderKeySortLayerIndexBitCount)

typedef uint64 FrameBufferIndex;
typedef uint64 PassIndex;
typedef uint64 TextureIndex;
typedef uint64 MaterialIndex;
typedef uint64 SortLayerIndex; // Objects are rendered with descending order (highest first)

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
    RenderKey key;

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
    GFX_BatchGroup*  batch_groups; // one batch_group per sorting layer (length -> sorting layer capacity)
    FrameBufferIndex frame_buffer;
    bool32           is_empty;
} GFX_Pass;

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

/** Render Key */
internal RenderKey gfx_render_key_new(GFX_ViewType view_type, SortLayerIndex sort_layer, PassIndex pass, TextureIndex texture, GFX_MeshType mesh_type, MaterialIndex material_index);
internal RenderKey gfx_render_key_new_default(GFX_ViewType view_type, SortLayerIndex sort_layer, PassIndex pass, TextureIndex texture, MaterialIndex material_index);
internal uint64    gfx_render_key_mask(RenderKey key, uint64 bit_start, uint64 bit_count);

/** Pipeline Configuration */
internal PassIndex gfx_pipeline_add_pass(FrameBufferIndex frame_buffer);

/** Vertex Attribute Configuration (Per Impl) */
internal GFX_VertexAttributeInfo* gfx_attribute_info_new(Arena* arena);
internal void                     gfx_attribute_info_add_vec2(GFX_VertexAttributeInfo* info);
internal void                     gfx_attribute_info_add_vec4(GFX_VertexAttributeInfo* info);
internal void                     gfx_attribute_info_add_int(GFX_VertexAttributeInfo* info);
internal void                     gfx_attribute_info_add_uint(GFX_VertexAttributeInfo* info);

/** Resources */
internal MaterialIndex gfx_material_new(String vertex_shader_text, String fragment_shader_text, uint32 uniform_data_size, GFX_DrawType draw_type);
internal TextureIndex  gfx_texture_new(uint32 width, uint32 height, uint32 channels, uint32 filter, void* data);
internal TextureIndex  gfx_texture_array_new(uint32 width, uint32 height, uint32 channels, uint32 filter, uint32 layer_count, TextureData* data);
internal void          gfx_texture_write(TextureIndex texture_index, void* data);
internal void          gfx_texture_write_rect(TextureIndex texture_index, Rect rect, void* data);
internal IVec2         gfx_texture_dims(TextureIndex texture);
internal uint32        gfx_shader_load(String vertex_shader_text, String fragment_shader_text);

internal uint64 gfx_material_uniform_data_size(MaterialIndex material);

/** FrameBuffer Controls */
internal FrameBufferIndex gfx_frame_buffer_new(uint32 width, uint32 height, uint32 filter, Color clear_color);
internal void             gfx_frame_buffer_set_blend(GFX_Handle frame_buffer, uint32 blend_src_rgb, uint32 blend_dst_rgb, uint32 blend_src_alpha, uint32 blend_dst_alpha);
internal GFX_Handle       gfx_frame_buffer_texture(GFX_Handle frame_buffer);

/** Camera Controls */
internal GFX_Camera gfx_camera_new(float32 width, float32 height, float32 near_plane, float32 far_plane, uint32 window_width, uint32 window_height);
internal void       gfx_camera_move(GFX_Camera* camera, Vec2 position);
internal Vec3       gfx_camera_position(GFX_Camera camera);
internal Rect       gfx_camera_world_bounds(GFX_Camera camera);

/** Utility */
internal Vec2   gfx_window_to_screen_position(Vec2 p);
internal Vec2   gfx_window_to_world_position(Vec2 p);
internal Bounds gfx_rect_to_texture_bounds(Rect rect, uint32 texture_width, uint32 texture_height);

/** Debug */
internal void gfx_enable_debug();