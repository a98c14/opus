#pragma once

#include "base/defines.h"
#include "base/log.h"
#include "base/memory.h"

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
    int32  version;
} Entity;

typedef struct EntityNode EntityNode;

struct EntityNode
{
    Entity value;

    EntityNode* prev;
    EntityNode* next;
};

typedef struct EntityList EntityList;

struct EntityList
{
    uint16 count;
    uint16 capacity;

    EntityNode* first;
    EntityNode* last;
};

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

    // relations
    Entity*     entity_parents;
    EntityList* entity_children;
} World;

typedef struct
{
    Arena* persistent_arena;
    Arena* temp_arena;

    ComponentTypeManager* type_manager;
    World*                world;
} EntityManager;
global EntityManager* g_entity_manager;

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

typedef struct
{
    Entity value;
} Parent;

internal Entity        entity_get_parent(EntityManager* entity_manager, Entity entity);
internal EntityAddress entity_address_null();
internal bool32        entity_address_is_null(EntityAddress address);
internal bool32        entity_is_same(Entity a, Entity b);
internal bool32        entity_is_null(Entity a);
internal Entity        entity_null();

internal ArchetypeIndex archetype_get_or_create(EntityManager* manager, ComponentTypeField types);

internal bool32     chunk_has_space(Chunk* chunk, uint32 count);
internal ChunkIndex chunk_get_or_create(EntityManager* manager, ComponentTypeField components, uint32 space_required, uint32 capacity);
internal void       chunk_delete_entity_data(EntityManager* manager, EntityAddress address);
internal void       chunk_copy_data(EntityManager* manager, EntityAddress src, EntityAddress dst);

internal Entity entity_create(EntityManager* manager, ComponentTypeField types);
internal void   entity_destroy(EntityManager* manager, Entity entity);
internal void   entity_activate(EntityManager* manager, Entity entity);
internal void   entity_deactivate(EntityManager* manager, Entity entity);

internal void               entity_add_child(EntityManager* manager, Entity parent, Entity child);
internal ComponentTypeField entity_get_types(EntityManager* manager, Entity entity);
internal void               entity_copy_data(EntityManager* manager, Entity src, Entity dst);
internal void               entity_move(EntityManager* manager, Entity entity, ChunkIndex destination);

internal void   component_add(EntityManager* manager, Entity entity, ComponentType type);
internal void   component_add_many(EntityManager* manager, Entity entity, ComponentTypeField components);
internal void*  component_add_ref_internal(EntityManager* manager, Entity entity, ComponentType type);
internal void   component_remove(EntityManager* manager, Entity entity, ComponentType type);
internal void   component_remove_many(EntityManager* manager, Entity entity, ComponentTypeField components);
internal bool32 component_data_exists_internal(EntityManager* entity_manager, Entity entity, ComponentType component_type);
internal void*  component_data_ref_internal(EntityManager* entity_manager, Entity entity, ComponentType component_type);
internal void   component_copy(EntityManager* entity_manager, Entity src, Entity dst, ComponentType component_type);
#define component_add_ref(entity_manager, entity, component_type)     ((component_type*)component_add_ref_internal(entity_manager, entity, CT_##component_type))
#define component_data_exists(entity_manager, entity, component_type) component_data_exists_internal(entity_manager, entity, CT_##component_type)
#define component_data_get(entity_manager, entity, component_type)    (*((component_type*)component_data_ref_internal(entity_manager, entity, CT_##component_type)))
#define component_data_ref(entity_manager, entity, component_type)    ((component_type*)component_data_ref_internal(entity_manager, entity, CT_##component_type))

internal World*         world_new(Arena* arena);
internal EntityManager* entity_manager_new(Arena* persistent_arena, Arena* temp_arena, ComponentTypeManager* type_manager);
internal void           entity_manager_global_init(Arena* persistent_arena, Arena* temp_arena, ComponentTypeManager* type_manager);

/** Entity Query */
internal EntityQuery       entity_query_default();
internal EntityQueryResult entity_get_all(Arena* arena, EntityManager* entity_manager, EntityQuery query);
internal bool32            entity_is_alive(EntityManager* entity_manager, Entity entity);
internal Entity            entity_get_parent(EntityManager* entity_manager, Entity entity);