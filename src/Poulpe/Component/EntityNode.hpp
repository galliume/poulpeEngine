#pragma once

#include "Entity.hpp"

#include <vector>

namespace Poulpe {

    class EntityNode {

    public:
        EntityNode(Entity* entity) : m_Entity(entity) {};
        
        Entity* getEntity() const { return m_Entity; }
        EntityNode* getParent() const { return m_Parent; }
        std::vector<EntityNode> getChildren() const { return m_Children; }

        void addChild(Entity* const child) {

            EntityNode node{ child };
            node.setParent(this);

            m_Children.emplace_back(node);
        }

        void setParent(EntityNode* const parent) { m_Parent = parent; }

        void clear() { m_Children.clear(); }
        bool hasChildren() { return !m_Children.empty(); }

    private:
        std::vector<EntityNode> m_Children{};

        Entity* m_Entity;
        EntityNode* m_Parent;
    };
}