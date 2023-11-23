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

        void setParent(EntityNode* parent) { m_Parent = parent; }

        void addChild(Entity* child) {

            EntityNode node{ child };
            node.setParent(this);

            m_Children.emplace_back(node);
        }

        bool hasChildren() { return !m_Children.empty(); }

    private:
        Entity* m_Entity;
        EntityNode* m_Parent;

        std::vector<EntityNode> m_Children{};
    };
}