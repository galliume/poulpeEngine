module Engine.Managers.AudioManager;

import std;

import Engine.Core.Json;
import Engine.Core.Logger;
import Engine.Core.MiniAudio;

namespace Poulpe
{
  AudioManager::~AudioManager()
  {
    ::ma_engine_stop(& _engine);
    ::ma_engine_uninit(& _engine);
  }

  std::string_view AudioManager::getState() const
  {
    if (State::PLAY == _state) {
      return "playing";
    } else if (State::STOP == _state) {
      return "stoped";
    } else {
      Logger::error("AudioManager in an unknown states");
      return "error";
    }
  }

  void AudioManager::load(json config)
  {
    for (auto const & sound : config["ambient"].items()) {
      _ambient_sounds.emplace_back(sound.value());
    }
  }

  void AudioManager::startAmbient(std::uint32_t const index)
  {
    try {
      if (State::PLAY == _state) {
        stop(_ambient_sound);
      }

      start(_ambient_sounds.at(index), _ambient_sound);

      if (State::PLAY == _state) {
        _ambient_sound_index = index;
      }
    } catch (std::out_of_range const&) {
      Logger::warn("Ambient sound index {} does not exists.", index);
    }
  }

  void AudioManager::stopAmbient()
  {
    stop(_ambient_sound);
  }

  void AudioManager::toggleLooping()
  {
    if (::ma_sound_is_looping(&_ambient_sound)) {
      ::ma_sound_set_looping(&_ambient_sound, false);
    }
    else {
      ::ma_sound_set_looping(&_ambient_sound, true);
    }
  }

  void AudioManager::clear()
  {
    if (State::PLAY == _state) {
      stopAmbient();
      ::ma_engine_stop(&_engine);
      ::ma_engine_uninit(&_engine);
    }

    _ambient_sounds.clear();
  }

  void AudioManager::init()
  {
    _state = State::STOP;
    ::ma_result result{ ::MA_ERROR };

    result = ::ma_engine_init(nullptr, & _engine);

    if (::MA_SUCCESS != result) {
      Logger::warn("Cannot init mini audio {}", static_cast<int>(result));
    }
  }

  void AudioManager::start(std::string const & soundPath, ::ma_sound & sound)
  {
    ::ma_result result{ ::MA_ERROR };
    ::ma_uint32 flags{ ::MA_SOUND_FLAG_ASYNC };

    result = ::ma_sound_init_from_file(& _engine, soundPath.c_str(), flags, nullptr, nullptr, & sound);

    if (result != ::MA_SUCCESS) {
      Logger::error("Cannot init sound {}", soundPath.c_str());
      _state = State::ERR;
      return;
    }

    if (_loop) {
      ::ma_sound_set_looping(& sound, true);
    }

    _state = (::MA_SUCCESS == ::ma_sound_start(& sound)) ? State::PLAY : State::ERR;
  }

  void AudioManager::stop(::ma_sound sound)
  {
    if (State::PLAY != _state) {
      return;
    }

    if (::MA_SUCCESS == ::ma_sound_stop(& sound)) {
      _state = State::STOP;
    } else {
      _state = State::ERR;
    }

    ::ma_sound_uninit(& sound);
  }
}
