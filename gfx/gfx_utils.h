#pragma once
#include "./gfx_core.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"

/* converts the unit value to actual screen pixel*/
internal float32 px(float32 u);
internal float32 unit(float32 u);
internal float32 em(float32 v);

/** helpers */
internal float32 screen_top();
internal float32 screen_left();
internal float32 screen_right();
internal float32 screen_bottom();
internal float32 screen_height();
internal float32 screen_width();
internal Rect    screen_rect();

/** resource load utility */
internal MaterialIndex gfx_material_from_path(String vertex_shader_path, String frag_shader_path, uint32 uniform_data_size, GFX_DrawType draw_type);
internal TextureIndex  gfx_texture_from_file(String path, bool32 pixel_perfect, bool32 flip_vertical);