#pragma once
#include "base.h"
#include "sprite.h"

// NOTE(selim): when a new batch is created all underlying buffers initialized at this size.
// upon calling `batch_end` buffers are resized to their counts
#define R_DEFAULT_BATCH_VERTEX_CAPACITY 8192

internal void r_batch_push_vertex(Vec2 pos, Vec2 tex_coord, uint64 instance_id);
internal void r_batch_sprite_push(Rect rect, Bounds tex_coord);
internal void r_batch_sprite_push_sprite(SpriteAtlas* atlas, SpriteIndex sprite, Vec2 pos);

// internal void r_batch_triangle_push_vertex(R_BatchNode* batch, Vec2 pos, Vec4 color);
// internal void r_batch_triangle_push_strip(R_BatchNode* batch, Vec2 pos, Vec4 color);