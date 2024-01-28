#pragma once

#include "base.h"
#include "stb_image.h"
#include <base/file.h>

internal TextureIndex texture_new_from_file(Renderer* renderer, String texture_path, bool32 pixel_perfect, bool32 flip_vertical);
internal TextureIndex texture_array_from_file(Arena* arena, Renderer* renderer, StringList texture_paths, bool32 pixel_perfect, bool32 flip_vertical);