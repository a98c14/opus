#pragma once
#include "../base/base_inc.h"
#include "../draw/draw_inc.h"
#include "../input/input_inc.h"

#include "ui_inputs.h"

#define UI_MAX_ANIMATION_COUNT   16
#define UI_ANIMATION_UPDATE_RATE 24.0f
#define UI_INPUT_BUFFER_SIZE     32

global read_only String _ui_id_separator = string_comp("###");

typedef struct
{
    uint64 value;
} UI_Key;

global read_only UI_Key ui_key_null = {0};

typedef struct
{
    float32 click_t;
    float32 hover_t;
} UI_Signal;

typedef struct
{
    UI_Key key;
    Rect   r;
} UI_Layout;

typedef struct UI_LayoutNode UI_LayoutNode;
struct UI_LayoutNode
{
    UI_Layout v;

    UI_LayoutNode* next;
};

/** Animation */
typedef struct
{
    UI_Key  owner;
    float32 started_at;
    float32 updated_at;

    Rect             last_rect;
    D_AnimationIndex exit_animation;
} UI_SpriteAnimator;

typedef struct UI_Entity UI_Entity;
struct UI_Entity
{
    UI_Entity* next;
    UI_Entity* prev;

    Vec2 handle_initial_position;
    Vec2 handle_current_position;
};

typedef struct
{
    Arena* persistent_arena;
    Arena* frame_arena;
    uint64 frame;

    UI_Key hot;
    UI_Key active;

    float32 update_t;
    float32 hot_t;
    float32 active_t;

    Vec2 hot_initial_pos;

    UI_LayoutNode* layout_stack;

    /** animation */
    D_SpriteAtlas* sprite_atlas;

    /** animation_ring_buffer */
    UI_SpriteAnimator active_animations[UI_MAX_ANIMATION_COUNT];
} UI_Context;

global UI_Context* ui_ctx;

internal UI_Key ui_key_str(String str);
internal UI_Key ui_key_cstr(char* str);
internal UI_Key ui_key_from_label(String label);
internal UI_Key ui_key(uint64 v);
internal bool32 ui_key_same(UI_Key a, UI_Key b);

internal void   ui_init(Arena* arena);
internal void   ui_update(float32 dt);
internal bool32 ui_is_hot(UI_Key key);

internal void ui_state_load_atlas(D_SpriteAtlas* atlas);
internal void ui_set_key(UI_Key key);

internal Rect  ui_rect();
internal Rect* ui_rect_ref();
internal void  ui_rect_set(Rect r);
internal Rect  ui_cut_dynamic(CutSide cut_side, float32 size);
internal Rect  ui_cut_left(float32 size);
internal Rect  ui_cut_right(float32 size);
internal Rect  ui_cut_top(float32 size);
internal Rect  ui_cut_bottom(float32 size);

/** stack management */
internal void ui_push_rect(UI_Key key, Rect r);
internal void ui_push_cut(UI_Key key, CutSide cut_side, float32 size);
internal void ui_pop_layout();

#define ui_create_fixed(rect)                     defer_loop(ui_push_rect(ui_key_null, rect), ui_pop_layout())
#define ui_create(cut_side, size)                 defer_loop(ui_push_cut(ui_key_null, cut_side, size), ui_pop_layout())
#define ui_create_with_key(key, cut_side, size)   defer_loop(ui_push_cut(key, cut_side, size), ui_pop_layout())
#define ui_create_with_name(name, cut_side, size) defer_loop(ui_push_cut(ui_key_cstr(name), cut_side, size), ui_pop_layout())

/** helpers */
internal void ui_resize_width(float32 w);
internal void ui_resize_height(float32 h);
internal void ui_shrink(float32 w, float32 h);
internal void ui_expand(float32 w, float32 h);
internal void ui_debug();
internal Rect ui_sprite_rect(D_SpriteIndex sprite);
internal Rect ui_animation_rect(D_AnimationIndex animation);

/** animation */
internal UI_SpriteAnimator* ui_animator_find(UI_Key key);
internal UI_SpriteAnimator* ui_animator_reserve(UI_Key key);
internal UI_SpriteAnimator* ui_animator_get(UI_Key key);

/** common widgets */
internal void      ui_pad(float32 x);
internal void      ui_fill(Color c);
internal UI_Signal ui_slider(String label, float32 min, float32 max, float32* value);