#include "world.h"
#include <ecs/component.h>
#include <ecs/world.h>

internal Entity
entity_null()
{
    Entity result;
    result.index   = 0;
    result.version = -1;
    return result;
}

internal EntityAddress
entity_address_null()
{
    EntityAddress result;
    result.chunk_index          = -1;
    result.chunk_internal_index = -1;
    return result;
}

internal bool32
entity_address_is_null(EntityAddress address)
{
    return address.chunk_index == -1 || address.chunk_internal_index == -1;
}

internal bool32
entity_is_same(Entity a, Entity b)
{
    return a.index == b.index && a.version == b.version;
}

internal bool32
entity_is_null(Entity a)
{
    return a.index <= 0 && a.version <= 0;
}

internal ArchetypeIndex
archetype_get_or_create(ComponentTypeField components)
{
    World* world = g_entity_manager->world;
    for (int i = 0; i < world->archetype_count; i++)
    {
        if (component_type_field_is_same(world->archetype_components[i], components))
            return i;
    }

    ArchetypeIndex archtype_index = world->archetype_count;
    Archetype*     archetype      = &world->archetypes[archtype_index];

    archetype->component_count            = component_type_field_count(components);
    archetype->component_buffer_index_map = arena_push_array_zero(g_entity_manager->persistent_arena, int32, COMPONENT_COUNT);
    archetype->components                 = arena_push_array_zero(g_entity_manager->persistent_arena, ComponentType, archetype->component_count);

    int32 current_index        = 0;
    archetype->byte_per_entity = 0;
    for (int i = 0; i < COMPONENT_COUNT; i++)
    {
        int32 type_location       = i / COMPONENT_TYPE_FIELD_SIZE;
        int32 type_location_index = i % COMPONENT_TYPE_FIELD_SIZE;

        archetype->component_buffer_index_map[i] = -1;
        if (components.value[type_location] & (1 << type_location_index))
        {
            archetype->component_buffer_index_map[i] = current_index;
            archetype->components[current_index]     = (ComponentType)i;
            archetype->byte_per_entity += g_entity_manager->type_manager->component_sizes[i];
            current_index++;
        }
    }

    world->archetype_components[archtype_index] = components;
    world->archetype_count++;
    return archtype_index;
}

internal bool32
chunk_has_space(Chunk* chunk, uint32 count)
{
    return chunk->entity_capacity - chunk->entity_count >= count;
}

internal ChunkIndex
chunk_get_or_create(ComponentTypeField components, uint32 space_required, uint32 capacity)
{
    World* world = g_entity_manager->world;
    for (int i = 0; i < world->chunk_count; i++)
    {
        if (component_type_field_is_same(world->chunk_components[i], components) && chunk_has_space(&world->chunks[i], space_required))
            return i;
    }

    ArchetypeIndex archetype_index = archetype_get_or_create(components);
    Archetype*     archetype       = &world->archetypes[archetype_index];

    ChunkIndex chunk_index = world->chunk_count;
    Chunk*     chunk       = &world->chunks[chunk_index];
    chunk->archetype_index = archetype_index;
    chunk->entity_count    = 0;
    chunk->entity_capacity = capacity;
    chunk->entities        = arena_push_array_zero(g_entity_manager->persistent_arena, Entity, capacity);
    chunk->data_buffers    = arena_push_array_zero(g_entity_manager->persistent_arena, DataBuffer, archetype->component_count);

    for (int i = 0; i < archetype->component_count; i++)
    {
        ComponentType component_index = archetype->components[i];

        ComponentDataType data_type = g_entity_manager->type_manager->component_data_types[component_index];
        if (data_type != ComponentDataTypeDefault)
            continue;

        usize component_size         = g_entity_manager->type_manager->component_sizes[component_index];
        int32 component_buffer_index = archetype->component_buffer_index_map[component_index];

        chunk->data_buffers[component_buffer_index].type = component_index;
        chunk->data_buffers[component_buffer_index].data = arena_push_zero(g_entity_manager->persistent_arena, component_size * capacity);
    }

    world->chunk_components[chunk_index] = components;
    world->chunk_count++;
    return chunk_index;
}

