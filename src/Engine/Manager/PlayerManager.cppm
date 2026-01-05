export module Engine.Managers.PlayerManager;

import std;

import Engine.Core.Logger;

import Engine.Managers.ComponentManager;

import Engine.Renderer.Mesh;

import Engine.Utils.IDHelper;

namespace Poulpe
{
  export class PlayerManager
  {
    public:
      PlayerManager(
        ComponentManager * const component_manager,
        std::string const & player_name)
        : _component_manager(component_manager)
        , _player_name(player_name)
        {

        }

      std::string const& getPlayerName() { return _player_name; }
      void setPlayerId(IDType id) { _player_id = id; _is_init = true; }
      void jump();
      bool isInit() { return _is_init; }

    private:
      ComponentManager * _component_manager;

      bool _is_init { false };
      std::string _player_name = "__PLP_NO_PLAYER";
      IDType _player_id = 0;
  };
}
