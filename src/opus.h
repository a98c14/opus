#pragma once

#include <base/asserts.h>
#include <base/bitfield.h>
#include <base/datetime.h>
#include <base/defines.h>
#include <base/easing.h>
#include <base/file.h>
#include <base/hash.h>
#include <base/log.h>
#include <base/math.h>
#include <base/memory.h>
#include <base/random.h>
#include <base/strings.h>

#include <gfx/base.h>
#include <gfx/debug.h>
#include <gfx/math.h>
#include <gfx/primitives.h>
#include <gfx/sprite.h>
#include <gfx/utils.h>

#include <physics/intersection.c>

#include <engine/color.h>
#include <engine/draw.h>
#include <engine/input.h>
#include <engine/layout.h>
#include <engine/perf_timer.h>
#include <engine/profiler.h>
#include <engine/text.h>
#include <engine/time.h>
#include <engine/window.h>

#include <ecs/component.h>
#include <ecs/events.h>
#include <ecs/prefab.h>
#include <ecs/world.h>
