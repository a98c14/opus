#include "ui_core.h"

internal UI_Key
ui_key_str(String str)
{
    UI_Key result;
    result.value = hash_string(str);
    return result;
}

internal UI_Key
ui_key_cstr(char* str)
{
    UI_Key result;
    result.value = hash_chars(str);
    return result;
}

internal UI_Key
ui_key_from_label(String label)
{
    ArenaTemp  temp  = scratch_begin(0, 0);
    StringList parts = string_split(temp.arena, label, _ui_id_separator);
    UI_Key     key   = {0};

    key = parts.count > 1
              ? ui_key_str(parts.first->next->value)
              : ui_key_str(parts.first->value);
    return key;
}

internal UI_Key
ui_key(uint64 v)
{
    UI_Key result;
    result.value = v;
    return result;
}

internal bool32
ui_key_same(UI_Key a, UI_Key b)
{
    return a.value == b.value;
}

internal void
ui_init(Arena* arena)
{
    ui_ctx                   = arena_push_struct_zero(arena, UI_Context);
    ui_ctx->persistent_arena = arena;
    ui_ctx->frame_arena      = arena_new_reserve(mb(32));

    _ui_entity_init_root();

    key_select = string("ui_select");

    input_register_key(key_select, OS_KeyCode_MouseLeft);

    ui_line_height = roundf(em(0.8f));
}

internal void
ui_state_load_atlas(D_SpriteAtlas* atlas)
{
    ui_ctx->sprite_atlas = atlas;
}

internal void
ui_update(float32 dt)
{
    //** Layout Calculations */
    UI_EntityNode* first = 0;
    UI_EntityNode* last  = 0;

    UI_EntityNode* nodes_to_process = 0;
    UI_EntityNode* process_node     = arena_push_struct_zero(ui_ctx->frame_arena, UI_EntityNode);
    process_node->value             = ui_ctx->root;

    /** Traverse the nodes */
    while (process_node != 0)
    {
        UI_Entity* e = process_node->value;

        for (UI_Entity* child = e->first_child; child != &ui_entity_nil; child = child->next)
        {
            UI_EntityNode* process_node = arena_push_struct_zero(ui_ctx->frame_arena, UI_EntityNode);
            process_node->value         = child;
            stack_push(nodes_to_process, process_node);
        }

        UI_EntityNode* node = arena_push_struct_zero(ui_ctx->frame_arena, UI_EntityNode);
        node->value         = e;
        dll_push_back(first, last, node);

        process_node = nodes_to_process;
        stack_pop(nodes_to_process);
    }

    /** Layout */
    for (UI_EntityNode* node = first; node != 0; node = node->next)
    {
        UI_Entity* e          = node->value;
        bool32     has_parent = e->parent != &ui_entity_nil;

        if (has_parent && e->rect.w == 0)
        {
            e->rect.w = e->parent->rect.w;
        }

        if (has_parent && e->rect.h == 0)
        {
            e->rect.h = ui_line_height * 2;
        }

        // e->rect.w += e->padding.w;
        // e->rect.h += e->padding.h;

        if (has_parent)
        {
            bool32 is_horizontal = e->parent->direction == UI_AxisHorizontal;

            Vec2 cursor_move = {0};

            if (!is_horizontal)
            {
                // e->rect.h     = e->parent->rect.h / e->parent->child_count;
                cursor_move.h = -e->rect.h;
            }
            else
            {
                // e->rect.w     = e->parent->rect.w / e->parent->child_count;
                cursor_move.x = e->rect.w;
            }

            e->rect = rect_at_o(e->parent->cursor, e->rect.size, e->pos, AlignmentTopLeft);

            e->parent->cursor = add_vec2(e->parent->cursor, cursor_move);
            e->parent->cursor = add_vec2(e->parent->cursor, e->pos);

            if (e->parent->cursor.x > rect_right(e->parent->rect))
            {
                float32 grow = e->parent->cursor.x - rect_right(e->parent->rect);
                e->parent->rect.w += grow;
                e->parent->rect.x += grow / 2;
            }
        }

        e->cursor = rect_tl(e->rect);
    }

    /** Events */
    Input_MouseInfo mouse = input_mouse_info();
    for (UI_EntityNode* node = last; node != 0; node = node->prev)
    {
        UI_Entity* e = node->value;

        Intersection intersection = intersects_rect_point(e->rect, mouse.screen);

        bool32 is_clickable = flag_is_set(e->kind, UI_ElementKind_Clickable);
        if (is_clickable && intersection.intersects && input_is_held(ui_input_select))
        {
            e->click_t = 1;
        }

        if (is_clickable && intersection.intersects)
        {
            e->hot_t = 1;
            break;
        }
    }

    /** Render */
    for (UI_EntityNode* node = first; node != 0; node = node->next)
    {
        UI_Entity* e = node->value;

        Color bg_color = e->hot_t > 0 ? e->highlight_color : e->bg_color;
        bg_color       = e->click_t > 0 ? ColorRed400 : bg_color;

        d_ui_element(e->rect, bg_color, 1, vec4_xxxx(5.0));
        if (!string_is_empty(e->text))
        {
            Rect inner_rect = rect_shrink(e->rect, e->padding);
            d_string(inner_rect, e->text, ui_line_height, e->fg_color, ANCHOR_L_L);
        }
    }

    ui_ctx->frame++;
    ui_ctx->update_t += dt;

    arena_reset(ui_ctx->frame_arena);

    _ui_entity_init_root();
}

