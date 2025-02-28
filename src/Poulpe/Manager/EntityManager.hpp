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
    inline Entity* getTerrain() { return _terrain.get(); }
    inline Entity* getWater() { return _water.get(); }
    inline Entity* getText(unsigned int const index) { return _texts.at(index).get(); }
    inline std::vector<std::unique_ptr<Entity>>& getTexts() { return _texts; }

    std::function<void()> load(nlohmann::json const& lvl_config);
    inline void setSkybox(std::unique_ptr<Entity> skybox) { _skybox = std::move(skybox); }
    inline void setTerrain(std::unique_ptr<Entity> terrain) { _terrain = std::move(terrain); }
    inline void setWater(std::unique_ptr<Entity> water) { _water = std::move(water); }
    inline int addText(std::unique_ptr<Entity> text) { _texts.emplace_back(std::move(text)); return _texts.size() - 1; }
    
    
    //void addEntity(Mesh* meshes);
    //inline size_t getTotalEntities() const { return _Entities.size(); }
    EntityNode * getWorldNode();
    void initMeshes(std::string const& name, nlohmann::json const& data);
    void initWorldGraph();
    std::shared_mutex& lockWorldNode() { return _mutex_shared; };

  private:
    ComponentManager* _component_manager;
    LightManager* _light_manager;
    TextureManager* _texture_manager;

    //std::vector<std::unique_ptr<Entity>> _Entities{};
    std::vector<std::unique_ptr<Entity>> _HUD{};

    nlohmann::json _lvl_config;

    Renderer* _renderer{nullptr};

    std::unique_ptr<Entity> _skybox{nullptr};
    std::unique_ptr<Entity> _terrain{nullptr};
    std::unique_ptr<Entity> _water{nullptr};
    std::vector<std::unique_ptr<Entity>> _texts;

    std::unique_ptr<EntityNode> _world_node;

    mutable std::shared_mutex _mutex_shared;
  };
}
