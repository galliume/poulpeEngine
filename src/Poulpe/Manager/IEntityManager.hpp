#pragma once

#include "Poulpe/Component/Camera.hpp"
#include "Poulpe/Component/Entity.hpp"
#include "Poulpe/Component/EntityNode.hpp"

#include "Poulpe/Core/TinyObjLoader.hpp"

#include "Poulpe/Renderer/Adapter/IRendererAdapter.hpp"

#include <nlohmann/json.hpp>
#include <queue>

namespace Poulpe
{
    class IEntityManager
    {
    public:
        IEntityManager() = default;
        ~IEntityManager() = default;

        virtual void addRenderer(IRendererAdapter* renderer) = 0;
        virtual std::function<void()> load(nlohmann::json levelConfig) = 0;
        virtual inline std::vector<std::unique_ptr<Entity>>* getEntities() = 0;
        virtual inline void setSkybox(Entity* skybox) = 0;
        virtual inline Entity* getSkybox() = 0;
        virtual void clear() = 0;
        virtual uint32_t getInstancedCount() = 0;
        virtual std::unordered_map<std::string, std::array<uint32_t, 2>> getLoadedEntities() = 0;
        virtual void addHUD(Entity* entity) = 0;
        virtual std::vector<std::unique_ptr<Entity>>* getHUD() = 0;
    };
}
