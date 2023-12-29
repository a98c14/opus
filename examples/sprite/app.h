#pragma once
#include <gfx/base.h>
#include <engine/draw.h>
#include <engine/window.h>
#include <engine/profiler.h>
#include <ui/base.h>

#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440
#define WORLD_WIDTH 640.0f
#define WORLD_HEIGHT 480.0f
#define PPU (1.0f / (WINDOW_WIDTH / WORLD_WITH)) // pixel per unit

typedef struct
{
    Arena* persistent_arena;
    Arena* frame_arena;

    Window* window;
    Renderer* renderer;
    DrawContext* draw_context;
    EngineTime time;
    Theme* theme;
    Rect screen;
    UIContext* ctx;
    InputMouse mouse;

    Profiler* main_frame;
    Profiler* update;
    Profiler* render;
    
    float32 cache_rate;
    float32 cache_clock;
} Engine;

internal Engine*
engine_init();

internal void
engine_frame_start(Engine* engine);

internal void
engine_render(Engine* engine);