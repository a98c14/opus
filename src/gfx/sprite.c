#include "sprite.h"

internal SpriteAtlas*
sprite_atlas_new(Arena* arena, TextureIndex texture_index, const Animation* animations, const Sprite* sprites, const TextureIndex* texture_indices, uint32 animation_count, uint32 sprite_count)
{
    SpriteAtlas* atlas            = arena_push_struct_zero(arena, SpriteAtlas);
    atlas->sprites                = sprites;
    atlas->animations             = animations;
    atlas->texture                = texture_index;
    atlas->sprite_count           = sprite_count;
    atlas->animation_count        = animation_count;
    atlas->sprite_texture_indices = texture_indices;
    return atlas;
}

internal void
renderer_load_sprite_atlas(Renderer* renderer, SpriteAtlas* atlas)
{
}