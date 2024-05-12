#pragma once

#include <base/defines.h>
#include <base/layout.h>
#include <base/math.h>
#include <base/memory.h>
#include <gfx/base.h>
#include <gfx/math.h>

#include "base.h"

typedef uint32 SpriteIndex;
typedef uint32 AnimationIndex;

typedef int8 AnimationState;
enum
{
    AnimationStateLoop   = 1 << 0,
    AnimationStateLocked = 1 << 1,
    AnimationStateAttack = 1 << 2
};

typedef struct
{
    Rect rect;
    Rect size;
    Vec2 pivot;
    Vec2 source_size;
} Sprite;

typedef struct
{
    SpriteIndex sprite_start_index;
    SpriteIndex sprite_end_index;
} Animation;

typedef struct
{
    TextureIndex        texture;
    const Sprite*       sprites;
    const Animation*    animations;
    const TextureIndex* sprite_texture_indices;

    uint32 sprite_count;
    uint32 animation_count;

    float32 width;
    float32 height;
} SpriteAtlas;

internal SpriteAtlas*     sprite_atlas_new(Arena* arena, TextureIndex texture_index, const Animation* animations, const Sprite* sprites, const TextureIndex* texture_indices, uint32 animation_count, uint32 sprite_count);
internal inline Vec2      r_sprite_get_pivot(Sprite sprite, Vec2 scale, Vec2 flip);
internal Bounds           r_sprite_tex_coords(SpriteAtlas* atlas, SpriteIndex sprite_index);
internal Rect             r_sprite_rect_scaled(SpriteAtlas* atlas, SpriteIndex sprite_index, Vec2 pos, float32 scale);
internal Rect             r_sprite_rect(SpriteAtlas* atlas, SpriteIndex sprite_index, Vec2 pos);
internal inline Animation animation_from_atlas(SpriteAtlas* atlas, AnimationIndex index);
internal inline uint16    animation_length(Animation animation);