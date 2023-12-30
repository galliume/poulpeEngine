#pragma once

#include "Entity.hpp"

#include <vector>

namespace Poulpe {

    class EntityNode {

    public:
        EntityNode(Entity* entity) : m_Entity(std::unique_ptr<Entity>(entity)) {};
        ~EntityNode() {};

        Entity* getEntity() const { return m_Entity.get(); }
        EntityNode* getParent() const { return m_Parent.get(); }
        std::vector<std::unique_ptr<EntityNode>>* getChildren() { return &m_Children; }

        void addChild(EntityNode* child) {

            std::unique_ptr<EntityNode> node = std::unique_ptr<EntityNode>(child);
            node->setParent(this);

            m_Children.emplace_back(std::move(node));
        }

        void setParent(EntityNode* parent) { m_Parent = std::unique_ptr<EntityNode>(parent); }

        bool hasChildren() { return !m_Children.empty(); }
        size_t size() const { return m_Children.size(); }

    private:
        std::vector<std::unique_ptr<EntityNode>> m_Children;
        std::unique_ptr<Entity> m_Entity;
        std::unique_ptr<EntityNode> m_Parent;
    };
}