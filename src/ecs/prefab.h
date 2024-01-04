#pragma once

#include <core/defines.h>

#include "component.h"
#include "world.h"

typedef struct
{
    Entity entity;
} Prefab;

internal Prefab
prefab_create(EntityManager* entity_manager, ComponentTypeField types);

internal Entity
prefab_instantiate(EntityManager* entity_manager, Prefab prefab);