/** Deletes the entity data at the given internal index. Does not handle parent/child relationships or entity addresses.  */
internal void
chunk_delete_entity_data(EntityAddress address)
{
    World*     world     = g_entity_manager->world;
    Chunk*     chunk     = &world->chunks[address.chunk_index];
    Archetype* archetype = &world->archetypes[chunk->archetype_index];

    // if the entity is not the last entity swap it with the last entity
    uint32 last_entity_internal_index = chunk->entity_count - 1;
    if (address.chunk_internal_index != last_entity_internal_index)
    {
        Entity         last_entity         = chunk->entities[last_entity_internal_index];
        EntityAddress* last_entity_address = &world->entity_addresses[last_entity.index];

        for (int i = 0; i < archetype->component_count; i++)
        {
            DataBuffer* data_buffer    = &chunk->data_buffers[i];
            usize       component_size = g_entity_manager->type_manager->component_sizes[data_buffer->type];
            memcpy(((uint8*)data_buffer->data) + (component_size * address.chunk_internal_index), ((uint8*)data_buffer->data) + (component_size * last_entity_address->chunk_internal_index), component_size);
        }

        chunk->entities[address.chunk_internal_index] = last_entity;
        last_entity_address->chunk_index              = address.chunk_index;
        last_entity_address->chunk_internal_index     = address.chunk_internal_index;
    }

    chunk->entity_count--;
}

internal void
chunk_copy_data(EntityAddress src, EntityAddress dst)
{
    World*                world        = g_entity_manager->world;
    ComponentTypeManager* type_manager = g_entity_manager->type_manager;

    Chunk* src_chunk = &world->chunks[src.chunk_index];
    Chunk* dst_chunk = &world->chunks[dst.chunk_index];

    Archetype* src_archetype = &world->archetypes[src_chunk->archetype_index];
    Archetype* dst_archetype = &world->archetypes[dst_chunk->archetype_index];

    for (int i = 0; i < dst_archetype->component_count; i++)
    {
        ComponentType dst_type = dst_archetype->components[i];

        // TODO(selim): figure out what to do about chunk components (we don't have those yet)
        if (type_manager->component_data_types[dst_type] != ComponentDataTypeDefault)
            continue;

        int32 dst_buffer_index = dst_archetype->component_buffer_index_map[dst_type];
        int32 src_buffer_index = src_archetype->component_buffer_index_map[dst_type];

        // if type doesn't exist in the source entity, skip the component
        if (src_buffer_index == -1)
            continue;

        usize       component_size = type_manager->component_sizes[dst_type];
        uint32      src_index      = src.chunk_internal_index;
        uint32      dst_index      = dst.chunk_internal_index;
        DataBuffer* src_buffer     = &src_chunk->data_buffers[src_buffer_index];
        DataBuffer* dst_buffer     = &dst_chunk->data_buffers[dst_buffer_index];

        memcpy((uint8*)dst_buffer->data + dst_index * component_size, (uint8*)src_buffer->data + src_index * component_size, component_size);
    }
}

internal uint32
entity_reserve_free()
{
    World* world = g_entity_manager->world;
    uint32 result;

    if (world->free_entity_count > 0)
    {
        result = world->free_entity_indices[world->free_entity_count - 1];
        world->free_entity_count--;
    }
    else
    {
        result = world->entity_count;
        world->entity_count++;
    }

    return result;
}

internal void
entity_free(Entity e)
{
    World* world = g_entity_manager->world;

    world->free_entity_indices[world->free_entity_count] = e.index;
    world->free_entity_count++;
}

internal EntityNode*
entity_node_alloc()
{
    World* world = g_entity_manager->world;

    if (world->free_entity_nodes)
    {
        EntityNode* result = world->free_entity_nodes;
        stack_pop(world->free_entity_nodes);
        return result;
    }

    EntityNode* result = arena_push_struct_zero(g_entity_manager->persistent_arena, EntityNode);
    return result;
}

