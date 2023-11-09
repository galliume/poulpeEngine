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
        AudioManager();
        virtual ~AudioManager();

        virtual void load(nlohmann::json config) override;
        virtual void startSplash(int index = 0) override;
        virtual void stopSplash() override;
        virtual void startAmbient(int index = 0) override;
        virtual void stopAmbient() override;
        virtual std::vector<std::string> const getAmbientSound() override { return m_AmbientSounds; }
        virtual std::string const getState() override;
        virtual int getAmbientSoundIndex() override { return m_AmbientSoundIndex; }
        virtual std::string getCurrentAmbientSound() override { return m_AmbientSounds[m_AmbientSoundIndex]; }
        virtual void toggleLooping() override;
        void clear();
        void init();

    private:
        void start(const std::string& soundPath, ma_sound& sound);
        void stop(ma_sound sound);

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
