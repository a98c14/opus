// clang-format off
#include <base/bitfield.c>
#include <base/datetime.c>
#include <base/easing.c>
#include <base/file.c>
#include <base/hash.c>
#include <base/log.c>
#include <base/math.c>
#include <base/memory.c>
#include <base/print.c>
#include <base/random.c>
#include <base/sort.c>
#include <base/strings.c>
#include <base/thread_context.c>
#include <base/layout.c>

#include <gfx/base.c>
#include <gfx/batch.c>
#include <gfx/debug.c>
#include <gfx/math.c>
#include <gfx/primitives.c>
#include <gfx/sprite.c>
#include <gfx/utils.c>

#include <physics/intersection.h>

#include <draw/draw_inc.c>

#include <engine/color.c>
#include <engine/draw.c>
#include <engine/input.c>
#include <engine/perf_timer.c>
#include <engine/profiler.c>
#include <engine/text.c>
#include <engine/time.c>
#include <engine/window.c>
#include <engine/audio.c>

#include <ui.c>

#include <ecs/component.c>
#include <ecs/events.c>
#include <ecs/prefab.c>
#include <ecs/world.c>