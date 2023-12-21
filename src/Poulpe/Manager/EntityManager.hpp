#pragma once

#include "IEntityManager.hpp"

#include <set>

namespace Poulpe
{
    class EntityManager : IEntityManager
    {
    public:

        explicit EntityManager(ComponentManager* componentManager);
        virtual ~EntityManager() = default;

        void addEntity(std::vector<Mesh*> meshes);

        std::vector<Mesh*> initMeshes(std::string const & name, std::string const & path,
            std::vector<std::string> const & textureNames, std::string const & shader,
            glm::vec3 const & pos, glm::vec3 const & scale, glm::vec3 rotation,
            bool shouldInverseTextureY);

        void addRenderer(IRendererAdapter* renderer) override { m_Renderer = renderer; }
        void addHUD(Entity* entity) override { m_HUD.emplace_back(std::unique_ptr<Entity>(entity)); }
        void clear() override;

        inline std::vector<std::unique_ptr<Entity>>* getEntities() override { return & m_Entities; }
        inline Entity* getSkybox() override { return m_Skybox.get(); }
        inline uint32_t getTotalEntities() const { return m_Entities.size(); }
        inline std::unordered_map<std::string, std::array<uint32_t, 2>> getLoadedEntities() override { return m_LoadedEntities; }
        inline void setSkybox(Entity* skybox) override { m_Skybox = std::unique_ptr<Entity>(skybox); }
        inline bool IsLoadingDone() { return m_LoadingDone.load(); }

        uint32_t getInstancedCount() override;
        Entity* getEntityByName(std::string const & name);
        std::vector<std::unique_ptr<Entity>>* getHUD() override { return & m_HUD; }
        std::function<void()> load(nlohmann::json levelConfig) override;

        EntityNode const * getWorldNode() const { return m_WorldNode.get(); }
        void initWorldGraph();

    private:
        std::vector<std::unique_ptr<Entity>> m_Entities;
        std::unordered_map<std::string, std::array<uint32_t, 2>> m_LoadedEntities;
        std::unique_ptr<Entity> m_Skybox = nullptr;
        std::vector<std::unique_ptr<Entity>> m_HUD = {};
        IRendererAdapter* m_Renderer = nullptr;
        nlohmann::json m_LevelConfig;
        std::atomic_bool m_LoadingDone{ false };

        std::unique_ptr<Entity> m_World;
        std::unique_ptr<EntityNode> m_WorldNode;

        std::set<std::string> m_ObjLoaded{};

        ComponentManager* m_ComponentManager;
    };
}
