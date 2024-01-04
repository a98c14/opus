#include "component.h"

internal ComponentTypeManager*
component_type_manager_new(Arena* arena)
{
    ComponentTypeManager* result = arena_push_struct_zero(arena, ComponentTypeManager);
    result->arena                = arena;
    return result;
}

internal ComponentTypeRegistrationRequest*
component_type_register_begin(Arena* temp_arena)
{
    ComponentTypeRegistrationRequest* request = arena_push_struct_zero(temp_arena, ComponentTypeRegistrationRequest);
    request->component_type_capacity          = COMPONENT_CAPACITY;
    request->component_sizes                  = arena_push_array_zero(temp_arena, usize, COMPONENT_CAPACITY);
    request->component_data_types             = arena_push_array_zero(temp_arena, ComponentDataType, COMPONENT_CAPACITY);
    request->component_indices                = arena_push_array_zero(temp_arena, ComponentType, COMPONENT_CAPACITY);
    return request;
}

internal void
component_type_register_add(ComponentTypeRegistrationRequest* request, ComponentType type_index, usize component_size, ComponentDataType component_type)
{
    xassert((component_type == ComponentDataTypeTag && component_size == 0) || (component_type == ComponentDataTypeDefault && component_size > 0), "component size should be larger than 0 or DefaultComponents and should be 0 for TagComponents");
    request->component_sizes[request->component_type_count]      = component_size;
    request->component_data_types[request->component_type_count] = component_type;
    request->component_indices[request->component_type_count]    = type_index;
    request->component_type_count++;
}

internal void
component_type_register_complete(ComponentTypeManager* manager, ComponentTypeRegistrationRequest* request)
{
    manager->component_type_count = request->component_type_count;
    manager->component_sizes      = arena_push_array_zero(manager->arena, usize, COMPONENT_COUNT);
    manager->component_data_types = arena_push_array_zero(manager->arena, ComponentDataType, COMPONENT_COUNT);
    for (int i = 0; i < request->component_type_count; i++)
    {
        ComponentType type                  = request->component_indices[i];
        manager->component_sizes[type]      = request->component_sizes[i];
        manager->component_data_types[type] = request->component_data_types[i];
    }
    log_info("component registration completed. %d components registered", request->component_type_count);
}

internal uint32
component_type_field_count(ComponentTypeField a)
{
    uint32 type_count = 0;
    for (int i = 0; i < COMPONENT_TYPE_FIELD_LENGTH; i++)
        type_count += __popcnt(a.value[i]);
    return type_count;
}

internal ComponentTypeField
component_type_field_or(ComponentTypeField a, ComponentTypeField b)
{
    ComponentTypeField result = {0};
    for (int i = 0; i < COMPONENT_TYPE_FIELD_LENGTH; i++)
        result.value[i] = a.value[i] | b.value[i];
    return result;
}

internal ComponentTypeField
component_type_field_add(ComponentTypeField field, ComponentType type_index)
{
    ComponentTypeField result = field;
    result.value[type_index / COMPONENT_TYPE_FIELD_SIZE] |= 1 << (type_index % COMPONENT_TYPE_FIELD_SIZE);
    return result;
}

internal ComponentTypeField
component_type_field_remove(ComponentTypeField field, ComponentType type_index)
{
    ComponentTypeField result = field;
    result.value[type_index / COMPONENT_TYPE_FIELD_SIZE] &= ~(1 << (type_index % COMPONENT_TYPE_FIELD_SIZE));
    return result;
}

internal void
component_type_field_set(ComponentTypeField* field, ComponentType type_index)
{
    field->value[type_index / COMPONENT_TYPE_FIELD_SIZE] |= 1 << (type_index % COMPONENT_TYPE_FIELD_SIZE);
}

internal void
component_type_field_unset(ComponentTypeField* field, ComponentType type_index)
{
    field->value[type_index / COMPONENT_TYPE_FIELD_SIZE] &= ~(1 << (type_index % COMPONENT_TYPE_FIELD_SIZE));
}

internal void
component_type_field_set_group(ComponentTypeField* field, ComponentTypeField b)
{
    for (int i = 0; i < COMPONENT_TYPE_FIELD_LENGTH; i++)
        field->value[i] = field->value[i] | b.value[i];
}

/** A is same as B */
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

/** A contains all components from B */
internal bool32
component_type_field_contains(ComponentTypeField a, ComponentTypeField b)
{
    bool32 result = true;
    for (int i = 0; i < COMPONENT_TYPE_FIELD_LENGTH; i++)
        result = result && ((a.value[i] | b.value[i]) <= a.value[i]);
    return result;
}

/** A contains at least one component from B */
internal bool32
component_type_field_any(ComponentTypeField a, ComponentTypeField b)
{
    bool32 result = false;
    for (int i = 0; i < COMPONENT_TYPE_FIELD_LENGTH; i++)
        result = result || ((a.value[i] & b.value[i]) > 0);
    return result;
}

/** A contains none of the component from B */
internal bool32
component_type_field_none(ComponentTypeField a, ComponentTypeField b)
{
    bool32 result = true;
    for (int i = 0; i < COMPONENT_TYPE_FIELD_LENGTH; i++)
        result = result && ((a.value[i] & b.value[i]) == 0);
    return result;
}