#pragma once

#include "IEntityManager.hpp"

namespace Poulpe
{
    class EntityManager : IEntityManager
    {
    public:

        explicit EntityManager();
        virtual ~EntityManager() = default;

        void addEntity(Entity* entity);
        void addRenderer(IRendererAdapter* renderer) override { m_Renderer = renderer; }
        void addHUD(std::vector<Mesh*> hud) override { m_HUD = std::move(hud); }
        void clear() override;

        inline std::vector<std::unique_ptr<Entity>>* getEntities() override { return & m_Entities; }
        inline Mesh* getSkybox() override { return m_Skybox.get(); }
        inline uint32_t getTotalEntities() const { return m_Entities.size(); }
        inline std::unordered_map<std::string, std::array<uint32_t, 2>> getLoadedEntities() override { return m_LoadedEntities; }
        inline void setSkybox(Mesh* skybox) override { m_Skybox = std::unique_ptr<Mesh>(skybox); }
        inline bool IsLoadingDone() { return m_LoadingDone.load(); }

        uint32_t getInstancedCount() override;
        Entity* getEntityByName(std::string const & name);
        std::vector<Mesh*> getHUD() override { return m_HUD; }
        std::function<void()> load(nlohmann::json levelConfig, std::condition_variable & cv) override;

        EntityNode const * getWorldNode() const { return m_WorldNode.get(); }

    private:
        std::vector<std::unique_ptr<Entity>> m_Entities;
        std::unordered_map<std::string, std::array<uint32_t, 2>> m_LoadedEntities;
        std::unique_ptr<Mesh> m_Skybox = nullptr;
        std::vector<Mesh*> m_HUD = {};
        IRendererAdapter* m_Renderer = nullptr;
        nlohmann::json m_LevelConfig;
        std::atomic_bool m_LoadingDone{ false };

        std::unique_ptr<WorldEntity> m_World;
        std::unique_ptr<EntityNode> m_WorldNode;
    };
}