internal void
entity_node_free(EntityNode* node)
{
    World* world = g_entity_manager->world;
    stack_push(world->free_entity_nodes, node);
}

internal Entity
entity_create(ComponentTypeField components)
{
    World*     world       = g_entity_manager->world;
    ChunkIndex chunk_index = chunk_get_or_create(components, 1, DEFAULT_CHUNK_CAPACITY);
    Chunk*     chunk       = &world->chunks[chunk_index];

    uint32 entity_index                                        = entity_reserve_free();
    int32  entity_chunk_index                                  = chunk->entity_count;
    world->entity_addresses[entity_index].chunk_index          = chunk_index;
    world->entity_addresses[entity_index].chunk_internal_index = entity_chunk_index;

    Entity* entity = &world->entities[entity_index];
    entity->version += 1;
    chunk->entities[entity_chunk_index] = *entity;

    chunk->entity_count++;
    world->entity_count++;
    g_entity_manager->world->chunk_count;

    return *entity;
}

internal void
entity_destroy(Entity entity)
{
    World*        world           = g_entity_manager->world;
    EntityAddress current_address = world->entity_addresses[entity.index];

    // if the entity is already deleted exit early
    if (entity_address_is_null(current_address))
        return;

    chunk_delete_entity_data(current_address);
    world->entity_addresses[entity.index] = entity_address_null();
    world->entity_parents[entity.index]   = entity_null();
    entity_free(entity);

    EntityList* children = &world->entity_children[entity.index];
    if (children->count > 0)
    {
        EntityNode* child = children->first;
        while (child)
        {
            EntityNode* next_child = child->next;
            entity_destroy(child->value);
            entity_node_free(child);
            child = next_child;
        }

        children->first = 0;
        children->last  = 0;
        children->count = 0;
    }
}

internal void
entity_activate(Entity entity)
{
    World* world = g_entity_manager->world;
    if (!component_data_exists_internal(entity, CTT_InactiveComponent))
        return;

    component_remove(entity, CTT_InactiveComponent);
    EntityList* children = &world->entity_children[entity.index];
    if (children->count > 0)
    {
        EntityNode* child = children->first;
        while (child && child->value.version > 0)
        {
            component_remove(child->value, CTT_InactiveComponent);
            child = child->next;
        }
    }
}

internal void
entity_deactivate(Entity entity)
{
    World* world = g_entity_manager->world;
    component_add(entity, CTT_InactiveComponent);
    EntityList* children = &world->entity_children[entity.index];
    if (children->count > 0)
    {
        EntityNode* child = children->first;
        while (child && child->value.version > 0)
        {
            component_add(child->value, CTT_InactiveComponent);
            child = child->next;
        }
    }
}

internal void
component_add_many(Entity entity, ComponentTypeField components)
{
    World*        world           = g_entity_manager->world;
    EntityAddress current_address = world->entity_addresses[entity.index];

    xassert(!entity_address_is_null(current_address), "entity address is not valid");

    ComponentTypeField current_components = world->chunk_components[current_address.chunk_index];
    ComponentTypeField new_components     = component_type_field_union(current_components, components);
    if (component_type_field_is_same(current_components, new_components))
    {
        log_warn("trying to add a component that is already added! entity id %d", entity.index);
        return;
    }

    ChunkIndex new_chunk_index = chunk_get_or_create(new_components, 1, DEFAULT_CHUNK_CAPACITY);
    entity_move(entity, new_chunk_index);
}

internal void
component_add(Entity entity, ComponentType type)
{
    ComponentTypeField field = {0};
    component_type_field_set(&field, type);
    component_add_many(entity, field);
}

internal void*
component_add_ref_internal(Entity entity, ComponentType type)
{
    ComponentTypeField field = {0};
    component_type_field_set(&field, type);
    component_add_many(entity, field);
    return component_data_ref_internal(entity, type);
}