internal bool32
ui_is_hot(UI_Key key)
{
    return ui_key_same(key, ui_ctx->hot);
}

internal bool32
ui_is_active(UI_Key key)
{
    return ui_key_same(key, ui_ctx->active);
}

internal void
ui_set_key(UI_Key key)
{
    ui_ctx->layout_stack->v.key = key;
}

internal Rect
ui_rect(void)
{
    xassert_m(ui_ctx->layout_stack, "there is no active layout!");
    return ui_ctx->layout_stack->v.r;
}

internal Rect*
ui_rect_ref(void)
{
    xassert_m(ui_ctx->layout_stack, "there is no active layout!");
    return &ui_ctx->layout_stack->v.r;
}
internal void
ui_rect_set(Rect r)
{
    xassert_m(ui_ctx->layout_stack, "there is no active layout!");
    ui_ctx->layout_stack->v.r = r;
}

internal Rect
ui_cut_dynamic(CutSide cut_side, float32 size)
{
    xassert_m(ui_ctx->layout_stack, "there is no active layout!");
    return rect_cut(&ui_ctx->layout_stack->v.r, size, cut_side);
}

internal Rect
ui_cut_left(float32 size)
{
    return rect_cut_left(&ui_ctx->layout_stack->v.r, size);
}

internal Rect
ui_cut_right(float32 size)
{
    return rect_cut_right(&ui_ctx->layout_stack->v.r, size);
}

internal Rect
ui_cut_top(float32 size)
{
    return rect_cut_top(&ui_ctx->layout_stack->v.r, size);
}

internal Rect
ui_cut_bottom(float32 size)
{
    return rect_cut_bottom(&ui_ctx->layout_stack->v.r, size);
}

internal void
ui_push_rect(UI_Key key, Rect r)
{
    UI_LayoutNode* node = arena_push_struct_zero(ui_ctx->frame_arena, UI_LayoutNode);
    node->v.r           = r;
    node->v.key         = key;
    stack_push(ui_ctx->layout_stack, node);
}

internal void
ui_push_cut(UI_Key key, CutSide cut_side, float32 size)
{
    UI_LayoutNode* node = arena_push_struct_zero(ui_ctx->frame_arena, UI_LayoutNode);
    node->v.r           = ui_cut_dynamic(cut_side, size);
    node->v.key         = key;
    stack_push(ui_ctx->layout_stack, node);
}

internal void
ui_pop(void)
{
    xassert_m(ui_ctx->layout_stack, "there is no active layout!");
    stack_pop(ui_ctx->layout_stack);
}

internal void
ui_resize_width(float32 w)
{
    ui_ctx->layout_stack->v.r.w = w;
}

internal void
ui_resize_height(float32 h)
{
    ui_ctx->layout_stack->v.r.h = h;
}

internal void
ui_shrink(float32 w, float32 h)
{
    ui_ctx->layout_stack->v.r = rect_shrink(ui_rect(), vec2(w, h));
}

internal void
ui_expand(float32 w, float32 h)
{
    ui_ctx->layout_stack->v.r = rect_shrink(ui_rect(), vec2(w, h));
}

internal void
ui_debug(void)
{
    // draw_debug_rect(ui_rect());
}

