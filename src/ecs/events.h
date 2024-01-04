#pragma once

#include <core/defines.h>

#include "component.h"

typedef uint16 EntityEventType;
#define ENTITY_EVENT_BUFFER_CAPACITY 256

enum
{
    ET_Undefined = 0,
    ET_RESERVE_COUNT
};

typedef struct
{
    uint32              count;
    ComponentTypeField* component_types;
    void*               data;
} EntityEventBuffer;

typedef struct
{
    Arena*             arena;
    uint32             event_type_count;
    usize*             event_data_sizes;
    EntityEventBuffer* event_buffers;
} EventManager;

typedef struct
{
    uint32 event_count;
    void*  event_data;
} EventQueryResult;

internal EventManager* event_manager_new(Arena* arena, uint32 event_type_count);
internal void          event_manager_initialize_event_type(EventManager* event_manager, EntityEventType type, usize data_size);

internal void               event_manager_fire(EventManager* event_manager, EntityEventType type, ComponentTypeField types, void* data);
internal void               event_manager_clear(EventManager* event_manager);
internal EntityEventBuffer* event_manager_get_event_buffer(Arena* arena, EventManager* event_manager, EntityEventType event_type);