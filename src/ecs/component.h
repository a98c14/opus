#pragma once

#include <intrin.h>

#include <core/asserts.h>
#include <core/defines.h>
#include <core/log.h>
#include <core/memory.h>

#ifndef COMPONENT_COUNT
#define COMPONENT_COUNT 1
#endif
#define COMPONENT_TYPE_FIELD_SIZE   32
#define COMPONENT_TYPE_FIELD_LENGTH ((COMPONENT_COUNT / COMPONENT_TYPE_FIELD_SIZE) + 1)

#define COMPONENT_TYPE_CAPACITY 8192
typedef uint16 ComponentIndex;

enum
{
    CI_Undefined = 0,
    CI_Prefab    = 1,
    CI_RESERVE_COUNT
};

typedef enum
{
    ComponentTypeTag,
    ComponentTypeData,
    ComponentTypeChunk,
} ComponentType;

typedef struct
{
    Arena* arena;

    usize*         component_sizes;
    ComponentType* component_types;
    uint16         component_type_count;
} ComponentTypeManager;

typedef struct
{
    ComponentType* component_types;
    usize*         component_sizes;
    uint16         component_type_count;
    uint16         component_type_capacity;
} ComponentTypeRegistrationRequest;

typedef struct
{
    uint32* value;
} ComponentTypeField;

internal ComponentTypeManager*             component_type_manager_new(Arena* arena);
internal ComponentTypeRegistrationRequest* component_type_register_begin(Arena* temp_arena);
internal void                              component_type_register_add(ComponentTypeRegistrationRequest* request, ComponentIndex type_index, usize component_size, ComponentType component_type);
internal void                              component_type_register_complete(ComponentTypeManager* manager, ComponentTypeRegistrationRequest* request);

internal ComponentTypeField* component_type_field_new(Arena* arena, ComponentTypeManager* manager);
internal uint32              component_type_field_count(ComponentTypeField a);
internal bool32              component_type_field_is_same(ComponentTypeField a, ComponentTypeField b);
internal ComponentTypeField  component_type_field_add_internal(ComponentTypeField field, ComponentIndex type_index);
internal ComponentTypeField  component_type_field_remove_internal(ComponentTypeField field, ComponentIndex type_index);
#define component_type_field_add(field, type)    component_type_field_add_internal(field, CI_##type);
#define component_type_field_remove(field, type) component_type_field_remove_internal(field, CI_##type);
