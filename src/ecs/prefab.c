#include "prefab.h"

internal void
prefab_manager_init(Arena* arena)
{
    g_prefab_manager          = arena_push_struct_zero(arena, PrefabManager);
    g_prefab_manager->prefabs = arena_push_array_zero(arena, PrefabNode, PREFAB_CAPACTIY);
}

internal PrefabIndex
prefab_create(ComponentTypeField types)
{
    component_type_field_set(&types, CTT_PrefabComponent);
    Entity entity = entity_create(types);

    PrefabIndex index = g_prefab_manager->prefab_count;
    g_prefab_manager->prefab_count++;
    g_prefab_manager->prefabs[index].entity = entity;

    return index;
}

internal PrefabIndex
prefab_create_as_child(PrefabIndex parent, ComponentTypeField types)
{
    PrefabIndex result = prefab_create(types);
    prefab_add_child(parent, result);
    return result;
}

internal Entity
prefab_entity(PrefabIndex prefab)
{
    return g_prefab_manager->prefabs[prefab].entity;
}

internal Entity
prefab_instantiate_internal(PrefabNode* p, ComponentTypeField types)
{
    Entity entity = entity_create(types);
    entity_copy_data(p->entity, entity);

    PrefabNode* child = p->first_child;
    while (child)
    {
        ComponentTypeField types = entity_get_types(child->entity);
        component_type_field_unset(&types, CTT_PrefabComponent);
        component_type_field_set(&types, CTT_ParentComponent);

        Entity child_entity = entity_create(types);
        entity_copy_data(child->entity, child_entity);
        entity_add_child(entity, child_entity);

        child = child->next;
    }
    return entity;
}

internal Entity
prefab_instantiate(PrefabIndex prefab)
{
    return prefab_instantiate_with(prefab, (ComponentTypeField){0});
}

internal Entity
prefab_instantiate_with(PrefabIndex prefab, ComponentTypeField with)
{
    PrefabNode* p = &g_prefab_manager->prefabs[prefab];

    ComponentTypeField types = entity_get_types(p->entity);
    component_type_field_unset(&types, CTT_PrefabComponent);
    component_type_field_set_group(&types, with);

    return prefab_instantiate_internal(p, types);
}

internal Entity
prefab_instantiate_without(PrefabIndex prefab, ComponentTypeField without)
{
    PrefabNode* p = &g_prefab_manager->prefabs[prefab];

    ComponentTypeField types = entity_get_types(p->entity);
    component_type_field_unset(&types, CTT_PrefabComponent);
    component_type_field_unset_group(&types, without);

    return prefab_instantiate_internal(p, types);
}

internal void
prefab_add_child(PrefabIndex parent, PrefabIndex child)
{
    PrefabNode* node = arena_push_struct(g_entity_manager->persistent_arena, PrefabNode);
    node->entity     = g_prefab_manager->prefabs[child].entity;

    PrefabNode* parent_node = &g_prefab_manager->prefabs[parent];
    queue_push(parent_node->first_child, parent_node->last_child, node);
}

internal void
prefab_copy_data(PrefabIndex src, PrefabIndex dst)
{
    PrefabNode* src_prefab = &g_prefab_manager->prefabs[src];
    PrefabNode* dst_prefab = &g_prefab_manager->prefabs[dst];
    xassert(!entity_is_same(src_prefab->entity, dst_prefab->entity), "cannot copy prefab to itself");
    entity_copy_data(src_prefab->entity, dst_prefab->entity);
}

internal PrefabIndex
prefab_duplicate(PrefabIndex prefab)
{
    PrefabNode*        p     = &g_prefab_manager->prefabs[prefab];
    ComponentTypeField types = entity_get_types(p->entity);
    return prefab_create(types);
}