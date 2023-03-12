#pragma once

#include <nlohmann/json.hpp>
#include <queue>
#include "Rebulk/Renderer/Adapter/IRendererAdapter.hpp"
#include "Rebulk/Core/TinyObjLoader.hpp"
#include "Rebulk/Component/Camera.hpp"
#include "Rebulk/Component/Mesh.hpp"
#include "Rebulk/Component/Entity.hpp"

namespace Rbk
{
    class IEntityManager
    {
    public:
        IEntityManager() = default;
        ~IEntityManager() = default;

        virtual void AddRenderer(std::shared_ptr<IRendererAdapter> renderer) = 0;
        virtual std::function<void()> Load(nlohmann::json levelConfig, std::condition_variable& cv) = 0;
        virtual inline std::vector<std::shared_ptr<Entity>>* GetEntities() = 0;
        virtual inline void SetSkybox(std::shared_ptr<Mesh> skybox) = 0;
        virtual inline std::shared_ptr<Mesh> GetSkybox() = 0;
        virtual void Clear() = 0;
        virtual const uint32_t GetInstancedCount() = 0;
        virtual std::unordered_map<std::string, std::array<uint32_t, 2>> GetLoadedEntities() = 0;
        virtual void AddHUD(std::vector<std::shared_ptr<Mesh>> hud) = 0;
        virtual std::vector<std::shared_ptr<Mesh>> GetHUD() = 0;
    };
}
