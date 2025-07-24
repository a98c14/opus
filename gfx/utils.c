#include "utils.h"

internal TextureIndex
texture_new_from_file(Renderer* renderer, String texture_path, bool32 pixel_perfect, bool32 flip_vertical)
{
    stbi_set_flip_vertically_on_load(flip_vertical);

    int32  width, height, channels;
    uint8* data = stbi_load(texture_path.value, &width, &height, &channels, 0);

    if (!data)
    {
        log_error("failed to load texture");
        return 0;
    }

    uint32       filter  = pixel_perfect ? GL_NEAREST : GL_LINEAR;
    TextureIndex texture = texture_new(renderer, width, height, channels, filter, data);
    stbi_image_free(data);
    return texture;
}

internal TextureIndex
texture_array_from_file(Arena* arena, Renderer* renderer, StringList texture_paths, bool32 pixel_perfect, bool32 flip_vertical)
{
    TextureData* data_array = arena_push_array(arena, TextureData, texture_paths.count);
    StringNode*  path       = texture_paths.first;
    int32        width = 0, height = 0, channels = 0;
    uint32       data_index = 0;
    while (path)
    {
        stbi_set_flip_vertically_on_load(flip_vertical);
        uint8* data = stbi_load(path->value.value, &width, &height, &channels, 0);
        if (!data)
        {
            log_error("failed to load texture array texture, index: %d, path: %s", data_index, path->value.value);
            return 0;
        }
        data_array[data_index].value = data;
        data_index++;
        path = path->next;
    }
    xassert_m(width > 0 && height > 0 && channels > 0, "failed to load texture array");
    uint32       filter  = pixel_perfect ? GL_NEAREST : GL_LINEAR;
    TextureIndex texture = texture_array_new(renderer, width, height, channels, filter, texture_paths.count, data_array);
    for (int i = 0; i < texture_paths.count; i++)
    {
        stbi_image_free(data_array[i].value);
    }
    return texture;
}