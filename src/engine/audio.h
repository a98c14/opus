#pragma once
#include <base.h>

internal void oe_audio_init();
internal void oe_audio_shutdown();
internal void oe_audio_stream_callback(float32* buffer, int32 num_frames, int32 num_channels);