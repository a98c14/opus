#pragma once
#include "gfx_core.c"
#include "gfx_math.c"
#include "gfx_utils.c"

#if OS_WINDOWS
#include "opengl/gfx_core_opengl.c"
#include "opengl/gfx_primitives_opengl.c"
#else
#error no GFX layer setup
#endif
