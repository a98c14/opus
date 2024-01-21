#pragma once

#include <core/defines.h>
#include <engine/draw.h>
#include <engine/input.h>
#include <engine/time.h>
#include <physics/intersection.h>
#include <theme/base.h>

#include "debug.h"

#define UI_DEBUG            1
#define UI_FRAME_CAPACITY   256
#define DEFAULT_LINE_HEIGHT 20
#define SORT_LAYER_INDEX_UI 13

typedef uint8 FrameIndex;

typedef struct
{
    int16 owner;
    int16 item;
} UIID;

typedef struct
{
    EngineTime time;
    InputMouse mouse;

    /* State */
    UIID active;
    UIID hot;

    float32 activation_time;
    Vec2    drag_start;
    Vec2    drag_offset;
} UIContext;

internal UIID       uuid_new(int16 id, int16 owner);
internal UIID       uuid_null();
internal bool32     uuid_is_null(UIID id);
internal UIContext* ui_context_new(Arena* arena);
internal bool32     ui_is_active(UIContext* ctx, UIID id);
internal bool32     ui_is_active_any(UIContext* ctx);

internal bool32  ui_is_free(UIContext* ctx);
internal void    ui_activate(UIContext* ctx, UIID id);
internal void    ui_active_clear(UIContext* ctx);
internal float32 ui_active_time(UIContext* ctx);
internal bool32  ui_is_hot(UIContext* ctx, UIID id);