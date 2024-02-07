#include "prefab.h"
#include <ecs/component.h>
#include <ecs/world.h>

internal Prefab
prefab_create(ComponentTypeField types)
{
    component_type_field_set(&types, CTT_PrefabComponent);
    Entity entity = entity_create(types);

    Prefab result = {0};
    result.entity = entity;
    return result;
}

internal Prefab
prefab_create_as_child(Prefab* parent, ComponentTypeField types)
{
    Prefab result = prefab_create(types);
    prefab_add_child(parent, result);
    return result;
}

internal Entity
prefab_instantiate(Prefab prefab)
{
    return prefab_instantiate_with(prefab, (ComponentTypeField){0});
}

internal Entity
prefab_instantiate_with(Prefab prefab, ComponentTypeField with)
{
    ComponentTypeField types = entity_get_types(prefab.entity);
    component_type_field_unset(&types, CTT_PrefabComponent);
    component_type_field_set_group(&types, with);
    Entity entity = entity_create(types);
    entity_copy_data(prefab.entity, entity);

    PrefabNode* child = prefab.first_child;
    while (child)
    {
        ComponentTypeField types = entity_get_types(child->value.entity);
        component_type_field_unset(&types, CTT_PrefabComponent);
        component_type_field_set(&types, CTT_ParentComponent);

        Entity child_entity = entity_create(types);
        entity_copy_data(child->value.entity, child_entity);
        entity_add_child(entity, child_entity);

        child = child->next;
    }
    return entity;
}

internal Entity
prefab_instantiate_without(Prefab prefab, ComponentTypeField without)
{
    ComponentTypeField types = entity_get_types(prefab.entity);
    component_type_field_unset(&types, CTT_PrefabComponent);
    component_type_field_unset_group(&types, without);
    Entity entity = entity_create(types);
    entity_copy_data(prefab.entity, entity);

    PrefabNode* child = prefab.first_child;
    while (child)
    {
        ComponentTypeField types = entity_get_types(child->value.entity);
        component_type_field_unset(&types, CTT_PrefabComponent);
        component_type_field_set(&types, CTT_ParentComponent);

        Entity child_entity = entity_create(types);
        entity_copy_data(child->value.entity, child_entity);
        entity_add_child(entity, child_entity);

        child = child->next;
    }
    return entity;
}

internal void
prefab_add_child(Prefab* parent, Prefab child)
{
    PrefabNode* node = arena_push_struct(g_entity_manager->persistent_arena, PrefabNode);
    node->value      = child;

    if (!parent->first_child)
        parent->first_child = node;

    if (parent->last_child)
        parent->last_child->next = node;

    parent->last_child = node;
}

internal void
prefab_copy_data(Prefab src, Prefab dst)
{
    xassert(!entity_is_same(src.entity, dst.entity), "cannot copy prefab to itself");
    entity_copy_data(src.entity, dst.entity);
}