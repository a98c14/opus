#pragma once
#include "../base/base_inc.h"
#include "../draw/draw_inc.h"
#include "../input/input_inc.h"

#include "ui_inputs.h"

#define UI_ANIMATION_UPDATE_RATE 24.0f
#define UI_INPUT_BUFFER_SIZE     32
#define UI_HASH_MAP_CAPACITY     4096

global read_only String _ui_id_separator = string_comp("##");

typedef struct
{
    uint64 value;
} UI_Key;

typedef struct
{
    UI_Key key;
    String label;
} UI_KeyFromLabelResult;

global read_only UI_Key ui_key_null = {0};

typedef struct
{
    float32 click_t;
    float32 hover_t;

    bool32 is_hot;
    bool32 is_warm;

    bool32 clicked;

    Rect rect;
} UI_Signal;

typedef enum
{
    UI_ElementKind_Undefined = 0,
    UI_ElementKind_Clickable = 1 << 1,
    UI_ElementKind_Container = 1 << 2,
} UI_ElementKind;

typedef enum
{
    UI_SizeKind_FitContents,
    UI_SizeKind_Fixed,
    UI_SizeKind_SumOfChildren,
    UI_SizeKind_Percentage,
    UI_SizeKind_COUNT
} UI_SizeKind;

typedef enum
{
    UI_AxisVertical   = 0,
    UI_AxisHorizontal = 1,
    UI_AxisCOUNT
} UI_Axis;

typedef struct UI_Entity UI_Entity;
struct UI_Entity
{
    UI_Entity* next;
    UI_Entity* prev;
    UI_Entity* parent;
    UI_Entity* first_child;
    UI_Entity* last_child;

    uint32 child_count;

#if BUILD_DEBUG
    String debug_str;
#endif
    UI_ElementKind kind;
    UI_Key         key;

    /** cursor */
    Vec2 cursor;

    /** events */
    float32 hot_t;
    float32 click_t;

    /** render */
    Rect rect;
    Rect inner_rect;
    Rect outer_rect;

    /** styling */
    String text;
    Color  bg_color;
    Color  highlight_color;
    Color  fg_color;

    /** layout */
    bool32 grow[UI_AxisCOUNT];

    /** translation */
    UI_SizeKind size_kind;
    Vec2        pos;
    Vec2        size;
    UI_Axis     direction;

    Vec2 margin;
    Vec2 padding;
};

read_only global UI_Entity ui_entity_nil = {
    &ui_entity_nil,
    &ui_entity_nil,
    &ui_entity_nil,
    &ui_entity_nil,
};

typedef struct UI_EntityNode UI_EntityNode;
struct UI_EntityNode
{
    UI_EntityNode* next;
    UI_EntityNode* prev;

    UI_Entity* value;
};

typedef struct
{
    UI_EntityNode* first;
    UI_EntityNode* last;
} UI_EntityList;

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

typedef struct
{
    Arena* persistent_arena;

    Arena* frame_arena;
    Arena* next_frame_arena;

    Arena* available_frame_arenas[2];
    uint64 frame;

    UI_Entity* root;
    UI_Entity* active_element;
    UI_Entity* active_parent;

    UI_EntityList hash_map[UI_HASH_MAP_CAPACITY];

    UI_Key hot;
    UI_Key active;

    float32 update_t;
    float32 hot_t;
    float32 active_t;

    Vec2 active_initial_pos;
    Vec2 active_offset;

    UI_LayoutNode* layout_stack;

    D_SpriteAtlas* sprite_atlas;

} UI_Context;

global UI_Context* ui_ctx;
global float32     ui_line_height;

/** UI Keys */
global String key_select;

internal UI_Key                ui_key_str(String str);
internal UI_Key                ui_key_cstr(char* str);
internal UI_KeyFromLabelResult ui_key_from_label(String label);
internal UI_Key                ui_key(uint64 v);
internal bool32                ui_key_same(UI_Key a, UI_Key b);

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
internal void ui_pop();

#define ui_create_fixed(rect)                     defer_loop(ui_push_rect(ui_key_null, rect), ui_pop())
#define ui_create(cut_side, size)                 defer_loop(ui_push_cut(ui_key_null, cut_side, size), ui_pop())
#define ui_create_with_key(key, cut_side, size)   defer_loop(ui_push_cut(key, cut_side, size), ui_pop())
#define ui_create_with_name(name, cut_side, size) defer_loop(ui_push_cut(ui_key_cstr(name), cut_side, size), ui_pop())

/** helpers */
internal void ui_resize_width(float32 w);
internal void ui_resize_height(float32 h);
internal void ui_shrink(float32 w, float32 h);
internal void ui_expand(float32 w, float32 h);
internal void ui_debug();
internal Rect ui_sprite_rect(D_SpriteIndex sprite);
internal Rect ui_animation_rect(D_AnimationIndex animation);

/** common widgets */
internal void      ui_pad(float32 x);
internal void      ui_fill(Color c);
internal void      ui_border(Color c, float32 thickness);
internal UI_Signal ui_slider(String label, float32 min, float32 max, float32* value);
internal UI_Signal ui_slider_int(String label, int32 min_value, int32 max_value, float32* value);

internal UI_Signal ui_text(String str);
internal UI_Signal ui_textf(const char* fmt, ...);

/** V2 */
internal UI_Entity* ui_entity_new(UI_ElementKind kind);
internal void       ui_entity_add_to_ui(UI_Entity* entity);
internal UI_Entity* ui_entity_init(UI_ElementKind kind);
internal UI_Entity* ui_entity_init_widget(UI_ElementKind kind);
internal void       ui_entity_add_to_list(UI_Entity* entity, UI_EntityList* list);

internal void _ui_entity_init_root(void);

/** V2 Widgets */
internal void ui_begin_vertical();
internal void ui_begin_horizontal();
internal void ui_end();

internal UI_Signal ui_button(String label);
internal UI_Signal ui_label(String label);

internal void ui_set_wh(float32 w, float32 h);
internal void ui_set_bg_color(Color color);
internal void ui_set_margin(float32 x, float32 y);