internal Rect
ui_sprite_rect(D_SpriteIndex sprite)
{
    const D_Sprite* s = &ui_ctx->sprite_atlas->sprites[sprite];
    // NOTE(selim): -2 is removed because by default all our sprites have 1 px padding on each side
    return rect_from_wh(s->size.w - 2, s->size.h - 2);
}

internal Rect
ui_animation_rect(D_AnimationIndex animation)
{
    const D_Animation* a        = &ui_ctx->sprite_atlas->animations[animation];
    Rect               max_rect = {0};
    for (uint64 i = a->sprite_start_index; i < a->sprite_end_index; i++)
    {
        const D_Sprite* s = &ui_ctx->sprite_atlas->sprites[i];
        max_rect.w        = max(max_rect.w, s->size.w - 2);
        max_rect.h        = max(max_rect.h, s->size.h - 2);
    }
    return max_rect;
}

/** common widgets */
internal void
ui_pad(float32 x)
{
    Rect r = ui_rect();
    ui_rect_set(rect_shrink(r, vec2(x, x)));
}

internal void
ui_fill(Color c)
{
    Rect r = ui_rect();
    d_rect(r, 0, c);
}

internal void
ui_border(Color c, float32 thickness)
{
    Rect r = ui_rect();
    d_rect(r, thickness, c);
}

internal UI_Signal
ui_slider(String label, float32 min_value, float32 max_value, float32* value)
{
    // make sure value is always inside the bounds
    *value = clamp(min_value, *value, max_value);

    ArenaTemp temp                = scratch_begin(0, 0);
    Color     slider_handle_color = ColorSlate100;
    UI_Key    key                 = ui_key_from_label(label);

    // layout
    ui_push_cut(key, CutSideTop, ui_line_height);
    Rect root_container = ui_rect();

    Rect label_container = d_string(root_container, label, ui_line_height, ColorWhite, ANCHOR_L_L);
    rect_cut_left(&root_container, label_container.w);
    Rect value_container = d_string(root_container, string_pushf(temp.arena, "%6.2f", *value), ui_line_height, ColorWhite, ANCHOR_L_L);
    rect_cut_left(&root_container, value_container.w);
    Rect outer_bar = rect_shrink(root_container, vec2(16, 2));
    Rect inner_bar = rect_shrink(outer_bar, vec2(4, 4));

    float32 bar_x           = remap_f32(min_value, max_value, rect_left(inner_bar), rect_right(inner_bar), *value);
    Vec2    handle_position = vec2(bar_x, inner_bar.y);

    Circle handle = circle(handle_position, ui_line_height);
    ui_pop();

    // controls
    Input_MouseInfo mouse               = input_mouse_info();
    UI_Signal       result              = {0};
    Intersection    handle_intersection = intersects_circle_point(handle, mouse.screen);
    if (handle_intersection.intersects)
    {
        ui_ctx->hot    = key;
        result.is_warm = true;
    }
    else
    {
        ui_ctx->hot = ui_key_null;
    }

    if (ui_is_hot(key))
    {
        slider_handle_color = ColorSlate400;
    }

    if (ui_is_hot(key) && input_is_pressed(ui_input_select))
    {
        ui_ctx->active_offset      = sub_vec2(mouse.screen, handle.center);
        ui_ctx->active_initial_pos = handle.center;
        ui_ctx->active             = key;
    }

    if (ui_is_active(key))
    {
        *value        = remap_f32(rect_left(inner_bar), rect_right(inner_bar), min_value, max_value, mouse.screen.x);
        *value        = clamp(min_value, *value, max_value);
        result.is_hot = true;
    }

    if (ui_is_active(key) && !input_is_held(ui_input_select))
    {
        ui_ctx->active = ui_key_null;
    }

    // draw
    d_rect(outer_bar, 0, ColorSlate300);
    d_rect(inner_bar, 0, ColorSlate500);
    d_circle(handle.center, handle.radius, 1, slider_handle_color);

    scratch_end(temp);
    return result;
}

internal UI_Signal
ui_slider_int(String label, int32 min_value, int32 max_value, float32* value)
{
    UI_Signal result = ui_slider(label, (float32)min_value, (float32)max_value, value);
    if (!result.is_hot)
    {
        *value = roundf(*value);
    }
    return result;
}

