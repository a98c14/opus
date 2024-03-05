#include "prefab.h"
#include <base/thread_context.h>
#include <ecs/world.h>

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
    Entity entity = entity_create_from_type(types);

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

internal EntityBuffer
prefab_instantiate_internal(Arena* arena, PrefabNode* p, ComponentTypeField types, uint32 count)
{
    EntityBuffer result = entity_create_many(arena, types, count);
    for (uint32 i = 0; i < result.count; i++)
    {
        entity_copy_data(p->entity, result.entities[i]);

        PrefabNode* child = p->first_child;
        while (child)
        {
            ComponentTypeField types = entity_get_types(child->entity);
            component_type_field_unset(&types, CTT_PrefabComponent);
            component_type_field_set(&types, CTT_ParentComponent);

            Entity child_entity = entity_create_from_type(types);
            entity_copy_data(child->entity, child_entity);
            entity_add_child(result.entities[i], child_entity);

            child = child->next;
        }
    }
    return result;
}

internal EntityBuffer
prefab_instantiate_many(Arena* arena, PrefabIndex prefab, uint32 count)
{
    return prefab_instantiate_with(arena, prefab, (ComponentTypeField){0}, count);
}

internal EntityBuffer
prefab_instantiate_with(Arena* arena, PrefabIndex prefab, ComponentTypeField with, uint32 count)
{
    PrefabNode* p = &g_prefab_manager->prefabs[prefab];

    ComponentTypeField types = entity_get_types(p->entity);
    component_type_field_unset(&types, CTT_PrefabComponent);
    component_type_field_set_group(&types, with);

    return prefab_instantiate_internal(arena, p, types, count);
}

internal EntityBuffer
prefab_instantiate_without(Arena* arena, PrefabIndex prefab, ComponentTypeField without, uint32 count)
{
    PrefabNode* p = &g_prefab_manager->prefabs[prefab];

    ComponentTypeField types = entity_get_types(p->entity);
    component_type_field_unset(&types, CTT_PrefabComponent);
    component_type_field_unset_group(&types, without);

    return prefab_instantiate_internal(arena, p, types, count);
}

internal Entity
prefab_instantiate(PrefabIndex prefab)
{
    ArenaTemp    temp   = scratch_begin(0, 0);
    EntityBuffer buffer = prefab_instantiate_with(temp.arena, prefab, (ComponentTypeField){0}, 1);
    Entity       result = buffer.entities[0];
    scratch_end(temp);
    return result;
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