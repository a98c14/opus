#pragma once

#include <core/defines.h>

#include "component.h"
#include "world.h"

typedef struct PrefabNode PrefabNode;

typedef struct
{
    Entity entity;

    // TODO(selim): Prefab should not contain any pointers! Use prefab index instead?
    PrefabNode* first_child;
    PrefabNode* last_child;
} Prefab;

struct PrefabNode
{
    Prefab value;

    PrefabNode* next;
};

internal Prefab
prefab_create(EntityManager* entity_manager, ComponentTypeField types);

internal Entity
prefab_instantiate(EntityManager* entity_manager, Prefab prefab);

internal void
prefab_add_child(EntityManager* entity_manager, Prefab* parent, Prefab child);

internal void
prefab_copy_data(EntityManager* entity_manager, Prefab src, Prefab dst);
