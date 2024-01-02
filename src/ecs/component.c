#include "component.h"

internal ComponentTypeManager*
component_type_manager_new(Arena* arena)
{
    ComponentTypeManager* result = arena_push_struct_zero(arena, ComponentTypeManager);
    return result;
}

internal ComponentTypeRegistrationRequest*
component_type_register_begin(Arena* temp_arena)
{
    ComponentTypeRegistrationRequest* request = arena_push_struct_zero(temp_arena, ComponentTypeRegistrationRequest);
    request->component_type_capacity          = COMPONENT_TYPE_CAPACITY;
    request->component_sizes                  = arena_push_array_zero(temp_arena, usize, COMPONENT_TYPE_CAPACITY);
    return request;
}

internal void
component_type_register_add(ComponentTypeRegistrationRequest* request, ComponentIndex type_index, usize component_size, ComponentType component_type)
{
    xassert((component_type == ComponentTypeTag && component_size == 0) || (component_type == ComponentTypeData && component_size > 0), "component size should be larger than 0 or DataComponents and should be 0 for TagComponents");
    request->component_sizes[request->component_type_count] = component_size;
    request->component_types[request->component_type_count] = component_type;
    request->component_type_count++;
}

internal void
component_type_register_complete(ComponentTypeManager* manager, ComponentTypeRegistrationRequest* request)
{
    manager->component_type_count = request->component_type_count;
    manager->component_sizes      = arena_push_array_zero(manager->arena, usize, manager->component_type_count);

    memcpy(manager->component_sizes, request->component_sizes, request->component_type_count);
    memcpy(manager->component_types, request->component_types, request->component_type_count);
    log_info("component registration completed. %d components registered", request->component_type_count);
}

internal ComponentTypeField*
component_type_field_new(Arena* arena, ComponentTypeManager* manager)
{
    ComponentTypeField* result = arena_push_struct(arena, ComponentTypeField);
    result->value              = arena_push_array_zero(arena, uint32, COMPONENT_TYPE_FIELD_LENGTH);
    return result;
}

internal uint32
component_type_field_count(ComponentTypeField a)
{
    uint32 type_count = 0;
    for (int i = 0; i < COMPONENT_TYPE_FIELD_LENGTH; i++)
        type_count += __popcnt(a.value[i]);
    return type_count;
}

internal bool32
component_type_field_is_same(ComponentTypeField a, ComponentTypeField b)
{
    bool32 is_same = true;
    for (int i = 0; i < COMPONENT_TYPE_FIELD_LENGTH; i++)
    {
        if (a.value[i] - b.value[i] != 0)
        {
            is_same = false;
            break;
        }
    }
    return is_same;
}

internal ComponentTypeField
component_type_field_add_internal(ComponentTypeField field, ComponentIndex type_index)
{
    ComponentTypeField result = field;
    result.value[type_index / COMPONENT_TYPE_FIELD_SIZE] |= 1 << (type_index % COMPONENT_TYPE_FIELD_SIZE);
    return result;
}

internal ComponentTypeField
component_type_field_remove_internal(ComponentTypeField field, ComponentIndex type_index)
{
    ComponentTypeField result = field;
    result.value[type_index / COMPONENT_TYPE_FIELD_SIZE] &= ~(1 << (type_index % COMPONENT_TYPE_FIELD_SIZE));
    return result;
}