#pragma once

#include <base.h>
#include <intrin.h>

#ifndef COMPONENT_COUNT
#define COMPONENT_COUNT 1
#endif
#define COMPONENT_TYPE_FIELD_SIZE   32
#define COMPONENT_TYPE_FIELD_LENGTH ((COMPONENT_COUNT / COMPONENT_TYPE_FIELD_SIZE) + 1)

#define COMPONENT_CAPACITY 8192
typedef uint64 ComponentType;

/** Used by component code-generation script.
 * Define the tag component as follows
 * `typedef TagComponent EnemyComponent;` */
typedef uint32 TagComponent;

enum
{
    CT_Undefined          = 0,
    CTT_InactiveComponent = 1, // Hides the entity from all systems
    CTT_PrefabComponent   = 2,
    CTT_ParentComponent   = 3,
    CT_RESERVE_COUNT
};

typedef enum
{
    ComponentDataTypeDefault,
    ComponentDataTypeTag,
    ComponentDataTypeChunk,
} ComponentDataType;

typedef struct
{
    Arena* arena;

    usize*             component_sizes;
    ComponentDataType* component_data_types;
    uint16             component_type_count;
} ComponentTypeManager;

typedef struct
{
    ComponentType*     component_indices;
    ComponentDataType* component_data_types;
    usize*             component_sizes;
    uint16             component_type_count;
    uint16             component_type_capacity;
} ComponentTypeRegistrationRequest;

typedef struct
{
    uint32 value[COMPONENT_TYPE_FIELD_LENGTH];
} ComponentTypeField;

internal ComponentTypeManager*             component_type_manager_new(Arena* arena);
internal ComponentTypeRegistrationRequest* component_type_register_begin(Arena* temp_arena);
internal void                              component_type_register_add(ComponentTypeRegistrationRequest* request, ComponentType type_index, usize component_size, ComponentDataType component_type);
internal void                              component_type_register_complete(ComponentTypeManager* manager, ComponentTypeRegistrationRequest* request);

internal uint32             component_type_field_count(ComponentTypeField a);
internal ComponentTypeField component_type_field_or(ComponentTypeField a, ComponentTypeField b);
internal ComponentTypeField component_type_field_not(ComponentTypeField a, ComponentTypeField b);
internal ComponentTypeField component_type_field_add(ComponentTypeField field, ComponentType type_index);
internal ComponentTypeField component_type_field_remove(ComponentTypeField field, ComponentType type_index);
internal ComponentTypeField component_type_field_union(ComponentTypeField a, ComponentTypeField b);
internal void               component_type_field_set(ComponentTypeField* field, ComponentType type_index);
internal void               component_type_field_unset(ComponentTypeField* field, ComponentType type_index);
internal void               component_type_field_set_group(ComponentTypeField* field, ComponentTypeField b);
internal void               component_type_field_unset_group(ComponentTypeField* field, ComponentTypeField b);

internal bool32 component_type_field_contains(ComponentTypeField a, ComponentTypeField b);
internal bool32 component_type_field_is_same(ComponentTypeField a, ComponentTypeField b);
internal bool32 component_type_field_any(ComponentTypeField a, ComponentTypeField b);
internal bool32 component_type_field_none(ComponentTypeField a, ComponentTypeField b);
internal bool32 component_type_field_is_set(ComponentTypeField* field, ComponentType type);