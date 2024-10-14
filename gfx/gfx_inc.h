#pragma once
#include "gfx_core.h"
#include "gfx_math.h"
#include "gfx_utils.h"

#if OS_WINDOWS
#include "opengl/gfx_core_opengl.h"
#include "opengl/gfx_primitives_opengl.h"
#else
#error no OS layer setup
#endif
