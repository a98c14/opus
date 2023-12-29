#include "app.h"

internal void
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

internal Engine*
engine_init()
{
    logger_init();
    Arena* persistent_arena = make_arena_reserve(mb(512));
    Arena* frame_arena = make_arena_reserve(mb(256));
    Window* window = window_create(persistent_arena, WINDOW_WIDTH, WINDOW_HEIGHT, "Simple Example", key_callback);

    RendererConfiguration renderer_configuration = {
        .window_width  = WINDOW_WIDTH,
        .window_height = WINDOW_HEIGHT,
        .world_width  = 0,
        .world_height = WORLD_HEIGHT,
        .clear_color  = ColorSlate900
    };
    Renderer* renderer = renderer_new(persistent_arena, &renderer_configuration);
    DrawContext* dc = draw_context_new(persistent_arena, renderer);
    EngineTime time = engine_time_new();
    Theme* default_theme = theme_init_default(persistent_arena, renderer);
    Rect screen = rect_from_wh(renderer->world_width, renderer->world_height);
    UIContext* ctx = ui_context_new(persistent_arena, dc);

    Profiler* main_frame = profiler_new(persistent_arena, string("Total"));
    Profiler* update = profiler_new(persistent_arena, string("Update"));
    Profiler* render = profiler_new(persistent_arena, string("Render"));

    Engine* engine = arena_push_struct_zero_aligned(persistent_arena, Engine, 16);
    engine->persistent_arena = persistent_arena;
    engine->frame_arena = frame_arena;
    engine->renderer = renderer;
    engine->window = window;
    engine->render = render;
    engine->draw_context = dc;
    engine->screen = screen;
    engine->theme = default_theme;
    engine->ctx = ctx;
    engine->main_frame = main_frame;
    engine->update = update;
    engine->render = render;
    engine->time = time;
    engine->mouse = (InputMouse){0};
    engine->cache_rate = 0.2;
    engine->cache_clock = 0;
    log_info("engine initialized successfully");
    return engine;
}

internal void
engine_frame_start(Engine* engine)
{
    arena_reset(engine->frame_arena);
    profiler_end(engine->main_frame);
    profiler_begin(engine->main_frame);
    engine->time = engine_get_time(engine->time);
    engine->mouse = input_mouse_get(engine->window, engine->renderer->camera, engine->mouse);
    engine->ctx->mouse = engine->mouse;
    engine->ctx->time = engine->time;
    profiler_begin(engine->update);

    // draw grid
    int32 cell_size = 50;
    int32 grid_cols = engine->renderer->world_width / cell_size;
    grid_cols += grid_cols % 2 == 0 ? 0 : 1;
    int32 grid_rows = engine->renderer->world_height / cell_size;
    grid_rows += grid_rows % 2 == 0 ? 0 : 1;

    float32 left  = cell_size * (grid_cols / 2.0f) * -1;
    float32 right  = cell_size * (grid_cols / 2.0f);
    float32 bottom = cell_size * (grid_rows / 2.0f) * -1;
    float32 top    = cell_size * (grid_rows / 2.0f);

    for(int32 y = 0; y < grid_rows+1; y++)
    {
        draw_line(engine->draw_context, vec2(left, bottom + cell_size * y), vec2(right, bottom + cell_size * y), ColorSlate700, 1.1);
    }

    for(int32 x = 0; x < grid_cols+1; x++)
    {
        draw_line(engine->draw_context, vec2(left + x * cell_size, top), vec2(left + x * cell_size, bottom), ColorSlate700, 1.1);
    }
}

internal void
engine_render(Engine* engine)
{
    profiler_end(engine->update);
    profiler_begin(engine->render);
    renderer_render(engine->renderer, engine->time.dt);
    window_update(engine->window);
    profiler_end(engine->render);
    if(engine->cache_clock > engine->cache_rate)
    {
        profiler_refresh_cache(engine->main_frame);
        profiler_refresh_cache(engine->update);
        profiler_refresh_cache(engine->render);
        engine->cache_clock = 0;
    }
    engine->cache_clock += engine->time.dt;
}