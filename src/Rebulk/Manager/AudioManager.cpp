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

        StartAmbient();
    }

    void AudioManager::StartAmbient()
    {
        ma_result result;

        result = ma_sound_init_from_file(&m_Engine, m_AmbientSounds[0].c_str(), 0, NULL, NULL, &m_AmbientSound);
        if (result != MA_SUCCESS) {
            Rbk::Log::GetLogger()->warn("Cannot init sound {}", m_AmbientSounds[0].c_str());
        }
        
        ma_sound_start(&m_AmbientSound);
    }

    void AudioManager::StopAmbient()
    {
        ma_sound_stop(&m_AmbientSound);
        ma_sound_uninit(&m_AmbientSound);
    }
}
