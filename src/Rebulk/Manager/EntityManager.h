#pragma once

#include "IEntityManager.h"

namespace Rbk
{
    class EntityManager : IEntityManager
    {
    public:

        explicit EntityManager();

        virtual void AddRenderer(std::shared_ptr<IRendererAdapter> renderer) override { m_Renderer = renderer; };
        virtual std::vector<std::future<void>> Load(nlohmann::json levelConfig) override;
        virtual inline std::vector<std::shared_ptr<Entity>>* GetEntities() override { return &m_Entities; };
        virtual inline void SetSkyboxMesh(std::shared_ptr<Mesh> skyboxMesh) override { m_SkyboxMesh = skyboxMesh; };
        virtual inline std::vector<std::shared_ptr<Entity>>* GetBBox() override  { return &m_BoundingBox; };
        virtual inline std::shared_ptr<Mesh> GetSkyboxMesh() override { return m_SkyboxMesh; };
        virtual void Clear() override;
        virtual bool ShowBBox() const override { return m_ShowBBox; };
        virtual inline const std::map<std::string, std::array<uint32_t, 2>> GetLoadedBBox() const override { return m_LoadedBbox; };
        virtual const uint32_t GetInstancedCount() override;
        virtual void SetShowBBox(bool show) override;

        void AddEntity(const std::shared_ptr<Entity>& entity);
        void AddBBox(const std::shared_ptr<Entity>& bbox);
        const std::shared_ptr<Entity> GetEntityByName(const std::string& name) const;

        inline const std::map<std::string, std::array<uint32_t, 2>> GetLoadedEntities() const { return m_LoadedEntities; };
        inline const uint32_t GetTotalEntities() const { return m_Entities.size(); };


    private:
        std::vector<std::shared_ptr<Entity>> m_Entities;
        std::vector<std::shared_ptr<Entity>> m_BoundingBox;
        std::map<std::string, std::array<uint32_t, 2>> m_LoadedEntities;
        std::map<std::string, std::array<uint32_t, 2>> m_LoadedBbox;
        std::shared_ptr<Mesh> m_SkyboxMesh = nullptr;
        std::shared_ptr<IRendererAdapter> m_Renderer = nullptr;
        bool m_ShowBBox = false;
    };
}