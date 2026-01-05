
module Engine.Managers.PlayerManager;

import std;

import Engine.Component.Components;

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
}
