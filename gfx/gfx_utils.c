#include "gfx_utils.h"

/** resource load utility */
internal MaterialIndex
gfx_material_from_path(String vertex_shader_path, String frag_shader_path, uint32 uniform_data_size, GFX_DrawType draw_type)
{
    ArenaTemp     temp          = scratch_begin(0, 0);
    String        vertex_shader = os_path_read_all_as_string(vertex_shader_path, temp.arena);
    String        frag_shader   = os_path_read_all_as_string(frag_shader_path, temp.arena);
    MaterialIndex result        = gfx_material_new(vertex_shader, frag_shader, uniform_data_size, draw_type);
    return result;
}

internal TextureIndex
gfx_texture_from_file(String path, bool32 pixel_perfect, bool32 flip_vertical)
{
    stbi_set_flip_vertically_on_load(flip_vertical);

    int32  width, height, channels;
    uint8* data = stbi_load(path.value, &width, &height, &channels, 0);

    if (!data)
    {
        log_error("failed to load texture");
        return 0;
    }

    uint32       filter  = pixel_perfect ? GL_NEAREST : GL_LINEAR;
    TextureIndex texture = gfx_texture_new(width, height, channels, filter, data);
    stbi_image_free(data);
    return texture;
}