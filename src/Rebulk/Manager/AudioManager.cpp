#include "rebulkpch.h"
#include "AudioManager.h"
#include <miniaudio.h>

namespace Rbk
{
    AudioManager::AudioManager()
    {

        ma_result result;

        result = ma_engine_init(NULL, &m_Engine);

        if (result != MA_SUCCESS) {
            Rbk::Log::GetLogger()->warn("Cannot init mini audio {}", result);
        }
    }

    AudioManager::~AudioManager()
    {
        StopAmbient();
        ma_engine_stop(&m_Engine);
        ma_engine_uninit(&m_Engine);
    }

    void AudioManager::LoadAmbient()
    {
        m_AmbientSounds.emplace_back("assets/sounds/ambient/wind__artic__cold-6195.mp3");
        m_AmbientSounds.emplace_back("assets/sounds/ambient/67884__benboncan__lake-waves-2.wav");

        StartAmbient();
    }

    void AudioManager::StartAmbient(int index)
    {
        if (index > m_AmbientSounds.size() - 1 || 0 > index) {
            return Rbk::Log::GetLogger()->warn("Ambient sound index {} does not exists.", index);
        }

        if (State::PLAY == m_State) {
            StopAmbient();
        }

        ma_result result;

        ma_uint32 flags = MA_SOUND_FLAG_ASYNC;
        result = ma_sound_init_from_file(&m_Engine, m_AmbientSounds[index].c_str(), flags, NULL, NULL, &m_AmbientSound);

        if (m_Looping) {
            ma_sound_set_looping(&m_AmbientSound, true);
        }

        if (result != MA_SUCCESS) {
            Rbk::Log::GetLogger()->warn("Cannot init sound {}", m_AmbientSounds[index].c_str());
        }

        if (MA_SUCCESS == ma_sound_start(&m_AmbientSound)) {
            m_AmbientSoundIndex = index;
            m_State = State::PLAY;
        } else {
            m_State = State::ERR;
        }
    }

    void AudioManager::StopAmbient()
    {
        if (State::PLAY != m_State) {
            return;
        }

        if (MA_SUCCESS == ma_sound_stop(&m_AmbientSound)) {
            m_State = State::STOP;
        } else {
            m_State = State::ERR;
        }

        ma_sound_uninit(&m_AmbientSound);
    }

    std::string const AudioManager::GetState()
    {
        if (State::PLAY == m_State) {
            return "playing";
        } else if (State::STOP == m_State) {
            return "stoped";
        } else if (State::ERR == m_State) {
            return "an error occured";
        }
    }

    void AudioManager::ToggleLooping()
    {
        if (ma_sound_is_looping(&m_AmbientSound)) {
            ma_sound_set_looping(&m_AmbientSound, false);
        } else {
            ma_sound_set_looping(&m_AmbientSound, true);
        }
    }
}
