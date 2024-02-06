#include "ui_core.h"

internal UI_Key
ui_key(String str)
{
    UI_Key result;
    result.value = hash_string(str);
    return result;
}

internal void
ui_state_init(Arena* arena)
{
    ui_state                   = arena_push_struct_zero(arena, UI_State);
    ui_state->persistent_arena = arena;
    ui_state->frame_arena      = make_arena_reserve(mb(32));
}

internal void
ui_state_update()
{
    arena_reset(ui_state->frame_arena);
}

internal Rect
ui_rect()
{
    xassert(ui_state->layout_stack, "there is no active layout!");
    return ui_state->layout_stack->v.r;
}

internal Rect*
ui_rect_ref()
{
    xassert(ui_state->layout_stack, "there is no active layout!");
    return &ui_state->layout_stack->v.r;
}

internal Rect
ui_cut_dynamic(CutSide cut_side, float32 size)
{
    xassert(ui_state->layout_stack, "there are no active layouts!");
    return rect_cut(&ui_state->layout_stack->v.r, size, cut_side);
}

internal Rect
ui_cut_left(float32 size)
{
    return rect_cut_left(&ui_state->layout_stack->v.r, size);
}

internal Rect
ui_cut_right(float32 size)
{
    return rect_cut_right(&ui_state->layout_stack->v.r, size);
}

internal Rect
ui_cut_top(float32 size)
{
    return rect_cut_top(&ui_state->layout_stack->v.r, size);
}

internal Rect
ui_cut_bottom(float32 size)
{
    return rect_cut_bottom(&ui_state->layout_stack->v.r, size);
}

internal void
ui_push_rect(Rect r)
{
    UI_LayoutNode* node = arena_push_struct_zero(ui_state->frame_arena, UI_LayoutNode);
    node->v.r           = r;
    stack_push(ui_state->layout_stack, node);
}

internal void
ui_push_cut(CutSide cut_side, float32 size)
{
    UI_LayoutNode* node = arena_push_struct_zero(ui_state->frame_arena, UI_LayoutNode);
    node->v.r           = ui_cut_dynamic(cut_side, size);
    stack_push(ui_state->layout_stack, node);
}

internal void
ui_pop_layout()
{
    stack_pop(ui_state->layout_stack);
}

internal void
ui_resize_width(float32 w)
{
    ui_state->layout_stack->v.r.w = w;
}

internal void
ui_resize_height(float32 h)
{
    ui_state->layout_stack->v.r.h = h;
}

internal void
ui_shrink(float32 w, float32 h)
{
    ui_state->layout_stack->v.r = rect_shrink(ui_rect(), vec2(w, h));
}

internal void
ui_expand(float32 w, float32 h)
{
    ui_state->layout_stack->v.r = rect_shrink(ui_rect(), vec2(w, h));
}

internal void
ui_debug()
{
    draw_debug_rect(ui_rect());
}