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
ui_state_init(Arena* arena)
{
    ui_state                   = arena_push_struct_zero(arena, UI_State);
    ui_state->persistent_arena = arena;
    ui_state->frame_arena      = arena_new_reserve(mb(32));
}

internal void
ui_state_load_atlas(SpriteAtlas* atlas)
{
    ui_state->sprite_atlas = atlas;
}

internal void
ui_state_update(EngineTime time)
{
    EngineTime last_update_time = ui_state->time;
    ui_state->frame++;
    ui_state->time = time;
    arena_reset(ui_state->frame_arena);

    for (uint32 i = 0; i < UI_MAX_ANIMATION_COUNT; i++)
    {
        UI_SpriteAnimator* animator = &ui_state->active_animations[i];

        /** if widget hasn't updated the animation last frame, take control and play exit animation */
        if (animator->owner.value > 0 && animator->exit_animation > 0 && animator->updated_at < last_update_time.current_frame_time)
        {
            Animation animation             = ui_state->sprite_atlas->animations[animator->exit_animation];
            uint16    exit_animation_length = animation_length(animation);

            uint32 current_frame = (ui_state->time.current_frame_time - animator->started_at) / UI_ANIMATION_UPDATE_RATE;
            if (current_frame == exit_animation_length)
                memory_zero_struct(animator);

            // draw_sprite_rect(animator->last_rect, animation.sprite_start_index + current_frame, ANCHOR_C_C);
        }
        else if (animator->owner.value > 0 && animator->updated_at < last_update_time.current_frame_time)
        {
            memory_zero_struct(animator);
        }
    }
}

internal void
ui_set_key(UI_Key key)
{
    ui_state->layout_stack->v.key = key;
}

internal Rect
ui_rect(void)
{
    xassert(ui_state->layout_stack, "there is no active layout!");
    return ui_state->layout_stack->v.r;
}

internal Rect*
ui_rect_ref(void)
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
ui_push_rect(UI_Key key, Rect r)
{
    UI_LayoutNode* node = arena_push_struct_zero(ui_state->frame_arena, UI_LayoutNode);
    node->v.r           = r;
    node->v.key         = key;
    stack_push(ui_state->layout_stack, node);
}

internal void
ui_push_cut(UI_Key key, CutSide cut_side, float32 size)
{
    UI_LayoutNode* node = arena_push_struct_zero(ui_state->frame_arena, UI_LayoutNode);
    node->v.r           = ui_cut_dynamic(cut_side, size);
    node->v.key         = key;
    stack_push(ui_state->layout_stack, node);
}

internal void
ui_pop_layout(void)
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
ui_debug(void)
{
    // draw_debug_rect(ui_rect());
}

internal Rect
ui_sprite_rect(SpriteIndex sprite)
{
    const Sprite* s = &ui_state->sprite_atlas->sprites[sprite];
    // NOTE(selim): -2 is removed because by default all our sprites have 1 px padding on each side
    return rect_from_wh(s->size.w - 2, s->size.h - 2);
}

internal Rect
ui_animation_rect(AnimationIndex animation)
{
    const Animation* a        = &ui_state->sprite_atlas->animations[animation];
    Rect             max_rect = {0};
    for (uint64 i = a->sprite_start_index; i < a->sprite_end_index; i++)
    {
        const Sprite* s = &ui_state->sprite_atlas->sprites[i];
        max_rect.w      = max(max_rect.w, s->size.w - 2);
        max_rect.h      = max(max_rect.h, s->size.h - 2);
    }
    return max_rect;
}

/** animation */
internal UI_SpriteAnimator*
ui_animator_find(UI_Key key)
{
    for (uint32 i = 0; i < UI_MAX_ANIMATION_COUNT; i++)
    {
        if (ui_state->active_animations[i].owner.value == key.value)
            return &ui_state->active_animations[i];
    }

    return 0;
}

internal UI_SpriteAnimator*
ui_animator_reserve(UI_Key key)
{
    for (uint32 i = 0; i < UI_MAX_ANIMATION_COUNT; i++)
    {
        if (ui_state->active_animations[i].owner.value == ui_key_null.value)
        {
            UI_SpriteAnimator* animator = &ui_state->active_animations[i];
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