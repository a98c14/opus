#include "input_core.h"

internal void
input_key_callback(OS_KeyCode key_code, OS_KeyState state)
{
    xassert(_input_context != NULL, "Input layer hasn't been initialized. Call `input_init` before using input layer callbacks!");
    log_debug("Key callback called. Key: %d, State: %d", key_code, state);

    Input_Event new_event = {0};
    new_event.key_code    = key_code;
    new_event.state       = state;

    _input_context->input_events[_input_context->input_event_buffer_end % _INPUT_EVENT_BUFFER_SIZE] = new_event;
    _input_context->input_event_buffer_end++;
}

internal void
input_init(Arena* arena)
{
    _input_context                   = arena_push_struct_zero(arena, Input_Context);
    _input_context->persistent_arena = arena;
}

internal void
input_update(float32 dt)
{
    Input_Context* ctx = _input_context;

    ctx->t_frame += dt;

    /** clear `new` status of keys from previous frame */
    for (uint32 i = 0; i < ctx->key_count; i++)
    {
        OS_KeyCode key_code = ctx->registered_key_codes[i];
        Input_Key* key      = &ctx->keys[key_code];
        if (flag_is_set(key->state, Input_KeyStateNew))
        {
            key->state = flag_clear(key->state, Input_KeyStateNew);
        }
    }

    /** process events */
    for (uint64 event_index = ctx->input_event_buffer_start; event_index < ctx->input_event_buffer_end; event_index++)
    {
        uint64      event_buffer_index = event_index % _INPUT_EVENT_BUFFER_SIZE;
        Input_Event event              = ctx->input_events[event_buffer_index];

        Input_Key* key = &ctx->keys[event.key_code];
        if (!key->is_initialized)
            continue;

        if (!flag_is_set(key->state, Input_KeyStateReleased) && event.state == OS_KeyState_Released)
        {
            log_debug("Registered action released. Action Name: %s", key->name.value);
            key->state = Input_KeyStateReleasedNew;
        }
        else if (!flag_is_set(key->state, Input_KeyStatePressed) && event.state == OS_KeyState_Pressed)
        {
            log_debug("Registered action pressed. Action Name: %s", key->name.value);
            key->state = Input_KeyStatePressedNew;
        }
    }
    ctx->input_event_buffer_start = ctx->input_event_buffer_end;

    Input_MouseInfo result = {0};
    result.raw             = os_input_mouse_pos();
    result.screen          = gfx_window_to_screen_position(result.raw);
    result.world           = gfx_window_to_world_position(result.raw);
    ctx->cached_mouse_info = result;
}

internal void
input_register_key(String action_name, OS_KeyCode code)
{
    uint32 registered_key_id = _input_context->key_count;
    _input_context->key_count++;
    _input_context->registered_key_codes[registered_key_id] = code;

    uint64 hash  = hash_string(action_name);
    uint64 index = hash % _INPUT_ACTION_MAP_TABLE_SIZE;

    _input_context->keys[code].name           = action_name;
    _input_context->keys[code].is_initialized = true;

    Input_KeyNode* node = arena_push_struct_zero(_input_context->persistent_arena, Input_KeyNode);
    node->hash          = hash;
    node->key_code      = code;

    // TODO(selim): check if it already exists in bucket
    Input_KeyBucket* bucket = &_input_context->key_index_map[index];
    queue_push(bucket->first, bucket->last, node);
}

internal Input_MouseInfo
input_mouse_info(void)
{
    return _input_context->cached_mouse_info;
}

internal bool32
input_is_pressed(String action_name)
{
    Input_Key* key = _input_key_from_action(action_name);
    if (!key)
        return false;

    bool32 is_pressed = flag_is_set(key->state, Input_KeyStatePressedNew);
    return is_pressed;
}

internal bool32
input_is_held(String action_name)
{
    Input_Key* key = _input_key_from_action(action_name);
    if (!key)
        return false;

    bool32 is_held = flag_is_set(key->state, Input_KeyStatePressed);
    return is_held;
}

internal bool32
input_is_released(String action_name)
{
    Input_Key* key = _input_key_from_action(action_name);
    if (!key)
        return false;

    return flag_is_set(key->state, Input_KeyStateReleasedNew);
}

/** Private Functions */
internal Input_Key*
_input_key_from_action(String action_name)
{
    uint64 hash  = hash_string(action_name);
    uint64 index = hash % _INPUT_ACTION_MAP_TABLE_SIZE;

    Input_KeyBucket* bucket = &_input_context->key_index_map[index];
    Input_KeyNode*   node;
    for_each(node, bucket->first)
    {
        if (node->hash == hash)
            break;
    }

    if (!node)
        return NULL;

    Input_Key* key = &_input_context->keys[node->key_code];
    return key;
}
