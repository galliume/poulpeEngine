#pragma once

#include "Poulpe/Component/EntityNode.hpp"

#include "Poulpe/Manager/LightManager.hpp"
#include "Poulpe/Manager/TextureManager.hpp"

#include <nlohmann/json.hpp>

#include <set>
#include <shared_mutex>

namespace Poulpe
{
  class ComponentManager;
  class Entity;
  class Renderer;

  class EntityManager
  {
  public:

    explicit EntityManager(
      ComponentManager* const component_manager,
      LightManager* const light_manager,
      TextureManager* const texture_manager);

    void inline addHUD(std::unique_ptr<Entity> entity) { _HUD.emplace_back(std::move(entity)); }
    void inline addRenderer(Renderer* const renderer) { _renderer = renderer; }
    void clear();
    //inline std::vector<std::unique_ptr<Entity>>* getEntities() { return & _Entities; }
    inline std::vector<std::unique_ptr<Entity>>* getHUD() { return & _HUD; }
    //inline size_t getInstancedCount() const { return _Entities.size(); }
    //inline std::unordered_map<std::string, std::array<uint32_t, 2>> getLoadedEntities() { return _LoadedEntities; }
    inline Entity* getSkybox() { return _skybox.get(); }
    std::function<void()> load(nlohmann::json const& lvl_config);
    inline void setSkybox(std::unique_ptr<Entity> skybox) { _skybox = std::move(skybox); }
    //void addEntity(Mesh* meshes);
    //inline size_t getTotalEntities() const { return _Entities.size(); }
    EntityNode * getWorldNode();
    void initMeshes(std::string const& name, nlohmann::json const& data);
    void initWorldGraph();
    std::mutex& lockWorldNode() { return _mutex_world_node; };

  private:
    ComponentManager* _component_manager;
    LightManager* _light_manager;
    TextureManager* _texture_manager;
        
    //std::vector<std::unique_ptr<Entity>> _Entities{};
    std::vector<std::unique_ptr<Entity>> _HUD{};
        
    nlohmann::json _lvl_config;
        
    Renderer* _renderer{nullptr};
        
    std::unique_ptr<Entity> _skybox{nullptr};
    std::unique_ptr<EntityNode> _world_node;

    std::mutex _mutex_world_node;
    mutable std::shared_mutex _mutex_shared;

  };
}
