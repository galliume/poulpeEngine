#pragma once

#include "Poulpe/Component/Component.hpp"

#include <cstdint>

namespace Poulpe
{
    class ComponentManager
    {
    public:

        ComponentManager() = default;
        ~ComponentManager() = default;

        template <typename T, typename IDType, typename... TArgs>
        T& addComponent(IDType entityID, TArgs&&... args)
        {
            T* newComponent(new T(std::forward<TArgs>(args)...));
            newComponent->setOwner(entityID);
            //newComponent->Initialize();

            m_ComponentTypeMap[&typeid(*newComponent)].emplace_back(newComponent);
            m_ComponentsEntityMap[entityID].emplace_back(&typeid(*newComponent));

            return *newComponent;
        }

        template <typename T>
        T* GetComponent(IDType entityID) {

            for (auto component : m_ComponentTypeMap[&typeid(T)]) {
                if (component->getOwner() == entityID) {
                    return static_cast<T*>(component);
                }
            }
            return nullptr;

            //auto it = std::find_if(m_ComponentTypeMap[&typeid(T)].begin(), m_ComponentTypeMap[&typeid(T)].end(),
            //    [&entityID](T & component) { return component.getOwner() == entityID; });

            //if (it != m_ComponentTypeMap[&typeid(T)].end()) {
            //    return static_cast<T*>(it);
            //}else {
            //    return nullptr;
            //}
        }

        template <typename T>
        bool HasComponent() const {
            return componentTypeMap.count(&typeid(T));
        }

        std::vector<const std::type_info*> getEntityComponents(IDType entityID) { return m_ComponentsEntityMap[entityID]; }

    private:
        std::unordered_map<const std::type_info*, std::vector<Component*>> m_ComponentTypeMap;
        std::unordered_map<IDType, std::vector<const std::type_info*>> m_ComponentsEntityMap;
    };
}