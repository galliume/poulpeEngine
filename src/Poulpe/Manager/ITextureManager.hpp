#pragma once
#include <nlohmann/json.hpp>
#include "Poulpe/Renderer/Adapter/IRendererAdapter.hpp"

#include "Poulpe/Component/Texture.hpp"

namespace Poulpe
{
    class ITextureManager
    {
    public:
        ITextureManager() = default;
        ~ITextureManager() = default;

        virtual inline std::unordered_map<std::string, Texture> GetTextures() = 0;
        virtual void AddConfig(nlohmann::json config) = 0;
        virtual void AddRenderer(std::shared_ptr<IRendererAdapter> renderer) = 0;
        virtual std::function<void()> Load(std::condition_variable& cv) = 0;
        virtual std::function<void()> LoadSkybox(const std::string& skybox, std::condition_variable& cv) = 0;
        virtual inline const Texture GetSkyboxTexture() const = 0;
        virtual void AddSkyBox(const std::vector<std::string>& skyboxImages) = 0;
        virtual void AddTexture(const std::string& name, const std::string& path, bool isPublic = false) = 0;
        virtual inline std::unordered_map<std::string, std::string> GetPaths() const = 0;
    };
}
