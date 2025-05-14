module;

#include <miniaudio.h>
#include <nlohmann/json.hpp>

#include <string>
#include <vector>

export module Poulpe.Manager.AudioManager;

import Poulpe.Core.Logger;

namespace Poulpe
{
  export class AudioManager
  {
    enum class State {
      STOP,
      PLAY,
      ERR
    };

  public:
    AudioManager() = default;
    ~AudioManager();

    std::vector<std::string> const getAmbientSound()  { return _ambient_sounds; }
    unsigned int getAmbientSoundIndex()  { return _ambient_sound_index; }
    std::string getCurrentAmbientSound()  { return _ambient_sounds[_ambient_sound_index]; }
    std::string const getState() ;

    void load(nlohmann::json config) ;
    void startAmbient(unsigned int const index = 0) ;
    void stopAmbient() ;
    void toggleLooping() ;

    void clear();
    void init();

  private:
    void start(std::string const & sound_path, ::ma_sound & sound);
    void stop(::ma_sound sound);

    unsigned int _ambient_sound_index{ 0 };

    bool _loop = true;

    ::ma_sound _ambient_sound;
    ::ma_engine _engine;
    State _state;

    std::vector<std::string> _ambient_sounds {};
  };
}
