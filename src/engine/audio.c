#include "audio.h"
#include "soloud_c.h"

internal void
oe_audio_logger(const char* tag,              // always "saudio"
                uint32_t    log_level,        // 0=panic, 1=error, 2=warning, 3=info
                uint32_t    log_item_id,      // SAUDIO_LOGITEM_*
                const char* message_or_null,  // a message string, may be nullptr in release mode
                uint32_t    line_nr,          // line number in sokol_audio.h
                const char* filename_or_null, // source filename, may be nullptr in release mode
                void*       user_data)
{
    // NOTE: sokol log levels match 1 to 1 with opus log levels so we don't need any conversion;
    log_output((LogLevel)log_level, "Audio Id: %d, message: %s, in file %s, at line %d", log_item_id, message_or_null, filename_or_null, line_nr);
}

internal void
oe_audio_init()
{
    Soloud* soloud = Soloud_create();
    Soloud_initEx(soloud, SOLOUD_CLIP_ROUNDOFF | SOLOUD_ENABLE_VISUALIZATION, SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO);

    Wav* wav = Wav_create();
    Wav_load(wav, "C:\\Users\\selim\\source\\github\\space_fighter\\assets\\audio\\gun.mp3");
    Soloud_play(soloud, wav);
}

internal void
oe_audio_shutdown()
{
    // saudio_shutdown();
}

internal void
oe_audio_stream_callback(float32* buffer, int32 num_frames, int32 num_channels)
{
    const int32 num_samples = num_frames * num_channels;
    for (int32 i = 0; i < num_samples; i++)
    {
        buffer[i] = 0.0f;
    }
}