#pragma once

#include "Poulpe/Component/MeshComponent.hpp"
#include "Poulpe/Component/RenderComponent.hpp"

#include <cstdint>

namespace Poulpe
{
    class ComponentManager
    {
    public:

        ComponentManager() = default;
        ~ComponentManager() = default;

        template <typename T, typename IDType, typename Component, typename... TArgs>
        T& addComponent(IDType entityID, Component* componentImpl, TArgs&&... args)
        {
            T* newComponent(new T(std::forward<TArgs>(args)...));
            newComponent->init(componentImpl);
            newComponent->setOwner(entityID);

            m_ComponentTypeMap[&typeid(*newComponent)].emplace_back(newComponent);
            m_ComponentsEntityMap[entityID].emplace_back(&typeid(*newComponent));

            return *newComponent;
        }

        template <typename T>
        T* getComponent(IDType entityID) {

            for (auto component : m_ComponentTypeMap[&typeid(T)]) {
                if (component->getOwner() == entityID) {
                    return static_cast<T*>(component);
                }
            }
            return nullptr;
        }

        std::vector<const std::type_info*> getEntityComponents(IDType entityID) { return m_ComponentsEntityMap[entityID]; }
        void clear();

    private:
        std::unordered_map<const std::type_info*, std::vector<Component*>> m_ComponentTypeMap;
        std::unordered_map<IDType, std::vector<const std::type_info*>> m_ComponentsEntityMap;
    };
}
