#pragma once

#include <base/defines.h>

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

internal Prefab prefab_create(ComponentTypeField types);
internal Prefab prefab_create_as_child(Prefab* parent, ComponentTypeField types);
internal Entity prefab_instantiate(Prefab prefab);

// Instantiates the prefab with extra types added beforehand
internal Entity prefab_instantiate_with(Prefab prefab, ComponentTypeField with);
internal Entity prefab_instantiate_without(Prefab prefab, ComponentTypeField without);
internal void   prefab_add_child(Prefab* parent, Prefab child);
internal void   prefab_copy_data(Prefab src, Prefab dst);
