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

        inline std::vector<std::string> const getAmbientSound()  { return _ambient_sounds; }
        inline unsigned int getAmbientSoundIndex()  { return _ambient_sound_index; }
        inline std::string getCurrentAmbientSound()  { return _ambient_sounds[_ambient_sound_index]; }
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
        unsigned int _ambient_sound_index{ 0 };

        bool _Looping = true;

        ma_sound _ambient_sound;
        ma_engine _engine;
        State _state;

        std::vector<std::string> _ambient_sounds {};
    };
}
