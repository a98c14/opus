#include "world.h"
#include <ecs/world.h>

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

internal ArchetypeIndex
archetype_get_or_create(EntityManager* manager, ComponentTypeField components)
{
    World* world = manager->world;
    for (int i = 0; i < world->archetype_count; i++)
    {
        if (component_type_field_is_same(world->archetype_components[i], components))
            return i;
    }

    ArchetypeIndex archtype_index = world->archetype_count;
    Archetype*     archetype      = &world->archetypes[archtype_index];

    archetype->component_count            = component_type_field_count(components);
    archetype->component_buffer_index_map = arena_push_array_zero(manager->persistent_arena, int32, COMPONENT_COUNT);
    archetype->components                 = arena_push_array_zero(manager->persistent_arena, ComponentIndex, archetype->component_count);

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
            archetype->byte_per_entity += manager->type_manager->component_sizes[i];
            current_index++;
        }
    }

    return archtype_index;
}

internal bool32
chunk_has_space(Chunk* chunk, uint32 count)
{
    return chunk->entity_capacity - chunk->entity_count >= count;
}

internal ChunkIndex
chunk_get_or_create(EntityManager* manager, ComponentTypeField components, uint32 space_required, uint32 capacity)
{
    World* world = manager->world;
    for (int i = 0; i < world->chunk_count; i++)
    {
        if (component_type_field_is_same(world->chunk_components[i], components) && chunk_has_space(&world->chunks[i], space_required))
            return i;
    }

    ArchetypeIndex archetype_index = archetype_get_or_create(manager, components);
    Archetype*     archetype       = &world->archetypes[archetype_index];

    ChunkIndex chunk_index = world->chunk_count;
    Chunk*     chunk       = &world->chunks[chunk_index];
    chunk->archetype_index = archetype_index;
    chunk->entity_count    = 0;
    chunk->entity_capacity = capacity;
    chunk->entities        = arena_push_array_zero(manager->persistent_arena, Entity, capacity);
    chunk->data_buffers    = arena_push_array_zero(manager->persistent_arena, DataBuffer, archetype->component_count);

    for (int i = 0; i < archetype->component_count; i++)
    {
        ComponentIndex component_index        = archetype->components[i];
        usize          component_size         = manager->type_manager->component_sizes[component_index];
        int32          component_buffer_index = archetype->component_buffer_index_map[component_index];

        chunk->data_buffers[component_buffer_index].type = component_index;
        chunk->data_buffers[component_buffer_index].data = arena_push_zero(manager->persistent_arena, component_size * capacity);
    }

    world->chunk_components[chunk_index] = components;
    world->chunk_count++;

    return chunk_index;
}

internal Entity
entity_create(EntityManager* manager, ComponentTypeField components)
{
    World*     world       = manager->world;
    ChunkIndex chunk_index = chunk_get_or_create(manager, components, 1, DEFAULT_CHUNK_CAPACITY);
    Chunk*     chunk       = &world->chunks[chunk_index];

    uint32 entity_index                                        = world->entity_count;
    int32  entity_chunk_index                                  = chunk->entity_count;
    world->entity_addresses[entity_index].chunk_index          = chunk_index;
    world->entity_addresses[entity_index].chunk_internal_index = entity_chunk_index;

    Entity* entity = &world->entities[entity_index];
    entity->version += 1;
    chunk->entities[entity_chunk_index] = *entity;

    chunk->entity_count++;
    world->entity_count++;
    manager->world->chunk_count;

    return *entity;
}

internal void
entity_destroy(EntityManager* manager, Entity entity)
{
    World* world = manager->world;

    EntityAddress current_address = world->entity_addresses[entity.index];

    // if the entity is already deleted exit early
    if (entity_address_is_null(current_address))
        return;

    world->entity_addresses[entity.index] = entity_address_null();
    Chunk*     current_chunk              = &world->chunks[current_address.chunk_index];
    Archetype* archetype                  = &world->archetypes[current_chunk->archetype_index];

    // if the entity is the last entity on the chunk we don't need to move any data
    uint32 last_entity_internal_index = current_chunk->entity_count - 1;
    if (current_address.chunk_internal_index == last_entity_internal_index)
    {
        current_chunk->entity_count--;
        return;
    }

    // if the entity is NOT the last entity on the chunk swap the entity wit the last entity
    // and decrement entity count by 1;
    Entity         last_entity         = current_chunk->entities[last_entity_internal_index];
    EntityAddress* last_entity_address = &world->entity_addresses[last_entity.index];

    for (int i = 0; i < archetype->component_count; i++)
    {
        DataBuffer* data_buffer    = &current_chunk->data_buffers[i];
        usize       component_size = manager->type_manager->component_sizes[data_buffer->type];
        memcpy(((uint8*)data_buffer->data) + (component_size * current_address.chunk_internal_index), ((uint8*)data_buffer->data) + (component_size * last_entity_address->chunk_internal_index), component_size);
    }

    current_chunk->entities[current_address.chunk_internal_index] = last_entity;
    last_entity_address->chunk_index                              = current_address.chunk_index;
    last_entity_address->chunk_internal_index                     = current_address.chunk_internal_index;
    current_chunk->entity_count--;
}
