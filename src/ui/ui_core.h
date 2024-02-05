#pragma once

#include <base.h>
#include <engine/draw.h>
#include <engine/layout.h>

typedef struct
{
    Rect r;
} UI_Layout;

typedef struct UI_LayoutNode UI_LayoutNode;
struct UI_LayoutNode
{
    UI_Layout v;

    UI_LayoutNode* next;
};

typedef struct
{
    Arena* persistent_arena;
    Arena* frame_arena;

    UI_LayoutNode* layout_stack;
} UI_State;

global UI_State* ui_state;

internal void  ui_state_init(Arena* arena);
internal void  ui_state_update();
internal Rect  ui_rect();
internal Rect* ui_rect_ref();
internal Rect  ui_cut(CutSide cut_side, float32 size);
internal Rect  ui_cut_left(float32 size);
internal Rect  ui_cut_right(float32 size);
internal Rect  ui_cut_top(float32 size);
internal Rect  ui_cut_bottom(float32 size);
internal void  ui_push_rect(Rect r);
internal void  ui_push_cut(CutSide cut_side, float32 size);
internal void  ui_pop_layout();
#define ui_rect_scope(rect)          defer_loop(ui_push_rect(rect), ui_pop_layout())
#define ui_cut_scope(cut_side, size) defer_loop(ui_push_cut(cut_side, size), ui_pop_layout())

/** helpers */
internal void ui_resize_width(float32 w);
internal void ui_resize_height(float32 h);
internal void ui_shrink(float32 w, float32 h);
internal void ui_expand(float32 w, float32 h);
internal void ui_debug();
