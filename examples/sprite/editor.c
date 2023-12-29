#include "editor.h"

// TODO: Should we group all these into a single `Engine` struct?
internal void
editor_draw_stats(Engine* engine)
{
    int32 row_count = 6;
    int32 row_height = 10;
    int32 col_width = 50;

    UIContext* ctx = engine->ctx;
    Theme* default_theme = engine->theme;
    Arena* frame_arena = engine->frame_arena;
    /* timing info */
    Rect timing_info_container = ui_container(ctx, rect_anchor(rect(4, 4, 4 * 40, row_height * row_count),  engine->screen, ANCHOR_BL_BL), default_theme->container_default);
    LayoutGrid layout = layout_grid(timing_info_container, 4, row_count, vec2(4, 4));
    ui_label(ctx, layout_grid_cell(layout, 1, 0), string("Avg"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(layout, 2, 0), string("Min"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(layout, 3, 0), string("Max"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(layout, 0, 1), string("Time:"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(layout, 2, 1), string("-"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(layout, 3, 1), string("-"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(layout, 1, 1), string_pushf(frame_arena, "%0.2fs", engine->time.current_frame / 1000), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(layout, 0, 2), string("FPS:"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(layout, 1, 2), string_pushf(frame_arena, "%d", (int32)(1000 / engine->main_frame->cached_avg)), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(layout, 2, 2), string_pushf(frame_arena, "%d", (int32)(1000 / engine->main_frame->cached_max)), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(layout, 3, 2), string_pushf(frame_arena, "%d", (int32)(1000 / engine->main_frame->cached_min)), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(layout, 0, 3), string_pushf(frame_arena, "%s:", engine->main_frame->name.value), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(layout, 1, 3), string_pushf(frame_arena, "%0.02fms", engine->main_frame->cached_avg), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(layout, 2, 3), string_pushf(frame_arena, "%0.02fms", engine->main_frame->cached_min), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(layout, 3, 3), string_pushf(frame_arena, "%0.02fms", engine->main_frame->cached_max), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(layout, 0, 4), string_pushf(frame_arena, "%s:", engine->update->name.value), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(layout, 1, 4), string_pushf(frame_arena, "%0.02fms", engine->update->cached_avg), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(layout, 2, 4), string_pushf(frame_arena, "%0.02fms", engine->update->cached_min), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(layout, 3, 4), string_pushf(frame_arena, "%0.02fms", engine->update->cached_max), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(layout, 0, 5), string_pushf(frame_arena, "%s:", engine->render->name.value), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(layout, 1, 5), string_pushf(frame_arena, "%0.02fms", engine->render->cached_avg), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(layout, 2, 5), string_pushf(frame_arena, "%0.02fms", engine->render->cached_min), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(layout, 3, 5), string_pushf(frame_arena, "%0.02fms", engine->render->cached_max), default_theme->label_default);
    
    /* input info */
    Rect input_info_container = ui_container(ctx, rect_anchor(rect(2, 0, 2 * (col_width+5), row_height * row_count), timing_info_container, ANCHOR_TR_TL), default_theme->container_default);
    LayoutGrid input_layout = layout_grid(input_info_container, 2, row_count, vec2(4, 4));
    ui_label(ctx, layout_grid_cell(input_layout, 0, 1), string("Mouse:"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(input_layout, 1, 1), string_pushf(frame_arena, "[%0.2f, %0.2f]",  engine->mouse.world.x,  engine->mouse.world.y), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(input_layout, 0, 2), string("MouseButton:"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(input_layout, 1, 2), string_pushf(frame_arena, "0x%x",  engine->mouse.button_state), default_theme->label_default);

    /* memory info */
    Rect memory_info_container = ui_container(ctx, rect_anchor(rect(2, 0, 4 * col_width, row_height * row_count), input_info_container, ANCHOR_TR_TL), default_theme->container_default);
    LayoutGrid memory_layout = layout_grid(memory_info_container, 4, row_count, vec2(4, 4));
    ui_label(ctx, layout_grid_cell(memory_layout, 0, 0), string("Memory"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(memory_layout, 1, 0), string("Used"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(memory_layout, 2, 0), string("Available"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(memory_layout, 3, 0), string("Ratio"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(memory_layout, 0, 1), string("Persistent"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(memory_layout, 1, 1), string_pushf(frame_arena, "%0.2fmb", (float32)to_mb(engine->persistent_arena->pos)), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(memory_layout, 2, 1), string_pushf(frame_arena, "%0.2fmb", (float32)to_mb(engine->persistent_arena->cap)), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(memory_layout, 3, 1), string_pushf(frame_arena, "%0.2f%%", 100 * (float32)to_mb(engine->persistent_arena->pos) / (float32)to_mb(engine->persistent_arena->cap)), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(memory_layout, 0, 2), string("Frame"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(memory_layout, 1, 2), string_pushf(frame_arena, "%0.2fmb", (float32)to_mb(frame_arena->pos)), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(memory_layout, 2, 2), string_pushf(frame_arena, "%0.2fmb", (float32)to_mb(frame_arena->cap)), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(memory_layout, 3, 2), string_pushf(frame_arena, "%0.2f%%", 100 * (float32)to_mb(frame_arena->pos) / (float32)to_mb(frame_arena->cap)), default_theme->label_default);

    /* renderer info */
    Rect renderer_info_container = ui_container(ctx, rect_anchor(rect(2, 0, 4 * col_width, row_height * row_count), memory_info_container, ANCHOR_TR_TL), default_theme->container_default);
    LayoutGrid renderer_layout = layout_grid(renderer_info_container, 4, row_count, vec2(4, 4));
    ui_label(ctx, layout_grid_cell(renderer_layout, 0, 0), string("Renderer"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(renderer_layout, 0, 1), string("Window"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(renderer_layout, 1, 1), string_pushf(frame_arena, "W: %0.0f, H: %0.0f", engine->renderer->window_width, engine->renderer->window_height), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(renderer_layout, 0, 2), string("World"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(renderer_layout, 1, 2), string_pushf(frame_arena, "W: %0.2f, H: %0.2f", engine->renderer->world_width, engine->renderer->world_height), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(renderer_layout, 0, 3), string("Draw Count"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(renderer_layout, 1, 3), string_pushf(frame_arena, "%d", engine->renderer->stat_draw_count), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(renderer_layout, 0, 4), string("Object Count"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(renderer_layout, 1, 4), string_pushf(frame_arena, "%d", engine->renderer->stat_object_count), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(renderer_layout, 0, 5), string("Buffer Count"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(renderer_layout, 1, 5), string_pushf(frame_arena, "%d / %d", engine->renderer->stat_initialized_buffer_count, MATERIAL_DRAW_BUFFER_CAPACITY), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(renderer_layout, 2, 3), string("Avg. Probe"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(renderer_layout, 3, 3), string_pushf(frame_arena, "%0.2f", engine->renderer->stat_probe_count_sum / (float32)engine->renderer->stat_probe_count), default_theme->label_default);
    ui_label(ctx, layout_grid_cell(renderer_layout, 2, 4), string("Max Probe"), default_theme->label_bold);
    ui_label(ctx, layout_grid_cell(renderer_layout, 3, 4), string_pushf(frame_arena, "%d", engine->renderer->stat_probe_count_max), default_theme->label_default);
}