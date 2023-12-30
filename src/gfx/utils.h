#pragma once

#include <core/file.h>
#include "stb_image.h"
#include "base.h"

internal TextureIndex texture_new_from_file(Renderer* renderer, String texture_path, bool32 pixel_perfect, bool32 flip_vertical);