#include "ui_components.h"

internal void
ui_label_animated(UI_Key key, String text, D_AnimationIndex animation, bool32 loop)
{
    xassert(ui_state->sprite_atlas, "SpriteAtlas needs to be loaded before calling `ui_widget_animated_label`");
    D_Animation animation_data = ui_state->sprite_atlas->animations[animation];

    Rect   animation_rect         = rect_shrink(ui_animation_rect(animation_data.sprite_start_index), vec2(8, 4));
    uint32 start_animation_length = animation_length(animation_data);

    ui_create_with_key(key, CutSideTop, animation_rect.h)
    {
        UI_SpriteAnimator* animator = ui_animator_get(key);
        if (animator->started_at <= 0)
        {
            animator->started_at = ui_state->update_t;
        }

        uint32 current_frame = (uint32)((ui_state->update_t - animator->started_at) / UI_ANIMATION_UPDATE_RATE);
        current_frame        = loop ? current_frame % start_animation_length : min(current_frame, start_animation_length - 1);

        Rect rect            = ui_cut_left(animation_rect.w);
        animator->last_rect  = rect;
        animator->updated_at = ui_state->update_t;

        d_sprite(ui_state->sprite_atlas, animation_data.sprite_start_index + current_frame, rect, vec2_one(), ANCHOR_C_C, ColorInvisible);
        d_string(ui_rect(), text, 8, ColorWhite, ANCHOR_L_L);
    }
}
