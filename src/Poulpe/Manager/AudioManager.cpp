#include "AudioManager.hpp"

namespace Poulpe
{
    AudioManager::~AudioManager()
    {
        ma_engine_stop(& _Engine);
        ma_engine_uninit(& _Engine);
    }

    std::string const AudioManager::getState()
    {
      if (State::PLAY == _State) {
        return "playing";
      }
      else if (State::STOP == _State) {
        return "stoped";
      }
      else {
        PLP_ERROR("AudioManager in an unknown states");
        return "error";
      }
    }

    void AudioManager::load(nlohmann::json config)
    {
        for (auto const & sound : config["ambient"].items()) {
            _AmbientSounds.emplace_back(sound.value());
        }
    }

    void AudioManager::startAmbient(unsigned int const index)
    {
      try {
        if (State::PLAY == _State) {
          stop(_AmbientSound);
        }

        start(_AmbientSounds.at(index), _AmbientSound);

        if (State::PLAY == _State) {
          _AmbientSoundIndex = index;
        }
      } catch (std::out_of_range const&) {
        PLP_WARN("Ambient sound index {} does not exists.", index);
      }
    }

    void AudioManager::stopAmbient()
    {
      stop(_AmbientSound);
    }

    void AudioManager::toggleLooping()
    {
      if (ma_sound_is_looping(&_AmbientSound)) {
        ma_sound_set_looping(&_AmbientSound, false);
      }
      else {
        ma_sound_set_looping(&_AmbientSound, true);
      }
    }

    void AudioManager::clear()
    {
      if (State::PLAY == _State) {
        stopAmbient();
        ma_engine_stop(&_Engine);
        ma_engine_uninit(&_Engine);
      }

      _AmbientSounds.clear();
    }

    void AudioManager::init()
    {
        _State = State::STOP;
        ma_result result{ MA_ERROR };

        result = ma_engine_init(nullptr, & _Engine);

        if (MA_SUCCESS != result) {
            PLP_WARN("Cannot init mini audio {}", result);
        }
    }

    void AudioManager::start(std::string const & soundPath, ma_sound & sound)
    {
        ma_result result{ MA_ERROR };
        ma_uint32 flags{ MA_SOUND_FLAG_ASYNC };

        result = ma_sound_init_from_file(& _Engine, soundPath.c_str(), flags, nullptr, nullptr, & sound);

        if (result != MA_SUCCESS) {
            PLP_ERROR("Cannot init sound {}", soundPath.c_str());
            _State = State::ERR;
            return;
        }

        if (_Looping) {
            ma_sound_set_looping(& sound, true);
        }

        _State = (MA_SUCCESS == ma_sound_start(& sound)) ? State::PLAY : State::ERR;
    }

    void AudioManager::stop(ma_sound sound)
    {
        if (State::PLAY != _State) {
            return;
        }

        if (MA_SUCCESS == ma_sound_stop(& sound)) {
            _State = State::STOP;
        } else {
            _State = State::ERR;
        }

        ma_sound_uninit(& sound);
    }
}
