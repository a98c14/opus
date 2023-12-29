#pragma once
#include <glad/gl.h>
#include <core/log.h>
#include <core/defines.h>
#include <core/strings.h>
#include <core/math.h>
#include <core/hash.h>
#include <core/asserts.h>

/* Constants */
#define MATERIAL_CAPACITY 32
#define TEXTURE_CAPACITY 32
#define GEOMETRY_CAPACITY 32
#define LAYER_CAPACITY 16
#define SORTING_LAYER_CAPACITY 16
#define SORT_LAYER_INDEX_DEFAULT 8

#define MATERIAL_DRAW_BUFFER_CAPACITY_PER_SETTING 16
#define MATERIAL_DRAW_BUFFER_CAPACITY 512
#define MATERIAL_DRAW_BUFFER_ELEMENT_CAPACITY 8192

// TEMP: testing global variable solution out. Potentially dangerous?
global float32 _pixel_per_unit = 1;
#define DEFAULT_FONT_SIZE 18

enum
{
    ViewTypeWorld,
    ViewTypeScreen,
    ViewTypeCOUNT,
};

typedef uint8 ViewType;
typedef int8 FrameBufferIndex;
typedef int8 TextureIndex;
typedef int8 MaterialIndex;
typedef uint8 GeometryIndex;
/** Objects are rendered with descending order (highest first) */
typedef uint8 SortLayerIndex;
typedef int16 MaterialDrawBufferIndex;

#define TEXTURE_INDEX_NULL 0
#define FRAME_BUFFER_INDEX_DEFAULT 0
#define MATERIAL_DRAW_BUFFER_EMPTY_KEY -1
#define MATERIAL_DRAW_BUFFER_MAX_PROBE 32

#define BINDING_SLOT_GLOBAL 0
#define BINDING_SLOT_TEXTURE 1
#define BINDING_SLOT_CAMERA 2
#define BINDING_SLOT_SSBO_MODEL 3
#define BINDING_SLOT_UBO_CUSTOM 4
#define BINDING_SLOT_SSBO_CUSTOM 4

typedef struct
{
    Mat3 projection;
    Mat3 view;
} Camera2D;

typedef struct
{
    Mat4 projection;
    Mat4 view;

    float32 window_width;
    float32 window_height;

    float32 world_height;
    float32 world_width;
} Camera;

typedef struct
{
    uint8 r;
    uint8 g;
    uint8 b;
    uint8 a;
} Color;

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
    Vec4 clear_color;
    uint32 width;
    uint32 height;
    uint32 buffer_id;
    TextureIndex texture_index;
} FrameBuffer;

typedef struct
{
    int32 width;
    int32 height;
    int32 channels;
    int32 format;
    int32 layer_count;
    uint32 gl_texture_id;
    uint32 gl_texture_type;
} Texture;

typedef struct
{
    float32 time;
    Vec3 _;
} GlobalUniformData;

typedef struct
{
    Vec2 size;
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
    uint64 key;
    MaterialDrawBufferIndex index;
    MaterialIndex material_index;
    uint32 element_count;
    Mat4* model_buffer;
    void* shader_data_buffer;
} MaterialDrawBuffer;

typedef struct
{
    GeometryIndex geometry_index;

    uint32 material_count;
    MaterialDrawBufferIndex material_buffer_indices[MATERIAL_DRAW_BUFFER_CAPACITY_PER_SETTING];
} GeometryDrawBuffer;

typedef struct
{
    TextureIndex texture_index;

    uint32 geometry_count;
    GeometryDrawBuffer geometry_draw_buffers[GEOMETRY_CAPACITY];
} TextureDrawBuffer;

typedef struct
{
    ViewType view_type;

    uint32 texture_count;
    TextureDrawBuffer texture_draw_buffers[TEXTURE_CAPACITY];
} ViewDrawBuffer;

typedef struct
{
    // grouped by view type (screen, world)
    uint32 view_count;
    ViewDrawBuffer view_buffers[ViewTypeCOUNT];
} SortingLayerDrawBuffer;

typedef struct
{
    FrameBufferIndex layer_index;

    /** TODO: Should we keep the count here and sort when a new buffer is created
     * instead of looping through all evevy frame? */ 
    SortingLayerDrawBuffer sorting_layer_draw_buffers[SORTING_LAYER_CAPACITY];
} LayerDrawBuffer;

