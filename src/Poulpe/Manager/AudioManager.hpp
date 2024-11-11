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

        inline std::vector<std::string> const getAmbientSound()  { return _AmbientSounds; }
        inline unsigned int getAmbientSoundIndex()  { return _AmbientSoundIndex; }
        inline std::string getCurrentAmbientSound()  { return _AmbientSounds[_AmbientSoundIndex]; }
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
        unsigned int _AmbientSoundIndex{ 0 };

        bool _Looping = true;

        ma_sound _AmbientSound;
        ma_engine _Engine;
        State _State;

        std::vector<std::string> _AmbientSounds {};
    };
}
