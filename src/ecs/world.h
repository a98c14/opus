#pragma once

#include <core/defines.h>

#include "component.h"

#define DEFAULT_CHUNK_CAPACITY 2048
#define ENTITY_CAPACITY        65536

typedef int32 ArchetypeIndex;
typedef int32 ChunkIndex;

typedef struct
{
    int32 component_count;

    // stores the available component for archetype
    ComponentIndex* components;

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
    ComponentIndex type;
    void*          data;
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

internal EntityAddress entity_address_null();
internal bool32        entity_address_is_null(EntityAddress address);

internal ArchetypeIndex archetype_get_or_create(EntityManager* manager, ComponentTypeField types);

internal bool32     chunk_has_space(Chunk* chunk, uint32 count);
internal ChunkIndex chunk_get_or_create(EntityManager* manager, ComponentTypeField components, uint32 space_required, uint32 capacity);

internal Entity entity_create(EntityManager* manager, ComponentTypeField types);
internal void   entity_destroy(EntityManager* manager, Entity entity);