typedef struct
{
    /** model and shader data that is needed to render the image
     * indexed by hash of layer, view, texture, material */
    uint32 material_draw_buffer_count;
    MaterialDrawBuffer* material_draw_buffers;
    Geometry active_geometry;

    // draw buffers
    uint32 layer_count;
    LayerDrawBuffer layer_draw_buffers[LAYER_CAPACITY];
} RendererDrawState;

typedef struct
{
    int32 capacity;
    int32 index;
    uint32 uniform_data_size;
    Mat4* model_buffer;
    void* uniform_data_buffer;
} DrawBuffer;

typedef struct
{
    uint32 count;
    uint32 index;
    DrawBuffer* elements;
} DrawBufferArray;

typedef struct
{
    Arena* arena;

    float32 window_width;
    float32 window_height;
    float32 world_width;
    float32 world_height;
    Camera camera;

    uint32 global_uniform_buffer_id;
    uint32 texture_uniform_buffer_id;
    uint32 camera_uniform_buffer_id;
    uint32 mvp_ssbo_id;

    /* state */
    float32 timer;
    float32 ppu;
    float32 aspect;
    RendererDrawState* draw_state;

    /* resources */
    uint8 frame_buffer_count;
    FrameBuffer* frame_buffers;

    uint8 texture_count;
    Texture* textures;

    uint8 geometry_count;
    Geometry* geometries;

    uint8 material_count;
    Material* materials;

    /* stats */
    uint32 stat_draw_count;
    uint32 stat_object_count;
    uint16 stat_initialized_buffer_count;
    uint32 stat_probe_count_max;
    uint64 stat_probe_count;
    float32 stat_probe_count_sum;
} Renderer;

typedef struct
{
    uint32 window_width;
    uint32 window_height;

    // Use 0 to auto calculate it using aspect ratio from window size (at least one of width or height still needs to be provided)
    float32 world_width;
    float32 world_height;
    Color clear_color;
} RendererConfiguration;

internal Renderer*
renderer_new(Arena* arena, RendererConfiguration* configuration);

internal RendererDrawState*
renderer_draw_state_new(Arena* arena);

internal Camera
camera_new(float32 width, float32 height, float32 near_plane, float32 far_plane, float32 window_width, float32 window_height);

internal uint32
shader_load(String vertex_shader_text, String fragment_shader_text);

internal MaterialIndex
material_new(Renderer* renderer, String vertex_shader_text, String fragment_shader_text, usize uniform_data_size, bool32 is_instanced);

internal GeometryIndex
geometry_new(Renderer* renderer, int32 index_count, int32 vertex_array_object);

internal TextureIndex
texture_new(Renderer* renderer, uint32 width, uint32 height, uint32 channels, uint32 filter, void* data);

internal MaterialDrawBuffer*
renderer_get_material_buffer(Renderer* renderer, ViewType view_type, SortLayerIndex sort_layer, FrameBufferIndex layer, TextureIndex texture, GeometryIndex geometry, MaterialIndex material_index, uint32 available_space);

internal DrawBuffer
renderer_buffer_request(Renderer* renderer, ViewType view_type, SortLayerIndex sort_layer, FrameBufferIndex layer, TextureIndex texture, GeometryIndex geometry, MaterialIndex material_index, uint32 count);

internal DrawBufferArray*
renderer_buffer_request_batched(Arena* arena, Renderer* renderer, ViewType view_type, SortLayerIndex sort_layer, FrameBufferIndex layer, TextureIndex texture, GeometryIndex geometry, MaterialIndex material_index, uint32 count);

internal bool32
draw_buffer_insert(DrawBuffer* draw_buffer, Mat4 model, void* uniform_data);

internal void
draw_buffer_array_insert(DrawBufferArray* draw_buffer_array, Mat4 model, void* uniform_data);

internal void
frame_buffer_begin(FrameBuffer* frame_buffer);

internal FrameBufferIndex
renderer_frame_buffer_init(Renderer* renderer, uint32 width, uint32 height, uint32 filter, Color clear_color);

internal Vec4
color_to_vec4(Color color);

internal Color
vec4_to_color(Vec4 c);

internal void
renderer_render(Renderer* renderer, float32 dt);

internal void
texture_shader_data_set(Renderer* renderer, const Texture* texture);

/* converts the unit value to actual screen pixel*/
internal float32 px(float32 u);
internal float32 em(float32 v);