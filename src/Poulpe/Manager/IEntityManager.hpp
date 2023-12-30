#pragma once

#include "Poulpe/Component/Camera.hpp"
#include "Poulpe/Component/Entity.hpp"
#include "Poulpe/Component/EntityNode.hpp"

#include "Poulpe/Core/TinyObjLoader.hpp"

#include "Poulpe/Manager/TextureManager.hpp"
#include "Poulpe/Manager/LightManager.hpp"
#include "Poulpe/Manager/ComponentManager.hpp"

#include "Poulpe/Renderer/IRenderer.hpp"

#include <nlohmann/json.hpp>
#include <queue>

namespace Poulpe
{
    class IEntityManager
    {
    public:
        virtual ~IEntityManager() = default;

        virtual void addHUD(Entity* entity) = 0;
        virtual void addRenderer(IRenderer* const renderer) = 0;
        virtual void clear() = 0;
        //virtual inline std::vector<std::unique_ptr<Entity>>* getEntities() = 0;
        virtual std::vector<std::unique_ptr<Entity>>* getHUD() = 0;
        //virtual inline size_t getInstancedCount() const = 0;
        //virtual std::unordered_map<std::string, std::array<uint32_t, 2>> getLoadedEntities() = 0;
        virtual inline Entity* getSkybox() = 0;
        virtual EntityNode const* getWorldNode() = 0;
        virtual std::function<void()> load(nlohmann::json levelConfig) = 0;
        virtual inline void setSkybox(Entity* const skybox) = 0;
    };
}
