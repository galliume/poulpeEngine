module;
#include <vector>
#include <memory>

export module Engine.Component.EntityNode;

import Engine.Component.Entity;

namespace Poulpe
{
  export class EntityNode 
  {

  public:
    EntityNode(Entity* entity) : _entity(std::unique_ptr<Entity>(entity)) {}

    Entity* getEntity() const { return _entity.get(); }
    EntityNode* getParent() const { return _parent.get(); }
    std::vector<EntityNode*> const& getChildren() const { return _children; }

    EntityNode* addChild(EntityNode* child) {
      child->setParent(this);
      _children.emplace_back(std::move(child));
      return _children.back();
    }

    void setParent(EntityNode* parent) { _parent = std::unique_ptr<EntityNode>(parent); }
    void setIsLoaded(bool const is_loaded) { _is_loaded = is_loaded; }

    bool isLoaded() const { return _is_loaded; }
    bool hasChildren() { return !_children.empty(); }
    size_t size() const { return _children.size(); }

    void clear() { _children.clear(); }

  private:
    std::vector<EntityNode*> _children;
    std::unique_ptr<Entity> _entity;
    std::unique_ptr<EntityNode> _parent;

    bool _is_loaded{ false };
  };
}
