#include "ui_components.h"

internal void
ui_widget_animated_label(UI_Key key, String text, AnimationIndex animation, bool32 loop)
{
    xassert(ui_state->sprite_atlas, "SpriteAtlas needs to be loaded before calling `ui_widget_animated_label`");
    Animation animation_data = ui_state->sprite_atlas->animations[animation];

    Rect   animation_rect         = rect_shrink(ui_animation_rect(animation_data.sprite_start_index), vec2(8, 4));
    uint16 start_animation_length = animation_length(animation_data);

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

        draw_sprite_rect(rect, animation_data.sprite_start_index + current_frame, ANCHOR_C_C);
        draw_text(text, ui_rect(), ANCHOR_L_L, 8, ColorWhite);
    }
}
