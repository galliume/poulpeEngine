#pragma once

#include <nlohmann/json.hpp>
#include <queue>
#include "Poulpe/Renderer/Adapter/IRendererAdapter.hpp"
#include "Poulpe/Core/TinyObjLoader.hpp"
#include "Poulpe/Component/Camera.hpp"
#include "Poulpe/Component/Mesh.hpp"
#include "Poulpe/Component/Entity.hpp"

namespace Poulpe
{
    class IEntityManager
    {
    public:
        IEntityManager() = default;
        ~IEntityManager() = default;

        virtual void addRenderer(std::shared_ptr<IRendererAdapter> renderer) = 0;
        virtual std::function<void()> load(nlohmann::json levelConfig, std::condition_variable& cv) = 0;
        virtual inline std::vector<std::shared_ptr<Entity>>* getEntities() = 0;
        virtual inline void setSkybox(std::shared_ptr<Mesh> skybox) = 0;
        virtual inline std::shared_ptr<Mesh> getSkybox() = 0;
        virtual void clear() = 0;
        virtual uint32_t getInstancedCount() = 0;
        virtual std::unordered_map<std::string, std::array<uint32_t, 2>> getLoadedEntities() = 0;
        virtual void addHUD(std::vector<std::shared_ptr<Mesh>> hud) = 0;
        virtual std::vector<std::shared_ptr<Mesh>> getHUD() = 0;
    };
}
