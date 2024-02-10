#include "ui_components.h"

internal void
ui_widget_animation(UI_Key key, AnimationIndex start_animation_index, bool32 loop)
{
    xassert(ui_state->sprite_atlas, "SpriteAtlas needs to be loaded before calling `ui_widget_animation`");

    Animation animation      = ui_state->sprite_atlas->animations[start_animation_index];
    Rect      animation_rect = ui_animation_rect(animation.sprite_start_index);

    uint16 start_animation_length = animation_length(animation);
    ui_create_with_key(key, CutSideTop, animation_rect.h)
    {
        UI_SpriteAnimator* animator = ui_animator_get(key);
        if (animator->started_at <= 0)
        {
            animator->started_at = ui_state->time.current_frame_time;
        }

        uint16 current_frame = (ui_state->time.current_frame_time - animator->started_at) / UI_ANIMATION_UPDATE_RATE;
        current_frame        = loop ? current_frame % start_animation_length : min(current_frame, start_animation_length - 1);

        Rect rect            = ui_cut_left(animation_rect.w);
        animator->last_rect  = rect;
        animator->updated_at = ui_state->time.current_frame_time;

        draw_sprite_rect(rect, animation.sprite_start_index + current_frame, ANCHOR_C_C);
        draw_text(string("Texter"), ui_rect(), ANCHOR_L_L, 8, ColorWhite);
    }
}
