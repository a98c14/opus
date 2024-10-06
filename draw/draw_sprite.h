#pragma once

#include "../base/base_inc.h"
#include "../gfx/gfx_inc.h"

typedef uint32 D_SpriteIndex;
typedef uint32 D_AnimationIndex;

typedef int8 D_AnimationState;
enum
{
    D_AnimationStateLoop   = 1 << 0,
    D_AnimationStateLocked = 1 << 1,
    D_AnimationStateAttack = 1 << 2
};

typedef struct
{
    Rect rect;
    Rect size;
    Vec2 pivot;
    Vec2 source_size;
} D_Sprite;

typedef struct
{
    D_SpriteIndex sprite_start_index;
    D_SpriteIndex sprite_end_index;
} D_Animation;

typedef struct
{
    TextureIndex        texture;
    const D_Sprite*     sprites;
    const D_Animation*  animations;
    const TextureIndex* sprite_texture_indices;

    uint32 sprite_count;
    uint32 animation_count;

    float32 width;
    float32 height;
} D_SpriteAtlas;

/** atlas */
internal D_SpriteAtlas d_sprite_atlas_new(TextureIndex texture_index, const D_Animation* animations, const D_Sprite* sprites, const TextureIndex* texture_indices, uint32 animation_count, uint32 sprite_count);

/** sprite */
internal Vec2   d_sprite_pivot(D_Sprite sprite, Vec2 scale, Vec2 flip);
internal Bounds d_sprite_tex_coords(D_SpriteAtlas atlas, D_SpriteIndex sprite_index);
internal Rect   d_sprite_rect_at(D_SpriteAtlas atlas, D_SpriteIndex sprite, Vec2 position, Vec2 scale, Vec2 flip);
internal Rect   d_sprite_rect_scaled(D_SpriteAtlas atlas, D_SpriteIndex sprite_index, Vec2 pos, float32 scale);
internal Rect   d_sprite_rect(D_SpriteAtlas atlas, D_SpriteIndex sprite_index, Vec2 pos);
internal Rect   d_sprite_rect_base(D_SpriteAtlas atlas, D_SpriteIndex sprite);

/** animation */
internal inline D_Animation d_animation_from_atlas(D_SpriteAtlas atlas, D_AnimationIndex index);
internal inline uint32      animation_length(D_Animation animation);
