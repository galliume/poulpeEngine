export module Engine.Component.EntityNode;

import std;

import Engine.Component.Entity;

namespace Poulpe
{
  export class EntityNode : public std::enable_shared_from_this<EntityNode>
  {

  public:
    EntityNode(std::shared_ptr<Entity> entity) : _entity(entity) {}

    std::shared_ptr<Entity> getEntity() const { return _entity; }
    std::shared_ptr<EntityNode> getParent() const { return _parent.lock(); }
    std::vector<std::shared_ptr<EntityNode>> const& getChildren() const { return _children; }

    std::shared_ptr<EntityNode> addChild(std::shared_ptr<EntityNode> child) {
      child->setParent(shared_from_this());
      _children.emplace_back(child);
      return _children.back();
    }

    void setParent(std::shared_ptr<EntityNode> parent) { _parent = parent; }
    void setIsLoaded(bool const is_loaded) { _is_loaded = is_loaded; }

    bool isLoaded() const { return _is_loaded; }
    bool hasChildren() { return !_children.empty(); }
    std::size_t size() const { return _children.size(); }

    void clear() { _children.clear(); }

  private:
    std::vector<std::shared_ptr<EntityNode>> _children;
    std::shared_ptr<Entity> _entity;
    std::weak_ptr<EntityNode> _parent;

    bool _is_loaded{ false };
  };
}
