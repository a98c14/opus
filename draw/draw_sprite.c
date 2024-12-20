#include "draw_sprite.h"

internal D_SpriteAtlas
sprite_atlas_new(TextureIndex texture_index, const D_Animation* animations, const D_Sprite* sprites, const TextureIndex* texture_indices, uint32 animation_count, uint32 sprite_count)
{
    D_SpriteAtlas atlas;
    atlas.sprites                = sprites;
    atlas.animations             = animations;
    atlas.texture                = texture_index;
    atlas.sprite_count           = sprite_count;
    atlas.animation_count        = animation_count;
    atlas.sprite_texture_indices = texture_indices;

    IVec2 texture_wh = gfx_texture_dims(texture_index);
    atlas.width      = (float32)texture_wh.x;
    atlas.height     = (float32)texture_wh.y;
    return atlas;
}

internal Bounds
d_sprite_tex_coords(D_SpriteAtlas atlas, D_SpriteIndex sprite_index)
{
    D_Sprite sprite = atlas.sprites[sprite_index];
    Bounds   bounds;
    bounds.left   = sprite.rect.x / atlas.width;
    bounds.right  = (sprite.rect.x + sprite.rect.w) / atlas.width;
    bounds.top    = sprite.rect.y / atlas.height;
    bounds.bottom = (sprite.rect.y + sprite.rect.h) / atlas.height;

    // float x     = ((a_tex_coord.x * v_bounds.z) + v_bounds.x) / texture_size.x;
    // float y     = 1 - (((1 - a_tex_coord.y) * v_bounds.w) + v_bounds.y) / texture_size.y;
    // v_tex_coord = vec2(x, y);
    return bounds;
}

internal Rect
d_sprite_rect_scaled(D_SpriteAtlas atlas, D_SpriteIndex sprite_index, Vec2 pos, float32 scale)
{
    D_Sprite sprite = atlas.sprites[sprite_index];
    Rect     rect   = rect_at(pos, vec2_scale(sprite.size.size, scale), AlignmentCenter);
    return rect;
}

internal Rect
d_sprite_rect(D_SpriteAtlas atlas, D_SpriteIndex sprite_index, Vec2 pos)
{
    return d_sprite_rect_scaled(atlas, sprite_index, pos, 1);
}

internal inline Vec2
d_sprite_pivot(D_Sprite sprite, Vec2 scale, Vec2 flip)
{
    Vec2 result;
    // TODO(selim): first part needs to be done at startup for all sprites
    result.x = (sprite.size.w / 2.0f + sprite.size.x - sprite.pivot.x) * flip.x * scale.x;
    result.y = -(sprite.size.h / 2.0f + sprite.size.y - sprite.pivot.y) * flip.y * scale.y;
    return result;
}

internal inline D_Animation
animation_from_atlas(D_SpriteAtlas atlas, D_AnimationIndex index)
{
    return atlas.animations[index];
}

internal inline uint32
animation_length(D_Animation animation)
{
    return animation.sprite_end_index - animation.sprite_start_index;
}

internal Rect
sprite_rect(D_SpriteAtlas atlas, D_SpriteIndex sprite)
{
    const D_Sprite* s = &atlas.sprites[sprite];
    // NOTE(selim): -2 is removed because by default all our sprites have 1 px padding on each side
    return rect_from_wh(s->size.w, s->source_size.h);
}

internal Rect
sprite_rect_at(D_SpriteAtlas atlas, D_SpriteIndex sprite, Vec2 position, Vec2 scale, Vec2 flip)
{
    const float32 padding       = 2;
    D_Sprite      sprite_data   = atlas.sprites[sprite];
    Vec2          pivot         = d_sprite_pivot(sprite_data, scale, flip);
    Vec2          actual_scale  = vec2((sprite_data.size.w + padding) * flip.x * scale.x, (sprite_data.size.h + padding) * flip.y * scale.y);
    Vec2          rect_position = add_vec2(position, pivot);
    return rect_at(rect_position, actual_scale, AlignmentCenter);
}
