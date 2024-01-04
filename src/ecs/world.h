#pragma once

#include <core/defines.h>
#include <ecs/component.h>

#include "component.h"

#define DEFAULT_CHUNK_CAPACITY 2048
#define ENTITY_CAPACITY        65536
#define CHUNK_CAPACITY         512
#define ARCHETYPE_CAPACITY     256

typedef int32 ArchetypeIndex;
typedef int32 ChunkIndex;

typedef struct
{
    int32 component_count;

    // stores the available component for archetype
    ComponentType* components;

    // stores the buffer indices of components for archetype
    int32* component_buffer_index_map;

    // stores how many bytes is required per entity (doesn't include chunk
    // components)
    uint32 byte_per_entity;
} Archetype;

typedef struct
{
    uint32 index;
    uint32 version;
} Entity;

typedef struct
{
    ComponentType type;
    void*         data;
} DataBuffer;

typedef struct
{
    ArchetypeIndex archetype_index;
    uint32         entity_capacity;
    uint32         entity_count;
    Entity*        entities;
    DataBuffer*    data_buffers;
} Chunk;

typedef struct
{
    ChunkIndex chunk_index;
    int32      chunk_internal_index;
} EntityAddress;

typedef struct
{
    uint32              archetype_count;
    ComponentTypeField* archetype_components;
    Archetype*          archetypes;

    uint32              chunk_count;
    ComponentTypeField* chunk_components;
    Chunk*              chunks;

    uint32         entity_count;
    Entity*        entities;
    EntityAddress* entity_addresses;
} World;

typedef struct
{
    Arena* persistent_arena;
    Arena* temp_arena;

    ComponentTypeManager* type_manager;
    World*                world;
} EntityManager;

typedef struct
{
    ComponentTypeField all;
    ComponentTypeField any;
    ComponentTypeField none;
} EntityQuery;

typedef struct
{
    Entity* entities;
    uint32  count;
} EntityQueryResult;

internal EntityAddress entity_address_null();
internal bool32        entity_address_is_null(EntityAddress address);

internal ArchetypeIndex archetype_get_or_create(EntityManager* manager, ComponentTypeField types);

internal bool32     chunk_has_space(Chunk* chunk, uint32 count);
internal ChunkIndex chunk_get_or_create(EntityManager* manager, ComponentTypeField components, uint32 space_required, uint32 capacity);

internal Entity             entity_create(EntityManager* manager, ComponentTypeField types);
internal void               entity_destroy(EntityManager* manager, Entity entity);
internal void               entity_add_component(EntityManager* manager, Entity entity, ComponentType component);
internal void               entity_remove_component(EntityManager* manager, Entity entity, ComponentType component);
internal ComponentTypeField entity_get_types(EntityManager* manager, Entity entity);
internal void               entity_copy_data(EntityManager* manager, Entity src, Entity dst);

internal void* component_data_ref_internal(EntityManager* entity_manager, Entity entity, ComponentType component_type);
#define component_data_ref(entity_manager, entity, component_type) (component_type*)component_data_ref_internal(entity_manager, entity, CT_##component_type)
#define component_data_get(entity_manager, entity, component_type) *((component_type*)component_data_ref_internal(entity_manager, entity, CT_##component_type))

internal World*         world_new(Arena* arena);
internal EntityManager* entity_manager_new(Arena* persistent_arena, Arena* temp_arena, ComponentTypeManager* type_manager);

/** Entity Query */
internal EntityQuery       entity_query_default();
internal EntityQueryResult entity_get_all(Arena* arena, EntityManager* entity_manager, EntityQuery query);
internal bool32            entity_is_alive(EntityManager* entity_manager, Entity entity);