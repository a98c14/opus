#pragma once

#include <base/defines.h>

#include "component.h"
#include "world.h"

#define PREFAB_CAPACTIY 512
typedef struct PrefabNode PrefabNode;

typedef uint32 PrefabIndex;

struct PrefabNode
{
    Entity entity;

    PrefabNode* first_child;
    PrefabNode* last_child;

    PrefabNode* next;
    PrefabNode* prev;
};

typedef struct PrefabIndexNode PrefabIndexNode;
struct PrefabIndexNode
{
    PrefabIndex value;

    PrefabIndexNode* next;
    PrefabIndexNode* prev;
};

typedef struct
{
    uint32 count;

    PrefabIndexNode* first;
    PrefabIndexNode* last;
} PrefabList;

typedef struct PrefabGroupNode PrefabGroupNode;
struct PrefabGroupNode
{
    PrefabIndex value;
    int16       min_count;
    int16       max_count;

    PrefabGroupNode* next;
};

typedef struct
{
    uint32 count;

    PrefabGroupNode* first;
    PrefabGroupNode* last;
} PrefabGroupList;

typedef struct
{
    uint32      prefab_count;
    PrefabNode* prefabs;
} PrefabManager;
global PrefabManager* g_prefab_manager;

internal void        prefab_manager_init(Arena* arena);
internal PrefabIndex prefab_create(ComponentTypeField types);
internal PrefabIndex prefab_create_as_child(PrefabIndex parent, ComponentTypeField types);
internal Entity      prefab_entity(PrefabIndex prefab);

internal Entity      prefab_instantiate_internal(PrefabNode* p, ComponentTypeField types);
internal Entity      prefab_instantiate(PrefabIndex prefab);
internal Entity      prefab_instantiate_with(PrefabIndex prefab, ComponentTypeField with);
internal Entity      prefab_instantiate_without(PrefabIndex prefab, ComponentTypeField without);
internal void        prefab_add_child(PrefabIndex parent, PrefabIndex child);
internal void        prefab_copy_data(PrefabIndex src, PrefabIndex dst);
internal PrefabIndex prefab_duplicate(PrefabIndex prefab);
