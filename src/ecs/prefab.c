#include "prefab.h"
#include <ecs/component.h>

internal Prefab
prefab_create(EntityManager* entity_manager, ComponentTypeField types)
{
    component_type_field_set(&types, CTT_PrefabComponent);
    Entity entity = entity_create(entity_manager, types);

    Prefab result = {0};
    result.entity = entity;
    return result;
}

internal Prefab
prefab_create_as_child(EntityManager* entity_manager, Prefab* parent, ComponentTypeField types)
{
    Prefab result = prefab_create(entity_manager, types);
    prefab_add_child(entity_manager, parent, result);
    return result;
}

internal Entity
prefab_instantiate(EntityManager* entity_manager, Prefab prefab)
{
    return prefab_instantiate_modified(entity_manager, prefab, (ComponentTypeField){0});
}

internal Entity
prefab_instantiate_modified(EntityManager* entity_manager, Prefab prefab, ComponentTypeField additional_types)
{
    ComponentTypeField types = entity_get_types(entity_manager, prefab.entity);
    component_type_field_unset(&types, CTT_PrefabComponent);
    component_type_field_set_group(&types, additional_types);
    Entity entity = entity_create(entity_manager, types);
    entity_copy_data(entity_manager, prefab.entity, entity);

    PrefabNode* child = prefab.first_child;
    while (child)
    {
        ComponentTypeField types = entity_get_types(entity_manager, child->value.entity);
        component_type_field_unset(&types, CTT_PrefabComponent);
        component_type_field_set(&types, CTT_ParentComponent);

        Entity child_entity = entity_create(entity_manager, types);
        entity_copy_data(entity_manager, child->value.entity, child_entity);
        entity_add_child(entity_manager, entity, child_entity);

        child = child->next;
    }
    return entity;
}

internal void
prefab_add_child(EntityManager* entity_manager, Prefab* parent, Prefab child)
{
    PrefabNode* node = arena_push_struct(entity_manager->persistent_arena, PrefabNode);
    node->value      = child;

    if (!parent->first_child)
        parent->first_child = node;

    if (parent->last_child)
        parent->last_child->next = node;

    parent->last_child = node;
}

internal void
prefab_copy_data(EntityManager* entity_manager, Prefab src, Prefab dst)
{
    xassert(!entity_is_same(src.entity, dst.entity), "cannot copy prefab to itself");
    entity_copy_data(entity_manager, src.entity, dst.entity);
}