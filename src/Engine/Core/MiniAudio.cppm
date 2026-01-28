module;

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

export module Engine.Core.MiniAudio;

export
{
  using ::MA_SUCCESS;
  using ::MA_ERROR;
  using ::MA_SOUND_FLAG_ASYNC;

  using ::ma_sound;
  using ::ma_engine;
  using ::ma_result;
  using ::ma_uint32;

  using ::ma_engine_init;
  using ::ma_engine_stop;
  using ::ma_engine_uninit;
  using ::ma_sound_init_from_file;
  using ::ma_sound_uninit;
  using ::ma_sound_start;
  using ::ma_sound_stop;
  using ::ma_sound_is_looping;
  using ::ma_sound_set_looping;
}
