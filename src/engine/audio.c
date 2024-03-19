#include "audio.h"

internal void
oe_audio_init(Arena* arena)
{
    Soloud* soloud = Soloud_create();
    Soloud_initEx(soloud, SOLOUD_CLIP_ROUNDOFF, SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO);

    g_oe_audio_system               = arena_push_struct_zero(arena, OE_AudioSystem);
    g_oe_audio_system->_backend     = soloud;
    g_oe_audio_system->audio_buffer = arena_push_array_zero(arena, OE_Audio, OE_AUDIO_CACHE_SIZE);
}

internal void
oe_audio_shutdown()
{
    Soloud_destroy(g_oe_audio_system->_backend);
}

internal OE_AudioHandle
oe_audio_handle_from_path(String path)
{
    profiler_begin("oe_audio_handle_from_path");
    OE_AudioHandle result;

    uint64 hash  = hash_string(path);
    bool32 found = false;
    for (uint32 i = 0; i < g_oe_audio_system->audio_count; i++)
    {
        if (g_oe_audio_system->audio_buffer[i].hash != hash)
            continue;

        result.v[0] = i;
        found       = true;
        break;
    }

    if (!found)
    {
        uint64    index = g_oe_audio_system->audio_count;
        OE_Audio* audio = &g_oe_audio_system->audio_buffer[index];
        g_oe_audio_system->audio_count++;

        found        = true;
        audio->_data = Wav_create();
        audio->hash  = hash;
        Wav_load(audio->_data, path.value);
        result.v[0] = index;
    }

    profiler_end();
    return result;
}

internal void
oe_audio_play(OE_AudioHandle handle)
{
    OE_Audio audio         = g_oe_audio_system->audio_buffer[handle.v[0]];
    uint32   soloud_handle = Soloud_play(g_oe_audio_system->_backend, audio._data);
    Soloud_setRelativePlaySpeed(g_oe_audio_system->_backend, soloud_handle, random_between_f32(0.9, 1.1));
    Soloud_setVolume(g_oe_audio_system->_backend, soloud_handle, 0.2);
}