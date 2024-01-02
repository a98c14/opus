#pragma once

#include <core/defines.h>

#include "component.h"
#include "world.h"

typedef struct
{
    Entity entity;
} Prefab;

internal Prefab
prefab_create(ComponentBitField types);
