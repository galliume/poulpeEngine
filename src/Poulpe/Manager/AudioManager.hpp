#pragma once

#include "IAudioManager.hpp"

namespace Poulpe
{
    class AudioManager : IAudioManager
    {
        enum class State {
            STOP,
            PLAY,
            ERR
        };

    public:
        AudioManager() = default;
        virtual ~AudioManager();

        void load(nlohmann::json config) override;
        void startSplash(unsigned int const index = 0) override;
        void stopSplash() override;
        void startAmbient(unsigned int const index = 0) override;
        void stopAmbient() override;
        std::vector<std::string> const getAmbientSound() override { return m_AmbientSounds; }
        std::string const getState() override;
        unsigned int getAmbientSoundIndex() override { return m_AmbientSoundIndex; }
        std::string getCurrentAmbientSound() override { return m_AmbientSounds[m_AmbientSoundIndex]; }
        void toggleLooping() override;
        void clear();
        void init();

    private:
        void start(std::string const & soundPath, ma_sound & sound);
        void stop(ma_sound sound);

    private:
        ma_engine m_Engine;
        ma_sound m_AmbientSound;
        ma_sound m_SplashSound;
        State m_State;
        unsigned int m_AmbientSoundIndex{ 0 };
        unsigned int m_SplashSoundIndex{ 0 };
        bool m_Looping = true;
        std::vector<std::string> m_AmbientSounds {};
        std::vector<std::string> m_SplashSounds {};
    };
}
