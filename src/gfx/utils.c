#include "utils.h"

internal TextureIndex
texture_new_from_file(Renderer* renderer, String texture_path, bool32 pixel_perfect, bool32 flip_vertical)
{
    stbi_set_flip_vertically_on_load(flip_vertical);

    int32 width, height, channels;
    uint8 *data = stbi_load(texture_path.value, &width, &height, &channels, 0);
    
    if(!data)
    {
        log_error("failed to load texture");
        return 0;
    }

    uint32 filter = pixel_perfect ? GL_NEAREST : GL_LINEAR;
    TextureIndex texture = texture_new(renderer, width, height, channels, filter, data);
    stbi_image_free(data);
    return texture;
}