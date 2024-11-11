#pragma once

#include "Entity.hpp"

#include <vector>

namespace Poulpe {

  class EntityNode {

  public:
    EntityNode(Entity* entity) : _Entity(std::unique_ptr<Entity>(entity)) {}
    ~EntityNode() = default;

    Entity* getEntity() const { return _Entity.get(); }
    EntityNode* getParent() const { return _Parent.get(); }
    std::vector<EntityNode*> getChildren() { return _Children; }

    EntityNode* addChild(EntityNode* child) {
      child->setParent(this);
      _Children.emplace_back(std::move(child));
      return _Children.back();
    }

    void setParent(EntityNode* parent) { _Parent = std::unique_ptr<EntityNode>(parent); }

    bool hasChildren() { return !_Children.empty(); }
    size_t size() const { return _Children.size(); }

  private:
    std::vector<EntityNode*> _Children;
    std::unique_ptr<Entity> _Entity;
    std::unique_ptr<EntityNode> _Parent;
  };
}
