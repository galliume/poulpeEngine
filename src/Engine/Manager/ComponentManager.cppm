export module Engine.Managers.ComponentManager;

import std;

import Engine.Animation.AnimationScript;
import Engine.Animation.BoneAnimationScript;

import Engine.Component.Components;
import Engine.Component.Mesh;
import Engine.Renderer.RendererComponent;

import Engine.Core.Logger;

import Engine.Utils.IDHelper;

namespace Poulpe
{
  export class ComponentManager
  {
    public:
      template <typename T>
      void registerComponent()
      {
        auto const type_id { TypeIdGenerator<T>::GetID() };
        _component_pools[type_id] = std::make_unique<ComponentPool<T>>();
      }

      template <typename T, typename... Args>
      void add(IDType id, Args&&... args)
      {
        auto* pool { getPool<T>() };
        pool->set(id, std::forward<Args>(args)...);
      }

      template <typename T>
      T* get(IDType id)
      {
        auto* pool { getPool<T>() };
        if (pool == nullptr) {
          return nullptr;
        }

        return pool->get(id);
      }

      void clear();

    private:
        struct IPool
        {
          virtual ~IPool();
        };

        template <typename T>
        class ComponentPool : public IPool
        {
          std::vector<T> dense_data;
          std::flat_map<IDType, std::size_t> entity_to_index;

        public:
          template<typename... Args>
          void set(IDType id, Args&&... args)
          {
              auto const it { entity_to_index.find(id) };
              if (it != entity_to_index.end()) {
                dense_data[it->second] = std::move(T(std::forward<Args>(args)...));
                return;
              }

              entity_to_index[id] = dense_data.size();
              dense_data.emplace_back(std::forward<Args>(args)...);
          }

          T* get(IDType id)
          {
            auto it { entity_to_index.find(id) };
            if (it == entity_to_index.end()) return nullptr;
            return &dense_data[it->second];
          }
        };

        template <typename T>
        ComponentPool<T>* getPool()
        {
          auto const type_id { TypeIdGenerator<T>::GetID() };
          std::cout << "type_id " << std::to_string(type_id) << std::endl;
          Logger::debug("type id {}", type_id);
          auto const it { _component_pools.find(type_id) };
          if (it == _component_pools.end()) {
            return nullptr;
          }

          return static_cast<ComponentPool<T>*>(it->second.get());
        }

        std::unordered_map<IDType, std::unique_ptr<IPool>> _component_pools;
    };

    Poulpe::ComponentManager::IPool::~IPool() = default;
}
