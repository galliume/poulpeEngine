#pragma once

#include "IEntityManager.hpp"

namespace Rbk
{
    class EntityManager : IEntityManager
    {
    public:

        explicit EntityManager();

        virtual void AddRenderer(std::shared_ptr<IRendererAdapter> renderer) override { m_Renderer = renderer; }
        virtual std::vector<std::function<void()>> Load(nlohmann::json levelConfig) override;
        virtual inline std::vector<std::shared_ptr<Entity>>* GetEntities() override { return &m_Entities; }
        virtual inline void SetSkybox(std::shared_ptr<Mesh> skybox) override { m_Skybox = skybox; }
        virtual inline std::shared_ptr<Mesh> GetSkybox() override { return m_Skybox; }
        virtual void Clear() override;
        virtual const uint32_t GetInstancedCount() override;
        virtual void AddHUD(std::vector<std::shared_ptr<Mesh>> hud) override { m_HUD = hud; }
        virtual std::vector<std::shared_ptr<Mesh>> GetHUD() override { return m_HUD; }
        virtual inline std::map<std::string, std::array<uint32_t, 2>> GetLoadedEntities() override { return m_LoadedEntities; }

        void AddEntity(const std::shared_ptr<Entity>& entity);
        const std::shared_ptr<Entity> GetEntityByName(const std::string& name) const;

        inline const uint32_t GetTotalEntities() const { return m_Entities.size(); }
        bool IsLoadingQueuesEmpty() { return m_LoadingQueues.empty(); };

    private:
        std::vector<std::shared_ptr<Entity>> m_Entities;
        std::map<std::string, std::array<uint32_t, 2>> m_LoadedEntities;
        std::shared_ptr<Mesh> m_Skybox = nullptr;
        std::vector<std::shared_ptr<Mesh>> m_HUD = {};
        std::shared_ptr<IRendererAdapter> m_Renderer = nullptr;
        nlohmann::json m_LevelConfig;
        std::queue<int> m_LoadingQueues;
        int m_QueueIndex = 0;
    };
}