internal void
component_remove_many(Entity entity, ComponentTypeField components)
{
    World*        world           = g_entity_manager->world;
    EntityAddress current_address = world->entity_addresses[entity.index];

    xassert(!entity_address_is_null(current_address), "entity address is not valid");

    ComponentTypeField current_components = world->chunk_components[current_address.chunk_index];
    ComponentTypeField new_components     = component_type_field_not(current_components, components);
    if (component_type_field_is_same(current_components, new_components))
    {
        log_warn("trying to remove a component that is already removed! entity id %d", entity.index);
        return;
    }

    ChunkIndex new_chunk_index = chunk_get_or_create(new_components, 1, DEFAULT_CHUNK_CAPACITY);
    entity_move(entity, new_chunk_index);
}

internal void
component_remove(Entity entity, ComponentType type)
{
    ComponentTypeField field = {0};
    component_type_field_set(&field, type);
    component_remove_many(entity, field);
}

internal void
entity_add_child(Entity parent, Entity child)
{
    World* world = g_entity_manager->world;

    world->entity_parents[child.index] = parent;

    EntityList* children = &world->entity_children[parent.index];

    EntityNode* node = entity_node_alloc();
    node->value      = child;

    dll_push_back(children->first, children->last, node);
    children->count++;
}

internal ComponentTypeField
entity_get_types(Entity entity)
{
    World* world = g_entity_manager->world;

    EntityAddress current_address = world->entity_addresses[entity.index];
    xassert(!entity_address_is_null(current_address), "given entity does not exist");
    return world->chunk_components[current_address.chunk_index];
}

/** Copies all matching components from source entity to destination entity */
internal void
entity_copy_data(Entity src, Entity dst)
{
    World* world = g_entity_manager->world;

    EntityAddress src_address = world->entity_addresses[src.index];
    EntityAddress dst_address = world->entity_addresses[dst.index];
    xassert(!entity_address_is_null(src_address) && !entity_address_is_null(dst_address), "invalid entity during copy");
    chunk_copy_data(src_address, dst_address);
}

internal void
entity_move(Entity entity, ChunkIndex destination)
{
    World* world = g_entity_manager->world;

    EntityAddress src_address = world->entity_addresses[entity.index];
    xassert(!entity_address_is_null(src_address), "invalid entity during copy");

    Chunk*        dst_chunk          = &world->chunks[destination];
    EntityAddress dst_address        = {0};
    dst_address.chunk_index          = destination;
    dst_address.chunk_internal_index = dst_chunk->entity_count;

    dst_chunk->entities[dst_address.chunk_internal_index] = entity;
    dst_chunk->entity_count++;
    world->entity_addresses[entity.index] = dst_address;

    chunk_copy_data(src_address, dst_address);
    chunk_delete_entity_data(src_address);
}

internal World*
world_new(Arena* arena)
{
    World* world                = arena_push_struct_zero(arena, World);
    world->archetype_components = arena_push_array_zero(arena, ComponentTypeField, ARCHETYPE_CAPACITY);
    world->archetypes           = arena_push_array_zero(arena, Archetype, ARCHETYPE_CAPACITY);

    world->chunk_components = arena_push_array_zero(arena, ComponentTypeField, CHUNK_CAPACITY);
    world->chunks           = arena_push_array_zero(arena, Chunk, CHUNK_CAPACITY);

    world->entity_addresses = arena_push_array_zero(arena, EntityAddress, ENTITY_CAPACITY);
    world->entities         = arena_push_array_zero(arena, Entity, ENTITY_CAPACITY);

    world->entity_parents      = arena_push_array_zero(arena, Entity, ENTITY_CAPACITY);
    world->entity_children     = arena_push_array_zero(arena, EntityList, ENTITY_CAPACITY);
    world->free_entity_indices = arena_push_array_zero(arena, uint32, ENTITY_CAPACITY);

    for (int i = 0; i < ENTITY_CAPACITY; i++)
    {
        world->entity_addresses[i] = entity_address_null();
        world->entity_parents[i]   = entity_null();
        world->entities[i].index   = i;
    }
    return world;
}

