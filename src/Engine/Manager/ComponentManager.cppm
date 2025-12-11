export module Engine.Managers.ComponentManager;

import std;

import Engine.Animation.AnimationScript;
import Engine.Animation.BoneAnimationScript;

import Engine.Component.Components;
import Engine.Renderer;
import Engine.Renderer.RendererComponent;
import Engine.Renderer.Mesh;

import Engine.Utils.IDHelper;

namespace Poulpe
{
  export class ComponentManager
  {
  public:

    using Components = std::variant<
      std::unique_ptr<AnimationComponent>,
      std::unique_ptr<BoneAnimationComponent>,
      std::unique_ptr<MeshComponent>,
      std::unique_ptr<RendererComponent>>;

    ComponentManager() = default;
    ~ComponentManager() = default;

    template <typename T, typename IDType, typename Component>
    void add(IDType entityID, Component component_impl)
    {
      {
        //@todo useful ?
        //std::lock_guard<std::mutex> guard(_mutex);

        auto new_component = std::make_unique<T>();
        new_component->init(std::move(component_impl));
        new_component->setOwner(entityID);

        _component_type_map[&typeid(T)].emplace_back(std::move(new_component));
        _components_entity_map[entityID].emplace_back(&typeid(T));
      }
    }

    template <typename T>
    T* get(IDType entity_ID) {
      {
        std::lock_guard<std::mutex> guard(_mutex);
        auto it = std::ranges::find_if(_component_type_map[&typeid(T)], [&entity_ID](auto& component) {
          if (auto ptr = std::get_if<std::unique_ptr<T>>(&component)) {
            if (ptr) {
              return (*ptr)->getOwner() == entity_ID;
            }
          }
          return false;
          });

        if (it != _component_type_map[&typeid(T)].end()) {
          if (auto ptr = std::get_if<std::unique_ptr<T>>(&*it)) {
            return ptr->get();
          }
        }
        return nullptr;
      }
    }

    std::vector<const std::type_info*> getEntityComponents(IDType entity_id) { return _components_entity_map[entity_id]; }
    void clear();

  private:
    std::unordered_map<const std::type_info*, std::vector<Components>> _component_type_map;
    std::unordered_map<IDType, std::vector<const std::type_info*>> _components_entity_map;
    std::mutex _mutex;
  };
}
