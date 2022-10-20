#pragma once

#include <json.hpp>
#include "Rebulk/Renderer/Adapter/IRendererAdapter.h"
#include "Rebulk/Core/TinyObjLoader.h"
#include "Rebulk/Component/Camera.h"
#include "Rebulk/Component/Mesh.h"
#include "Rebulk/Component/Entity.h"

namespace Rbk
{
    class IEntityManager
    {
    public:
        IEntityManager() = default;
        ~IEntityManager() = default;

        virtual void AddRenderer(std::shared_ptr<IRendererAdapter> renderer) = 0;
        virtual std::vector<std::future<void>> Load(nlohmann::json levelConfig) = 0;
        virtual inline std::vector<std::shared_ptr<Entity>>* GetEntities() = 0;
        virtual inline void SetSkyboxMesh(std::shared_ptr<Mesh> skyboxMesh) = 0;
        virtual inline std::vector<std::shared_ptr<Entity>>* GetBBox() = 0;
        virtual inline std::shared_ptr<Mesh> GetSkyboxMesh() = 0;
        virtual void Clear() = 0;
        virtual bool ShowBBox() const = 0;
        virtual inline std::map<std::string, std::array<uint32_t, 2>> GetLoadedBBox() = 0;
        virtual const uint32_t GetInstancedCount() = 0;
        virtual void SetShowBBox(bool show) = 0;
        virtual void AddBBox(const std::shared_ptr<Entity>& bbox) = 0;
        virtual std::map<std::string, std::array<uint32_t, 2>> GetLoadedEntities() = 0;
    };
}
