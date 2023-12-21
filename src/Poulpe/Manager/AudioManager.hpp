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

        inline std::vector<std::string> const getAmbientSound() override { return m_AmbientSounds; }
        inline unsigned int getAmbientSoundIndex() override { return m_AmbientSoundIndex; }
        inline std::string getCurrentAmbientSound() override { return m_AmbientSounds[m_AmbientSoundIndex]; }
        std::string const getState() override;

        void load(nlohmann::json config) override;
        void startAmbient(unsigned int const index = 0) override;
        void stopAmbient() override;
        void toggleLooping() override;

        void clear();
        void init();

    private:
        void start(std::string const & soundPath, ma_sound & sound);
        void stop(ma_sound sound);

    private:
        unsigned int m_AmbientSoundIndex{ 0 };

        bool m_Looping = true;

        ma_sound m_AmbientSound;
        ma_engine m_Engine;
        State m_State;

        std::vector<std::string> m_AmbientSounds {};
    };
}
