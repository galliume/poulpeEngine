#pragma once

#include "IEntityManager.hpp"

#include <set>
#include <shared_mutex>

namespace Poulpe
{
  class EntityManager : public IEntityManager
  {
  public:

    explicit EntityManager(ComponentManager* const componentManager, 
      LightManager* const lightManager,
      TextureManager* const textureManager);
    ~EntityManager() override = default;

    void inline addHUD(Entity* entity) override { m_HUD.emplace_back(std::unique_ptr<Entity>(entity)); }
    void inline addRenderer(IRenderer* const renderer) override { m_Renderer = renderer; }
    void clear() override;
    //inline std::vector<std::unique_ptr<Entity>>* getEntities() override { return & m_Entities; }
    inline std::vector<std::unique_ptr<Entity>>* getHUD() override { return & m_HUD; }
    //inline size_t getInstancedCount() const override { return m_Entities.size(); }
    //inline std::unordered_map<std::string, std::array<uint32_t, 2>> getLoadedEntities() override { return m_LoadedEntities; }
    inline Entity* getSkybox() override { return m_Skybox.get(); }
    void load(nlohmann::json const& levelConfig) override;
    inline void setSkybox(Entity* const skybox) override { m_Skybox = std::unique_ptr<Entity>(skybox); }
    //void addEntity(Mesh* meshes);
    //inline size_t getTotalEntities() const { return m_Entities.size(); }
    EntityNode * getWorldNode() override;
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
        
    IRenderer* m_Renderer{nullptr};
        
    std::set<std::string> m_ObjLoaded{};

    std::unique_ptr<Entity> m_Skybox{nullptr};
    std::unique_ptr<EntityNode> m_WorldNode;

    std::mutex m_MutexWorldNode;
    mutable std::shared_mutex m_SharedMutex;

  };
}
