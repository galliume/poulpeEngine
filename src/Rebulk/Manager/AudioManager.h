#pragma once
#include "Rebulk/Component/Camera.h"
#include "Rebulk/GUI/Window.h"
#include <json.hpp>

namespace Rbk
{
    class AudioManager
    {
        enum class State {
            STOP,
            PLAY,
            ERR
        };

    public:
        AudioManager();
        ~AudioManager();
        void Load(nlohmann::json config);

        void StartSplash(int index = 0);
        void StopSplash();
        void StartAmbient(int index = 0);
        void StopAmbient();
        std::vector<std::string> const GetAmbientSound() { return m_AmbientSounds; };
        std::string const GetState();
        int const GetAmbientSoundIndex() { return m_AmbientSoundIndex; };
        std::string const GetCurrentAmbientSound() { return m_AmbientSounds[m_AmbientSoundIndex]; };
        void ToggleLooping();

    private:
        void Start(const std::string& soundPath, ma_sound& sound);
        void Stop(ma_sound sound);

    private:
        ma_engine m_Engine;
        ma_sound m_AmbientSound;
        ma_sound m_SplashSound;
        State m_State;
        int m_AmbientSoundIndex = 0;
        int m_SplashSoundIndex = 0;
        bool m_Looping = true;
        std::vector<std::string> m_AmbientSounds {};
        std::vector<std::string> m_SplashSounds {};
    };
}
