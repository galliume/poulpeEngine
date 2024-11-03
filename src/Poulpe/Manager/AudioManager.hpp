#pragma once

#include <miniaudio.h>

#include <nlohmann/json.hpp>

namespace Poulpe
{
    class AudioManager
    {
        enum class State {
            STOP,
            PLAY,
            ERR
        };

    public:
        AudioManager() = default;
        ~AudioManager() ;

        inline std::vector<std::string> const getAmbientSound()  { return m_AmbientSounds; }
        inline unsigned int getAmbientSoundIndex()  { return m_AmbientSoundIndex; }
        inline std::string getCurrentAmbientSound()  { return m_AmbientSounds[m_AmbientSoundIndex]; }
        std::string const getState() ;

        void load(nlohmann::json config) ;
        void startAmbient(unsigned int const index = 0) ;
        void stopAmbient() ;
        void toggleLooping() ;

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
