#include "prefab.h"

internal Prefab
prefab_create(EntityManager* entity_manager, ComponentTypeField types)
{
    component_type_field_set(&types, CT_Prefab);
    Entity entity = entity_create(entity_manager, types);

    Prefab result;
    result.entity = entity;
    return result;
}

internal Entity
prefab_instantiate(EntityManager* entity_manager, Prefab prefab)
{
    ComponentTypeField types = entity_get_types(entity_manager, prefab.entity);
    component_type_field_unset(&types, CT_Prefab);
    Entity new_entity = entity_create(entity_manager, types);
    entity_copy_data(entity_manager, prefab.entity, new_entity);
    return new_entity;
}

internal void
prefab_add_child(EntityManager* entity_manager, Prefab parent, Prefab child)
{
    PrefabNode* node = arena_push_struct(entity_manager->persistent_arena, PrefabNode);
    node->value      = child;

    if (!parent.first_child)
        parent.first_child = node;

    if (parent.last_child)
        parent.last_child->next = node;

    parent.last_child = node;
}