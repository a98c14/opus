#pragma once

#include <core/defines.h>

#include "component_type.h"
#include "entity.h"
#include "world.h"

typedef struct
{
    Entity entiy;
} Prefab;

internal Prefab
prefab_create(ComponentTypeField types);
