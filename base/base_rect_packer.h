#pragma once

#include "base_defines.h"
#include "base_math.h"
#include "base_memory.h"

typedef struct
{
    Vec2* points;
    int32 point_count;

    Rect* rects;
    int32 rect_count;

    Rect rect;
} RectPacker;

typedef struct
{
    Rect   rect;
    bool32 succeeded;
    int32  idx_left;
    int32  idx_right;
} RectPackerAddResult;

internal RectPacker*         rect_packer_new(Arena* arena, Rect rect);
internal RectPackerAddResult rect_packer_find(RectPacker* rect_packer, float32 width, float32 height);
internal RectPackerAddResult rect_packer_add(RectPacker* rect_packer, float32 width, float32 height);
