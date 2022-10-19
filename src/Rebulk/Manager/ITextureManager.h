#pragma once
#include "json.hpp"
#include "Rebulk/Renderer/Adapter/IRendererAdapter.h"

namespace Rbk
{
    class ITextureManager
    {
    public:
        ITextureManager() = default;
        ~ITextureManager() = default;

        virtual inline std::map<std::string, Texture> GetTextures() const = 0;
        virtual void AddConfig(nlohmann::json config) = 0;
        virtual void AddRenderer(std::shared_ptr<IRendererAdapter> renderer) = 0;
        virtual std::vector<std::future<void>> Load(const std::string& skybox) = 0;
        virtual inline const Texture GetSkyboxTexture() const = 0;
        virtual void AddSkyBox(const std::vector<std::string>& skyboxImages) = 0;
        virtual void AddTexture(const std::string& name, const std::string& path, bool isPublic = false) = 0;
        virtual inline std::map<std::string, std::string> GetPaths() const = 0;
    };
}