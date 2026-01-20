module;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

export module Engine.Managers.PlayerManager;

import std;

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
      bool hasMoved() const { return _moved; }
      void reset() { _moved = false; }

      void move(float const lx, float const ly, double const delta_time);
      glm::vec3 getThirdPersonCameraPos() const;

    private:
      ComponentManager * _component_manager;

      bool _is_init { false };
      bool _moved { false };
      
      std::string _player_name = "__PLP_NO_PLAYER";
      IDType _player_id = 0;

      glm::vec3 _third_person_camera_pos {};
  };
}
