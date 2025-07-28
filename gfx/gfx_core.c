#include "gfx_core.h"

internal GFX_Handle
gfx_handle_zero(void)
{
    GFX_Handle result = {0};
    return result;
}

internal GFX_Camera
gfx_camera_new(float32 width, float32 height, float32 near_plane, float32 far_plane, uint32 window_width, uint32 window_height)
{
    GFX_Camera result;
    result.projection    = mat4_ortho(width, height, near_plane, far_plane);
    result.view          = mat4_identity();
    result.inverse_view  = mat4_identity();
    result.world_width   = width;
    result.world_height  = height;
    result.window_width  = window_width;
    result.window_height = window_height;
    return result;
}

internal RenderKey
gfx_render_key_new(GFX_ViewType view_type, SortLayerIndex sort_layer, PassIndex pass, TextureIndex texture, GFX_MeshType mesh_type, MaterialIndex material_index)
{
    xassert_m(view_type < 4, "invalid view_type value provided");
    log_trace("Render key new, sort: %2d, pass: %2d, view: %2d, texture: %2d, geometry: %2d, material: %2d", sort_layer, pass, view_type, texture, geometry, material_index);
    RenderKey result = ((uint64)sort_layer << RenderKeySortLayerIndexBitStart) +
                       ((uint64)pass << RenderKeyPassIndexBitStart) +
                       ((uint64)view_type << RenderKeyViewTypeBitStart) +
                       ((uint64)texture << RenderKeyTextureIndexBitStart) +
                       ((uint64)material_index << RenderKeyMaterialIndexBitStart) +
                       ((uint64)mesh_type << RenderKeyMeshTypeBitStart);
    return result;
}

internal RenderKey
gfx_render_key_new_default(GFX_ViewType view_type, SortLayerIndex sort_layer, PassIndex pass, TextureIndex texture, MaterialIndex material_index)
{
    return gfx_render_key_new(view_type, sort_layer, pass, texture, 0, material_index);
}

internal uint64
gfx_render_key_mask(RenderKey key, uint64 bit_start, uint64 bit_count)
{
    return (key >> bit_start) & ((1ull << bit_count) - 1);
}

/** Utility */
internal Bounds
gfx_rect_to_texture_bounds(Rect rect, uint32 texture_width, uint32 texture_height)
{
    return bounds_from_rect(rect_scale_xy(rect_flip_y(rect), 1.f / texture_width, 1.f / texture_height));
}