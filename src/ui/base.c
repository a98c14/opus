#include "base.h"

internal UIID
uuid_new(int16 id, int16 owner)
{
    UIID result;
    result.item  = id;
    result.owner = owner;
    return result;
}

internal UIID
uuid_null()
{
    UIID result;
    result.item  = -1;
    result.owner = -1;
    return result;
}

internal bool32
uuid_is_null(UIID id)
{
    return id.item == -1 && id.owner == -1;
}

internal UIContext*
ui_context_new(Arena* arena)
{
    UIContext* ctx        = arena_push_struct_zero(arena, UIContext);
    ctx->active           = uuid_null();
    ctx->sort_layer_index = SORT_LAYER_INDEX_UI;
    return ctx;
}

internal bool32
ui_is_active_any(UIContext* ctx)
{
    return ctx->active.item >= 0 || ctx->active.owner >= 0;
}

internal void
ui_set_sort_layer(UIContext* ctx, SortLayerIndex sort_layer)
{
    ctx->sort_layer_index = sort_layer;
}

internal bool32
ui_is_free(UIContext* ctx)
{
    return !ui_is_active_any(ctx);
}

internal bool32
ui_is_active(UIContext* ctx, UIID id)
{
    return ctx->active.item == id.item && ctx->active.owner == id.owner;
}

internal void
ui_activate(UIContext* ctx, UIID id)
{
    ctx->active          = id;
    ctx->activation_time = ctx->time.current_frame;
}

internal void
ui_active_clear(UIContext* ctx)
{
    ctx->activation_time = 0;
    ctx->active          = uuid_null();
    ctx->drag_offset     = vec2_zero();
}

internal float32
ui_active_time(UIContext* ctx)
{
    return ctx->time.current_frame - ctx->activation_time;
}

internal bool32
ui_is_hot(UIContext* ctx, UIID id)
{
    return ctx->hot.item == id.item && ctx->hot.owner == id.owner;
}