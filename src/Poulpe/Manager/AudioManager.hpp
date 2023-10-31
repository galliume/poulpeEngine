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

        virtual void Load(nlohmann::json config) override;
        virtual void StartSplash(int index = 0) override;
        virtual void StopSplash() override;
        virtual void StartAmbient(int index = 0) override;
        virtual void StopAmbient() override;
        virtual std::vector<std::string> const GetAmbientSound() override { return m_AmbientSounds; }
        virtual std::string const GetState() override;
        virtual int GetAmbientSoundIndex() override { return m_AmbientSoundIndex; }
        virtual std::string GetCurrentAmbientSound() override { return m_AmbientSounds[m_AmbientSoundIndex]; }
        virtual void ToggleLooping() override;
        void Clear();
        void Init();

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
