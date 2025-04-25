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
    ui_line_height           = em(1);
}

internal void
ui_state_load_atlas(D_SpriteAtlas* atlas)
{
    ui_ctx->sprite_atlas = atlas;
}

internal void
ui_update(float32 dt)
{
    ui_ctx->frame++;
    ui_ctx->update_t += dt;
    arena_reset(ui_ctx->frame_arena);
    ui_create_fixed(screen_rect());

    // for (uint32 i = 0; i < UI_MAX_ANIMATION_COUNT; i++)
    // {
    //     UI_SpriteAnimator* animator = &ui_ctx->active_animations[i];

    //     /** if widget hasn't updated the animation last frame, take control and play exit animation */
    //     if (animator->owner.value > 0 && animator->exit_animation > 0 && animator->updated_at < ui_ctx->update_t)
    //     {
    //         D_Animation animation             = ui_ctx->sprite_atlas->animations[animator->exit_animation];
    //         uint32      exit_animation_length = animation_length(animation);

    //         uint32 current_frame = (uint32)((ui_ctx->update_t - animator->started_at) / UI_ANIMATION_UPDATE_RATE);
    //         if (current_frame == exit_animation_length)
    //             memory_zero_struct(animator);

    //         // draw_sprite_rect(animator->last_rect, animation.sprite_start_index + current_frame, ANCHOR_C_C);
    //     }
    //     else if (animator->owner.value > 0 && animator->updated_at < ui_ctx->update_t)
    //     {
    //         memory_zero_struct(animator);
    //     }
    // }
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
    xassert(ui_ctx->layout_stack, "there is no active layout!");
    return ui_ctx->layout_stack->v.r;
}

internal Rect*
ui_rect_ref(void)
{
    xassert(ui_ctx->layout_stack, "there is no active layout!");
    return &ui_ctx->layout_stack->v.r;
}
internal void
ui_rect_set(Rect r)
{
    xassert(ui_ctx->layout_stack, "there is no active layout!");
    ui_ctx->layout_stack->v.r = r;
}

internal Rect
ui_cut_dynamic(CutSide cut_side, float32 size)
{
    xassert(ui_ctx->layout_stack, "there is no active layout!");
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
    xassert(ui_ctx->layout_stack, "there is no active layout!");
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

/** animation */
internal UI_SpriteAnimator*
ui_animator_find(UI_Key key)
{
    for (uint32 i = 0; i < UI_MAX_ANIMATION_COUNT; i++)
    {
        if (ui_ctx->active_animations[i].owner.value == key.value)
            return &ui_ctx->active_animations[i];
    }

    return 0;
}

internal UI_SpriteAnimator*
ui_animator_reserve(UI_Key key)
{
    for (uint32 i = 0; i < UI_MAX_ANIMATION_COUNT; i++)
    {
        if (ui_ctx->active_animations[i].owner.value == ui_key_null.value)
        {
            UI_SpriteAnimator* animator = &ui_ctx->active_animations[i];
            animator->owner             = key;
            return animator;
        }
    }

    return 0;
}

internal UI_SpriteAnimator*
ui_animator_get(UI_Key key)
{
    UI_SpriteAnimator* animator = ui_animator_find(key);
    if (!animator)
    {
        animator = ui_animator_reserve(key);
        xassert(animator, "Could not reserve animator. Exceeded maximum active animation limit for UI");
    }

    return animator;
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
        ui_ctx->hot = key;
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
        *value = remap_f32(rect_left(inner_bar), rect_right(inner_bar), min_value, max_value, mouse.screen.x);
        *value = clamp(min_value, *value, max_value);
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