#pragma once

#include "Poulpe/Component/AnimationComponent.hpp"
#include "Poulpe/Component/BoneAnimationComponent.hpp"
#include "Poulpe/Component/MeshComponent.hpp"
#include "Poulpe/Component/RenderComponent.hpp"

#include <cstdint>
#include <optional>
#include <variant>

namespace Poulpe
{
  class ComponentManager
  {
  public:

    using Components = std::variant<
      std::unique_ptr<AnimationComponent>,
      std::unique_ptr<BoneAnimationComponent>,
      std::unique_ptr<MeshComponent>,
      std::unique_ptr<RenderComponent>>;

    ComponentManager() = default;
    ~ComponentManager() = default;

    template <typename T, typename IDType, typename Component>
    void add(IDType entityID, Component componentImpl)
    {
      auto newComponent = std::make_unique<T>();
      newComponent->init(std::move(componentImpl));
      newComponent->setOwner(entityID);

      _ComponentTypeMap[&typeid(T)].emplace_back(std::move(newComponent));
      _ComponentsEntityMap[entityID].emplace_back(&typeid(T));
    }

    template <typename T>
    T* get(IDType entityID) {
      auto it = std::ranges::find_if(_ComponentTypeMap[&typeid(T)], [&entityID](auto& component) {
        if (auto ptr = std::get_if<std::unique_ptr<T>>(&component)) {
          if (ptr) {
            return (*ptr)->getOwner() == entityID;
          }
        }
        return false;
      });
      
      if (it != _ComponentTypeMap[&typeid(T)].end()) {
        if (auto ptr = std::get_if<std::unique_ptr<T>>(&*it)) {
            return ptr->get();
        }
      }
      return nullptr;
    }

    std::vector<const std::type_info*> getEntityComponents(IDType entityID) { return _ComponentsEntityMap[entityID]; }
    void clear();

  private:
    std::unordered_map<const std::type_info*, std::vector<Components>> _ComponentTypeMap;
    std::unordered_map<IDType, std::vector<const std::type_info*>> _ComponentsEntityMap;
  };
}
