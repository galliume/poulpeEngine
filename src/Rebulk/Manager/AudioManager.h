#pragma once
#include "Rebulk/Component/Camera.h"
#include "Rebulk/GUI/Window.h"

namespace Rbk
{
    class AudioManager
    {
    public:
        AudioManager();
        ~AudioManager();
        void LoadAmbient();

        void StartAmbient();
        void StopAmbient();
        std::vector<std::string> const GetAmbientSound() { return m_AmbientSounds; };

    private:
        ma_engine m_Engine;
        ma_sound m_AmbientSound;

        std::vector<std::string> m_AmbientSounds {};
    };
}
