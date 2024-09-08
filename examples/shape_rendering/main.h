#pragma once
#include "ft2build.h"
#include FT_FREETYPE_H
#include <corecrt_math.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(lib, "user32.lib")

#include <opus.h>

#include <opus.c>
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

global read_only String AssetPath = string_comp("..\\assets");