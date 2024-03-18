/** OPUS ENGINE AUDIO
 * It's a wrapper around SoLoud for now
 */
#pragma once
#include <base.h>

#include "profiler.h"

#include "soloud_c.h"

#define OE_AUDIO_CACHE_SIZE 128

typedef struct
{
    uint64 v[1];
} OE_AudioHandle;

typedef struct
{
    uint64 hash;
    Wav*   _data;
} OE_Audio;

typedef struct
{
    Soloud* _backend;

    uint32    audio_count;
    OE_Audio* audio_buffer;
} OE_AudioSystem;

global OE_AudioSystem* g_oe_audio_system;

internal void           oe_audio_init(Arena* arena);
internal void           oe_audio_shutdown();
internal OE_AudioHandle oe_audio_handle_from_path(String path);
internal void           oe_audio_play(OE_AudioHandle handle);