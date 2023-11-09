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

        virtual inline std::unordered_map<std::string, Texture> getTextures() = 0;
        virtual void addConfig(nlohmann::json config) = 0;
        virtual void addRenderer(std::shared_ptr<IRendererAdapter> renderer) = 0;
        virtual std::function<void()> load(std::condition_variable& cv) = 0;
        virtual std::function<void()> loadSkybox(std::string_view, std::condition_variable& cv) = 0;
        virtual inline const Texture getSkyboxTexture() const = 0;
        virtual void addSkyBox(const std::vector<std::string>& skyboxImages) = 0;
        virtual void addTexture(const std::string& name, const std::string& path, bool isPublic = false) = 0;
        virtual inline std::unordered_map<std::string, std::string> getPaths() const = 0;
    };
}
