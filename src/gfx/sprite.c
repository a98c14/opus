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

    atlas->width  = g_renderer->textures[texture_index].width;
    atlas->height = g_renderer->textures[texture_index].height;
    return atlas;
}

internal void
renderer_load_sprite_atlas(Renderer* renderer, SpriteAtlas* atlas)
{
    /* Create Sprite Bounds SSBO */
    glGenBuffers(1, &renderer->sprites_ssbo_id);
    ArenaTemp temp          = arena_begin_temp(renderer->arena);
    Vec4*     sprite_bounds = arena_push_array_zero(temp.arena, Vec4, atlas->sprite_count);
    for (uint32 i = 0; i < atlas->sprite_count; i++)
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

internal Bounds
r_sprite_tex_coords(SpriteAtlas* atlas, SpriteIndex sprite_index)
{
    Sprite sprite = atlas->sprites[sprite_index];
    Bounds bounds;
    bounds.left   = sprite.rect.x / atlas->width;
    bounds.right  = (sprite.rect.x + sprite.rect.w) / atlas->width;
    bounds.top    = sprite.rect.y / atlas->height;
    bounds.bottom = (sprite.rect.y + sprite.rect.h) / atlas->height;

    // float x     = ((a_tex_coord.x * v_bounds.z) + v_bounds.x) / texture_size.x;
    // float y     = 1 - (((1 - a_tex_coord.y) * v_bounds.w) + v_bounds.y) / texture_size.y;
    // v_tex_coord = vec2(x, y);
    return bounds;
}

internal Rect
r_sprite_rect_scaled(SpriteAtlas* atlas, SpriteIndex sprite_index, Vec2 pos, float32 scale)
{
    Sprite sprite = atlas->sprites[sprite_index];
    Rect   rect   = rect_at(pos, vec2_scale(sprite.size.size, scale), AlignmentCenter);
    return rect;
}

internal Rect
r_sprite_rect(SpriteAtlas* atlas, SpriteIndex sprite_index, Vec2 pos)
{
    return r_sprite_rect_scaled(atlas, sprite_index, pos, 1);
}

internal inline Vec2
r_sprite_get_pivot(Sprite sprite, Vec2 scale, Vec2 flip)
{
    Vec2 result;
    result.x = (sprite.size.w / 2.0 + sprite.size.x - sprite.pivot.x) * flip.x * scale.x;
    result.y = -(sprite.size.h / 2.0 + sprite.size.y - sprite.pivot.y) * flip.y * scale.y;
    return result;
}

internal inline Animation
animation_from_atlas(SpriteAtlas* atlas, AnimationIndex index)
{
    return atlas->animations[index];
}

internal inline uint16
animation_length(Animation animation)
{
    return animation.sprite_end_index - animation.sprite_start_index;
}
