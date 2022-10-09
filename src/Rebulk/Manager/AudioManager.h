#pragma once
#include "Rebulk/Component/Camera.h"
#include "Rebulk/GUI/Window.h"

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
        void LoadAmbient();

        void StartAmbient(int index = 0);
        void StopAmbient();
        std::vector<std::string> const GetAmbientSound() { return m_AmbientSounds; };
        std::string const GetState();
        int const GetAmbientSoundIndex() { return m_AmbientSoundIndex; };
        std::string const GetCurrentAmbientSound() { return m_AmbientSounds[m_AmbientSoundIndex]; };
        void ToggleLooping();

    private:
        ma_engine m_Engine;
        ma_sound m_AmbientSound;
        State m_State;
        int m_AmbientSoundIndex = 0;
        bool m_Looping = true;
        std::vector<std::string> m_AmbientSounds {};
    };
}
