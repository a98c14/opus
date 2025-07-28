#include "base_rect_packer.h"

internal RectPacker*
rect_packer_new(Arena* arena, Rect rect, float32 padding)
{
    RectPacker* result = arena_push_struct_zero(arena, RectPacker);
    result->points     = arena_push_array_zero(arena, Vec2, (int32)rect.w);
    result->rect       = rect;
    result->rects      = arena_push_array_zero(arena, Rect, 4096); // TODO: maybe this should be a linked list

    result->point_count = 1;
    result->points[0]   = rect_bl(rect);
    result->padding     = padding;

    return result;
}

internal RectPackerAddResult
rect_packer_find(RectPacker* rect_packer, float32 width, float32 height)
{
    RectPackerAddResult result = {0};

    float32 min_y = MAX_FLOAT32;
    float32 min_x = MAX_FLOAT32;

    result.idx_left  = MAX_INT32;
    result.idx_right = MAX_INT32;

    for (int32 i = 0; i < rect_packer->point_count; i++)
    {
        Vec2 point = rect_packer->points[i];

        if (width > rect_right(rect_packer->rect) - point.x)
            break;

        if (point.y >= min_y)
            continue;

        int32 k;
        for (k = i + 1; k < rect_packer->point_count; k++)
        {
            Vec2 next_point = rect_packer->points[k];

            if (next_point.x >= point.x + width)
                break;

            if (point.y < next_point.y)
                point.y = next_point.y;
        }

        if (point.y > min_y)
            continue;

        if (height > rect_top(rect_packer->rect) - point.y)
            continue;

        min_y            = point.y;
        min_x            = point.x;
        result.idx_left  = i;
        result.idx_right = k;
    }

    result.succeeded = result.idx_right != MAX_INT32;
    result.rect      = rect_from_bl_tr(
        vec2(min_x, min_y),
        vec2(min_x + width, min_y + height));

    return result;
}

internal RectPackerAddResult
rect_packer_add(RectPacker* rect_packer, float32 width, float32 height)
{
    RectPackerAddResult result = rect_packer_find(rect_packer, width, height);

    if (!result.succeeded)
    {
        log_warn("Couldn't fit given rect");
        return result;
    }

    int32 removed_count  = result.idx_right - result.idx_left;
    int32 inserted_count = 1 + (result.idx_right < rect_packer->point_count
                                    ? rect_right(result.rect) < rect_packer->points[result.idx_right].x
                                    : rect_right(result.rect) < rect_packer->rect.w);

    int32 count_diff = inserted_count - removed_count;
    if (count_diff != 0)
    {
        int32 src_idx     = result.idx_left + (count_diff > 0 ? 0 : 2);
        int32 dst_idx     = result.idx_left + 1;
        int32 shift_count = rect_packer->point_count - src_idx;

        memory_copy_typed(&rect_packer->points[dst_idx],
                          &rect_packer->points[src_idx],
                          shift_count);
    }

    rect_packer->points[result.idx_left]          = rect_tl(result.rect);
    rect_packer->points[result.idx_left + 1]      = rect_br(result.rect);
    rect_packer->point_count                      = rect_packer->point_count + count_diff;
    rect_packer->rects[rect_packer->rect_count++] = result.rect;

    return result;
}