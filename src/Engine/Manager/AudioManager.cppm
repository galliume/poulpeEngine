export module Engine.Managers.AudioManager;

import std;

import Engine.Core.Json;
import Engine.Core.MiniAudio;

namespace Poulpe
{
  export class AudioManager
  {
    enum class State : std::uint8_t
    {
      STOP,
      PLAY,
      ERR
    };

  public:
    AudioManager() = default;
    ~AudioManager();

    std::vector<std::string> const getAmbientSound()  { return _ambient_sounds; }
    std::uint32_t getAmbientSoundIndex()  { return _ambient_sound_index; }
    std::string getCurrentAmbientSound()  { return _ambient_sounds[_ambient_sound_index]; }
    std::string const getState() ;

    void load(json config);
    void startAmbient(std::uint32_t const index = 0);
    void stopAmbient();
    void toggleLooping();

    void clear();
    void init();

  private:
    void start(std::string const & sound_path, ::ma_sound & sound);
    void stop(::ma_sound sound);

    std::uint32_t _ambient_sound_index{ 0 };

    bool _loop = true;

    ::ma_sound _ambient_sound;
    ::ma_engine _engine;
    State _state;

    std::vector<std::string> _ambient_sounds {};
  };
}
