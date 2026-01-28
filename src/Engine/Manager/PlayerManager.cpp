module Engine.Managers.PlayerManager;

import std;

import Engine.Component.Components;

import Engine.Core.Logger;
import Engine.Core.GLM;

/**
 * @todo States machine for movements / animations
 */
namespace Poulpe
{
  void PlayerManager::jump()
  {
    auto* boneAnimationComponent { _component_manager->get<BoneAnimationComponent>(_player_id) };
    if (boneAnimationComponent) {
      (*boneAnimationComponent).setAnimId(8);
      (*boneAnimationComponent).reset();
    }
  }

  void PlayerManager::move(float const lx, float const ly, double const delta_time)
  {
      auto* boneAnimationComponent { _component_manager->get<BoneAnimationComponent>(_player_id) };
      if (boneAnimationComponent) {
        (*boneAnimationComponent).setAnimId(0);
        (*boneAnimationComponent).reset();
      }

      auto* player { _component_manager->get<MeshComponent>(_player_id) };
      auto player_mesh { player->template has<Mesh>() };
      auto const& data { player_mesh->getData() };

      float const speed { 10.0f };
      data->_current_pos.x -= lx * speed * static_cast<float>(delta_time);
      data->_current_pos.z -= ly * speed * static_cast<float>(delta_time);

      auto const T { glm::translate(glm::mat4(1.0f), data->_current_pos) };
      auto const R { glm::mat4_cast(data->_current_rotation) };
      auto const S { glm::scale(glm::mat4(1.0f), data->_current_scale) };

      glm::mat4 const model { T * R * S * data->_inverse_transform_matrix * data->_transform_matrix };

      for (std::size_t i{ 0 }; i < data->_ubos.size(); i++) {
        std::ranges::for_each(data->_ubos.at(i), [&model](auto& ubo) {
          ubo.model = model;
        });
      }

    _position = data->_current_pos;
    _moved = true;
  }
}