internal void
entity_manager_init(Arena* persistent_arena, Arena* temp_arena, ComponentTypeManager* type_manager)
{
    g_entity_manager                   = arena_push_struct_zero(persistent_arena, EntityManager);
    g_entity_manager->persistent_arena = persistent_arena;
    g_entity_manager->temp_arena       = temp_arena;
    g_entity_manager->world            = world_new(persistent_arena);
    g_entity_manager->type_manager     = type_manager;
}

internal bool32
component_data_exists_internal(Entity entity, ComponentType component_type)
{
    World* world = g_entity_manager->world;
    xassert(world->entities[entity.index].version == entity.version, "invalid entity");
    EntityAddress address = world->entity_addresses[entity.index];
    return component_type_field_is_set(&world->chunk_components[address.chunk_index], component_type);
}

internal void*
component_data_ref_internal(Entity entity, ComponentType component_type)
{
    World*        world   = g_entity_manager->world;
    EntityAddress address = world->entity_addresses[entity.index];
    xassert(!entity_address_is_null(address), "given entity is not valid");
    Chunk*     chunk     = &world->chunks[address.chunk_index];
    Archetype* archetype = &world->archetypes[chunk->archetype_index];
    xassert(chunk->entities[address.chunk_internal_index].version == entity.version, "given entity is not the same as the one in chunk");

    int32 component_index = archetype->component_buffer_index_map[component_type];
    xassert(component_index >= 0, "component doesn't exist on the entity");
    usize component_size = g_entity_manager->type_manager->component_sizes[component_type];
    void* component_data = (void*)((uint8*)chunk->data_buffers[component_index].data + component_size * address.chunk_internal_index);
    return component_data;
}

internal void
component_copy(Entity src, Entity dst, ComponentType component_type)
{
    usize component_size = g_entity_manager->type_manager->component_sizes[component_type];
    void* src_ref        = component_data_ref_internal(src, component_type);
    void* dst_ref        = component_data_ref_internal(dst, component_type);
    memcpy(dst_ref, src_ref, component_size);
}

/** Entity Query */
internal EntityQuery
entity_query_default()
{
    EntityQuery result = {0};
    component_type_field_set(&result.none, CTT_PrefabComponent);
    component_type_field_set(&result.none, CTT_InactiveComponent);
    return result;
}

internal EntityQueryResult
entity_get_all(Arena* arena, EntityQuery query)
{
    World* world = g_entity_manager->world;

    // TODO(selim): cache query results (invalidate on chunk add/remove)
    ArenaTemp temp         = arena_begin_temp(g_entity_manager->temp_arena);
    Chunk*    chunks       = arena_push_array(temp.arena, Chunk, 128);
    bool32    chunk_count  = 0;
    uint32    entity_count = 0;

    for (int i = 0; i < world->chunk_count; i++)
    {
        bool32 all     = component_type_field_contains(world->chunk_components[i], query.all);
        bool32 any     = component_type_field_none(world->chunk_components[i], query.any);
        bool32 exclude = component_type_field_none(world->chunk_components[i], query.none);

        if (all && any && exclude)
        {
            chunks[chunk_count] = world->chunks[i];
            chunk_count++;
            entity_count += world->chunks[i].entity_count;
        }
    }

    EntityQueryResult result = {0};
    result.count             = 0;
    result.entities          = arena_push_array(arena, Entity, entity_count);
    for (int i = 0; i < chunk_count; i++)
    {
        memcpy(result.entities + result.count, chunks[i].entities, sizeof(Entity) * chunks[i].entity_count);
        result.count += chunks[i].entity_count;
    }
    arena_end_temp(temp);
    return result;
}

internal bool32
entity_is_alive(Entity entity)
{
    return !entity_is_null(entity) && g_entity_manager->world->entities[entity.index].version == entity.version && !entity_address_is_null(g_entity_manager->world->entity_addresses[entity.index]);
}

internal Entity
entity_get_parent(Entity entity)
{
    return g_entity_manager->world->entity_parents[entity.index];
}

internal EntityList
entity_get_children(Entity entity)
{
    return g_entity_manager->world->entity_children[entity.index];
}
