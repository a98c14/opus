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

    uint32      free_entity_count;
    uint32*     free_entity_indices;
    EntityNode* free_entity_nodes;
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

internal Entity        entity_get_parent(Entity entity);
internal EntityAddress entity_address_null();
internal bool32        entity_address_is_null(EntityAddress address);
internal bool32        entity_is_same(Entity a, Entity b);
internal bool32        entity_is_null(Entity a);
internal Entity        entity_null();

internal ArchetypeIndex archetype_get_or_create(ComponentTypeField types);

internal bool32     chunk_has_space(Chunk* chunk, uint32 count);
internal ChunkIndex chunk_get_or_create(ComponentTypeField components, uint32 space_required, uint32 capacity);
internal void       chunk_delete_entity_data(EntityAddress address);
internal void       chunk_copy_data(EntityAddress src, EntityAddress dst);

internal uint32      entity_reserve_free();
internal void        entity_free(Entity e);
internal EntityNode* entity_node_alloc();
internal void        entity_node_free(EntityNode* node);
internal Entity      entity_create(ComponentTypeField types);
internal void        entity_destroy(Entity entity);
internal void        entity_activate(Entity entity);
internal void        entity_deactivate(Entity entity);

internal void               entity_add_child(Entity parent, Entity child);
internal ComponentTypeField entity_get_types(Entity entity);
internal void               entity_copy_data(Entity src, Entity dst);
internal void               entity_move(Entity entity, ChunkIndex destination);

internal void   component_add(Entity entity, ComponentType type);
internal void   component_add_many(Entity entity, ComponentTypeField components);
internal void*  component_add_ref_internal(Entity entity, ComponentType type);
internal void   component_remove(Entity entity, ComponentType type);
internal void   component_remove_many(Entity entity, ComponentTypeField components);
internal bool32 component_data_exists_internal(Entity entity, ComponentType component_type);
internal void*  component_data_ref_internal(Entity entity, ComponentType component_type);
internal void   component_copy(Entity src, Entity dst, ComponentType component_type);
#define component_add_ref(entity, component_type)     ((component_type*)component_add_ref_internal(entity, CT_##component_type))
#define component_data_exists(entity, component_type) component_data_exists_internal(entity, CT_##component_type)
#define component_data_get(entity, component_type)    (*((component_type*)component_data_ref_internal(entity, CT_##component_type)))
#define component_data_ref(entity, component_type)    ((component_type*)component_data_ref_internal(entity, CT_##component_type))

internal World* world_new(Arena* arena);
internal void   entity_manager_init(Arena* persistent_arena, Arena* temp_arena, ComponentTypeManager* type_manager);

/** Entity Query */
internal EntityQuery       entity_query_default();
internal EntityQueryResult entity_get_all(Arena* arena, EntityQuery query);
internal bool32            entity_is_alive(Entity entity);
internal Entity            entity_get_parent(Entity entity);