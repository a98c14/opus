#include "base_geometry.h"

internal PointArray
geo_generate_circle_points(Arena* arena, float32 r)
{
    int32 t1 = (int32)(r / 16);
    int32 t2 = 0;
    int32 x  = (int32)r;
    int32 y  = 0;

    ArenaTemp temp               = scratch_begin(&arena, 1);
    Vec2*     temp_offset_buffer = arena_push_array_zero(temp.arena, Vec2, 2048); // TODO: calculate a better temp pixel count
    uint64    actual_count       = 0;
    while (x > y)
    {
        for (int32 i = -x; i <= x; i++)
        {
            temp_offset_buffer[actual_count++] = (Vec2){.x = (float32)i, .y = (float32)y};
            temp_offset_buffer[actual_count++] = (Vec2){.x = (float32)i, .y = (float32)y};
            temp_offset_buffer[actual_count++] = (Vec2){.x = (float32)i, .y = (float32)-y};
            temp_offset_buffer[actual_count++] = (Vec2){.x = (float32)-i, .y = (float32)-y};
        }

        for (int32 i = -x; i <= x; i++)
        {
            temp_offset_buffer[actual_count++] = (Vec2){.x = (float32)y, .y = (float32)i};
            temp_offset_buffer[actual_count++] = (Vec2){.x = (float32)-y, .y = (float32)i};
            temp_offset_buffer[actual_count++] = (Vec2){.x = (float32)y, .y = (float32)-i};
            temp_offset_buffer[actual_count++] = (Vec2){.x = (float32)-y, .y = (float32)-i};
        }

        y  = y + 1;
        t1 = t1 + y;
        t2 = t1 - x;
        if (t2 >= 0)
        {
            t1 = t2;
            x  = x - 1;
        }
    }

    PointArray result = {0};
    result.points     = arena_push_array(arena, Vec2, actual_count);
    result.count      = actual_count;

    memory_copy_typed(result.points, temp_offset_buffer, actual_count);

    return result;
}