#include "sprite.h"
#include <core/memory.h>
#include <gfx/base.h>

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
    /* Create Sprite Bounds SSBO */
    glGenBuffers(1, &renderer->sprites_ssbo_id);
    ArenaTemp temp          = arena_begin_temp(renderer->arena);
    Vec4*     sprite_bounds = arena_push_array_zero(temp.arena, Vec4, atlas->sprite_count);
    for (int i = 0; i < atlas->sprite_count; i++)
    {
        sprite_bounds[i].x = atlas->sprites[i].rect.x;
        sprite_bounds[i].y = atlas->sprites[i].rect.y;
        sprite_bounds[i].z = atlas->sprites[i].rect.w;
        sprite_bounds[i].w = atlas->sprites[i].rect.h;
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderer->sprites_ssbo_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Vec4) * atlas->sprite_count, sprite_bounds, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_SLOT_SPRITE_BOUNDS, renderer->sprites_ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    arena_end_temp(temp);
}

internal inline Vec2
sprite_get_pivot(Sprite sprite, Vec2 scale, Vec2 flip)
{
    Vec2 result;
    result.x = (sprite.size.w / 2.0 + sprite.size.x - sprite.pivot.x) * flip.x * scale.x;
    result.y = -(sprite.size.h / 2.0 + sprite.size.y - sprite.pivot.y) * flip.y * scale.y;
    return result;
}
