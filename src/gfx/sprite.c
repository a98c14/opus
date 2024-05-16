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

internal Rect
sprite_rect(SpriteAtlas* atlas, SpriteIndex sprite)
{
    const Sprite* s = &atlas->sprites[sprite];
    // NOTE(selim): -2 is removed because by default all our sprites have 1 px padding on each side
    return rect_from_wh(s->source_size.w - 2, s->source_size.h - 2);
}

internal Rect
sprite_rect_with_pivot(SpriteAtlas* atlas, SpriteIndex sprite, Vec2 position, Vec2 flip, float32 scale_multiplier)
{
    Sprite sprite_data   = atlas->sprites[sprite];
    Vec2   pivot         = r_sprite_get_pivot(sprite_data, vec2(scale_multiplier, scale_multiplier), flip);
    Vec2   scale         = mul_vec2_f32(vec2(sprite_data.size.w * flip.x, sprite_data.size.h * flip.y), scale_multiplier);
    Vec2   rect_position = add_vec2(position, pivot);
    return rect_at(rect_position, fabs_vec2(scale), AlignmentCenter);
}
