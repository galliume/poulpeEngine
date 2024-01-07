#include "AudioManager.hpp"

namespace Poulpe
{
    AudioManager::~AudioManager()
    {
        ma_engine_stop(& m_Engine);
        ma_engine_uninit(& m_Engine);
    }

    std::string const AudioManager::getState()
    {
      if (State::PLAY == m_State) {
        return "playing";
      }
      else if (State::STOP == m_State) {
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
            m_AmbientSounds.emplace_back(sound.value());
        }
    }

    void AudioManager::startAmbient(unsigned int const index)
    {
      try {
        if (State::PLAY == m_State) {
          stop(m_AmbientSound);
        }

        start(m_AmbientSounds.at(index), m_AmbientSound);

        if (State::PLAY == m_State) {
          m_AmbientSoundIndex = index;
        }
      } catch (std::out_of_range const&) {
        PLP_WARN("Ambient sound index {} does not exists.", index);
      }
    }

    void AudioManager::stopAmbient()
    {
      stop(m_AmbientSound);
    }

    void AudioManager::toggleLooping()
    {
      if (ma_sound_is_looping(&m_AmbientSound)) {
        ma_sound_set_looping(&m_AmbientSound, false);
      }
      else {
        ma_sound_set_looping(&m_AmbientSound, true);
      }
    }

    void AudioManager::clear()
    {
      if (State::PLAY == m_State) {
        stopAmbient();
        ma_engine_stop(&m_Engine);
        ma_engine_uninit(&m_Engine);
      }

      m_AmbientSounds.clear();
    }

    void AudioManager::init()
    {
        m_State = State::STOP;
        ma_result result{ MA_ERROR };

        result = ma_engine_init(nullptr, & m_Engine);

        if (MA_SUCCESS != result) {
            PLP_WARN("Cannot init mini audio {}", result);
        }
    }

    void AudioManager::start(std::string const & soundPath, ma_sound & sound)
    {
        ma_result result{ MA_ERROR };
        ma_uint32 flags{ MA_SOUND_FLAG_ASYNC };

        result = ma_sound_init_from_file(& m_Engine, soundPath.c_str(), flags, nullptr, nullptr, & sound);

        if (result != MA_SUCCESS) {
            PLP_ERROR("Cannot init sound {}", soundPath.c_str());
            m_State = State::ERR;
            return;
        }

        if (m_Looping) {
            ma_sound_set_looping(& sound, true);
        }

        m_State = (MA_SUCCESS == ma_sound_start(& sound)) ? State::PLAY : State::ERR;
    }

    void AudioManager::stop(ma_sound sound)
    {
        if (State::PLAY != m_State) {
            return;
        }

        if (MA_SUCCESS == ma_sound_stop(& sound)) {
            m_State = State::STOP;
        } else {
            m_State = State::ERR;
        }

        ma_sound_uninit(& sound);
    }
}
