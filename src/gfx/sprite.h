#include <core/defines.h>
#include <core/math.h>

typedef struct
{
    Rect rect;
    Rect size;
    Vec2 pivot;
    Vec2 source_size;
} Sprite;

typedef struct
{
    int sprite_start_index;
    int sprite_end_index;
}  Animation;