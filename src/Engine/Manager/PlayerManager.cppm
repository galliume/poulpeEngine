export module Engine.Managers.PlayerManager;

import std;

import Engine.Core.Logger;

namespace Poulpe
{
  export class PlayerManager
  {
    public:
      PlayerManager(std::string const & player_name)
        : _player_name(player_name)
        {

        }

    private:
      std::string _player_name;
  };
}
