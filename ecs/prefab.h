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
} PrefabIndexList;

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

internal EntityBuffer prefab_instantiate_internal(Arena* arena, PrefabNode* p, ComponentTypeField types, uint32 count);
internal EntityBuffer prefab_instantiate_many(Arena* arena, PrefabIndex prefab, uint32 count);
internal EntityBuffer prefab_instantiate_with(Arena* arena, PrefabIndex prefab, ComponentTypeField with, uint32 count);
internal EntityBuffer prefab_instantiate_without(Arena* arena, PrefabIndex prefab, ComponentTypeField without, uint32 count);
internal Entity       prefab_instantiate(PrefabIndex prefab);
internal void         prefab_add_child(PrefabIndex parent, PrefabIndex child);
internal void         prefab_copy_data(PrefabIndex src, PrefabIndex dst);
internal PrefabIndex  prefab_duplicate(PrefabIndex prefab);
