#include "events.h"

internal EventManager*
event_manager_new(Arena* arena, uint32 event_type_count)
{
    EventManager* event_manager     = arena_push_struct_zero(arena, EventManager);
    event_manager->arena            = arena;
    event_manager->event_buffers    = arena_push_array_zero(arena, EntityEventBuffer, event_type_count);
    event_manager->event_data_sizes = arena_push_array_zero(arena, usize, event_type_count);
    event_manager->event_type_count = event_type_count;
    return event_manager;
}

internal void
event_manager_global_init(Arena* arena, uint32 event_type_count)
{
    g_event_manager = event_manager_new(arena, event_type_count);
}

internal void
event_manager_initialize_event_type(EventManager* event_manager, EntityEventType type, usize data_size)
{
    xassert(type < event_manager->event_type_count, "invalid event type");
    event_manager->event_data_sizes[type]              = data_size;
    event_manager->event_buffers[type].component_types = arena_push_array_zero(event_manager->arena, ComponentTypeField, ENTITY_EVENT_BUFFER_CAPACITY);
    event_manager->event_buffers[type].data            = arena_push_zero(event_manager->arena, data_size * ENTITY_EVENT_BUFFER_CAPACITY);
}

internal void
event_manager_fire(EventManager* event_manager, EntityEventType type, ComponentTypeField types, void* data)
{
    xassert(event_manager->event_buffers[type].data, "event type hasn't been initialized");

    EntityEventBuffer* event_buffer    = &event_manager->event_buffers[type];
    usize              event_data_size = event_manager->event_data_sizes[type];

    event_buffer->component_types[event_buffer->count] = types;
    memcpy((uint8*)event_buffer->data + event_buffer->count * event_data_size, data, event_data_size);
    event_buffer->count++;
}

internal void
event_manager_clear(EventManager* event_manager)
{
    for (int i = ET_Undefined; i < event_manager->event_type_count; i++)
        event_manager->event_buffers[i].count = 0;
}

internal EntityEventBuffer*
event_manager_get_event_buffer(Arena* arena, EventManager* event_manager, EntityEventType event_type)
{
    return &event_manager->event_buffers[event_type];
}