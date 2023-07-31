#include "AudioManager.hpp"

namespace Poulpe
{
    AudioManager::AudioManager()
    {

    }

    AudioManager::~AudioManager()
    {
        ma_engine_stop(&m_Engine);
        ma_engine_uninit(&m_Engine);
    }

    void AudioManager::Load(nlohmann::json config)
    {
        for (auto& sound : config["splash"].items()) {
            m_SplashSounds.emplace_back(sound.value());
        }
        for (auto& sound : config["ambient"].items()) {
            m_AmbientSounds.emplace_back(sound.value());
        }
    }

    void AudioManager::Init()
    {
        m_State = State::STOP;

        ma_result result;

        result = ma_engine_init(NULL, &m_Engine);

        if (result != MA_SUCCESS) {
            PLP_WARN("Cannot init mini audio {}", result);
        }
    }

    void AudioManager::StartAmbient(int index)
    {
        if (index > m_AmbientSounds.size() - 1 || 0 > index) {
            PLP_WARN("Ambient sound index {} does not exists.", index);
            return;
        }

        if (State::PLAY == m_State) {
            Stop(m_AmbientSound);
        }

        Start(m_AmbientSounds[index], m_AmbientSound);

        if (State::PLAY == m_State) {
            m_AmbientSoundIndex = index;
        }
    }

    void AudioManager::StopAmbient()
    {
        Stop(m_AmbientSound);
    }

    void AudioManager::StartSplash(int index)
    {
        if (index > m_SplashSounds.size() - 1 || 0 > index) {
            PLP_WARN("Splash sound index {} does not exists.", index);
            return;
        }

        if (State::PLAY == m_State) {
            Stop(m_SplashSound);
        }

        Start(m_SplashSounds[index], m_SplashSound);

        if (State::PLAY == m_State) {
            m_SplashSoundIndex = index;
        }
    }

    void AudioManager::StopSplash()
    {
        Stop(m_SplashSound);
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

        PLP_WARN("AudioManager in an unknown states.");
        return "AudioManager unknown state";
    }

    void AudioManager::ToggleLooping()
    {
        if (ma_sound_is_looping(&m_AmbientSound)) {
            ma_sound_set_looping(&m_AmbientSound, false);
        } else {
            ma_sound_set_looping(&m_AmbientSound, true);
        }
    }

    void AudioManager::Start(const std::string& soundPath, ma_sound& sound)
    {
        ma_result result;

        ma_uint32 flags = MA_SOUND_FLAG_ASYNC;
        result = ma_sound_init_from_file(&m_Engine, soundPath.c_str(), flags, NULL, NULL, &sound);

        if (result != MA_SUCCESS) {
            PLP_WARN("Cannot init sound {}", soundPath.c_str());
            m_State = State::ERR;
            return;
        }

        if (m_Looping) {
            ma_sound_set_looping(&sound, true);
        }

        if (MA_SUCCESS == ma_sound_start(&sound)) {
            m_State = State::PLAY;
        } else {
            m_State = State::ERR;
        }
    }

    void AudioManager::Stop(ma_sound sound)
    {
        if (State::PLAY != m_State) {
            return;
        }

        if (MA_SUCCESS == ma_sound_stop(&sound)) {
            m_State = State::STOP;
        }
        else {
            m_State = State::ERR;
        }

        ma_sound_uninit(&sound);
    }

    void AudioManager::Clear()
    {
        if (State::PLAY == m_State) {
            StopAmbient();
            ma_engine_stop(&m_Engine);
            ma_engine_uninit(&m_Engine);
        }
        m_AmbientSounds.clear();
        m_SplashSounds.clear();
    }
}
