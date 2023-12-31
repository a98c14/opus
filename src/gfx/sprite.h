#pragma once

#include <core/defines.h>
#include <core/math.h>
#include <core/memory.h>
#include <gfx/base.h>

#include "base.h"

typedef uint32 SpriteIndex;
typedef uint32 AnimationIndex;

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
} SpriteAtlas;

internal SpriteAtlas* sprite_atlas_new(Arena* arena, TextureIndex texture_index, const Animation* animations, const Sprite* sprites, const TextureIndex* texture_indices, uint32 animation_count, uint32 sprite_count);
internal void         renderer_load_sprite_atlas(Renderer* renderer, SpriteAtlas* atlas);
internal inline Vec2  sprite_get_pivot(Sprite sprite, Vec2 scale, Vec2 flip);