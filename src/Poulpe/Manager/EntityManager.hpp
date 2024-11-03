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

    explicit EntityManager(ComponentManager* const componentManager,
      LightManager* const lightManager,
      TextureManager* const textureManager);
    ~EntityManager() = default;

    void inline addHUD(std::unique_ptr<Entity> entity) { m_HUD.emplace_back(std::move(entity)); }
    void inline addRenderer(Renderer* const renderer) { m_Renderer = renderer; }
    void clear();
    //inline std::vector<std::unique_ptr<Entity>>* getEntities() { return & m_Entities; }
    inline std::vector<std::unique_ptr<Entity>>* getHUD() { return & m_HUD; }
    //inline size_t getInstancedCount() const { return m_Entities.size(); }
    //inline std::unordered_map<std::string, std::array<uint32_t, 2>> getLoadedEntities() { return m_LoadedEntities; }
    inline Entity* getSkybox() { return m_Skybox.get(); }
    std::function<void()> load(nlohmann::json const& levelConfig);
    inline void setSkybox(std::unique_ptr<Entity> skybox) { m_Skybox = std::move(skybox); }
    //void addEntity(Mesh* meshes);
    //inline size_t getTotalEntities() const { return m_Entities.size(); }
    EntityNode * getWorldNode();
    inline bool isLoadingDone() const { return m_LoadingDone.load(); }
    void initMeshes(std::string const& name, nlohmann::json const data);
    void initWorldGraph();

  private:
    ComponentManager* m_ComponentManager;
    LightManager* m_LightManager;
    TextureManager* m_TextureManager;
        
    //std::vector<std::unique_ptr<Entity>> m_Entities{};
    std::vector<std::unique_ptr<Entity>> m_HUD{};
        
    nlohmann::json m_LevelConfig;
    //std::unordered_map<std::string, std::array<uint32_t, 2>> m_LoadedEntities{};
    std::atomic_bool m_LoadingDone{ false };
        
    Renderer* m_Renderer{nullptr};
        
    std::set<std::string> m_ObjLoaded{};

    std::unique_ptr<Entity> m_Skybox{nullptr};
    std::unique_ptr<EntityNode> m_WorldNode;

    std::mutex m_MutexWorldNode;
    mutable std::shared_mutex m_SharedMutex;

  };
}