internal UI_Signal
ui_text(String str)
{
    UI_Signal result = {0};

    Rect container   = ui_cut_top(ui_line_height);
    Rect string_area = d_string(container, str, ui_line_height, ColorWhite, ANCHOR_L_L);

    result.rect = string_area;

    return result;
}

internal UI_Signal
ui_textf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String result = string_pushfv(ui_ctx->frame_arena, fmt, args);
    va_end(args);

    return ui_text(result);
}

internal UI_Entity*
ui_entity_new(UI_ElementKind kind)
{
    UI_Entity* result;

    result = arena_push_struct_zero(ui_ctx->frame_arena, UI_Entity);

    result->next        = &ui_entity_nil;
    result->prev        = &ui_entity_nil;
    result->first_child = &ui_entity_nil;
    result->last_child  = &ui_entity_nil;
    result->parent      = &ui_entity_nil;

    result->key  = ui_key_null;
    result->kind = kind;

    result->bg_color        = ColorSlate400;
    result->highlight_color = ColorSlate200;
    result->fg_color        = ColorBlack;

    result->padding = vec2(8, 8);

    return result;
}

internal void
ui_entity_add_to_ui(UI_Entity* entity)
{
    if (ui_ctx->active_parent != &ui_entity_nil)
    {
        entity->parent = ui_ctx->active_parent;
        dll_push_front_z(&ui_entity_nil, entity->parent->first_child, entity->parent->last_child, entity);
        entity->parent->child_count++;
    }

    if (flag_is_set(entity->kind, UI_ElementKind_Container))
    {
        ui_ctx->active_parent = entity;
    }

    ui_ctx->active_element = entity;
}

internal UI_Entity*
ui_entity_init(UI_ElementKind kind)
{
    UI_Entity* entity = ui_entity_new(kind);
    ui_entity_add_to_ui(entity);
    return entity;
}

internal void
_ui_entity_init_root()
{
    ui_ctx->root           = ui_entity_new(UI_ElementKind_Container);
    ui_ctx->root->bg_color = 0;
    ui_ctx->root->rect     = screen_rect();
    ui_ctx->active_element = ui_ctx->root;
    ui_ctx->active_parent  = ui_ctx->root;
}

/** V2 */

internal void
ui_begin_vertical()
{
    ui_entity_init(UI_ElementKind_Container);
}

internal void
ui_begin_horizontal()
{
    UI_Entity* entity = ui_entity_init(UI_ElementKind_Container);
    entity->direction = UI_AxisHorizontal;
}

internal void
ui_end()
{
    ui_ctx->active_parent = ui_ctx->active_element->parent;
}

internal void
ui_set_rect()
{
}

internal void
ui_set_pos(float32 x, float32 y)
{
    xassert(ui_ctx->active_element != &ui_entity_nil);

    ui_ctx->active_element->pos = vec2(x, y);
}

internal void
ui_set_wh(float32 w, float32 h)
{
    xassert(ui_ctx->active_element != &ui_entity_nil);

    ui_ctx->active_element->rect = rect_at(rect_tl(ui_ctx->active_element->rect), vec2(w, h), AlignmentTopLeft);
}

internal void
ui_set_bg_color(Color color)
{
    xassert(ui_ctx->active_element != &ui_entity_nil);

    ui_ctx->active_element->bg_color        = color;
    Color hg_color                          = color_is_dark(color) ? ColorWhite : ColorBlack;
    ui_ctx->active_element->highlight_color = color_lerp(ui_ctx->active_element->bg_color, hg_color, 0.6f);
}

internal void
ui_set_margin(float32 x, float32 y)
{
    xassert(ui_ctx->active_element != &ui_entity_nil);

    ui_ctx->active_element->margin = vec2(x, y);
}

internal void
ui_set_padding(float32 x, float32 y)
{
    xassert(ui_ctx->active_element != &ui_entity_nil);

    ui_ctx->active_element->padding = vec2(x, y);
}

/** V2 Widgets */
internal UI_Signal
ui_button(String label)
{
    UI_Entity* entity = ui_entity_init(UI_ElementKind_Clickable);
    entity->text      = label;

    UI_Signal result = {0};

    return result;
}

internal UI_Signal
ui_label(String label)
{
    UI_Entity* entity = ui_entity_init(0);
    entity->text      = label;

    UI_Signal result = {0};

    return result;
}
