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

      m_ComponentTypeMap[&typeid(newComponent)].emplace_back(std::move(newComponent));
      m_ComponentsEntityMap[entityID].emplace_back(&typeid(newComponent));

      //return newComponent;
    }

    template <typename T>
    T* get(IDType entityID) {

      auto checkID = [entityID](auto& comp) {
        return std::visit([entityID](auto& concreteComp) {
          return concreteComp->getID() == entityID;
          }, comp);
      };

      auto component = std::ranges::find_if(m_ComponentTypeMap[&typeid(T)], checkID);

      if (component != m_ComponentTypeMap[&typeid(T)].end()) {
        return (std::get_if<std::unique_ptr<T>>(&*component))->get();
      }
      return nullptr;
    }

    std::vector<const std::type_info*> getEntityComponents(IDType entityID) { return m_ComponentsEntityMap[entityID]; }
    void clear();

  private:
    std::unordered_map<const std::type_info*, std::vector<Components>> m_ComponentTypeMap;
    std::unordered_map<IDType, std::vector<const std::type_info*>> m_ComponentsEntityMap;
  };
}
