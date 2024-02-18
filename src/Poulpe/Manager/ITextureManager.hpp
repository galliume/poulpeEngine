#pragma once

#include <nlohmann/json.hpp>

#include "Poulpe/Renderer/IRenderer.hpp"

#include "Poulpe/Component/Texture.hpp"

namespace Poulpe
{
    class ITextureManager
    {
    public:
        ITextureManager() = default;
        virtual ~ITextureManager() = default;

        virtual void addConfig(nlohmann::json config) = 0;
        virtual void addRenderer(IRenderer* const renderer) = 0;
        virtual Texture addNormalMapTexture(std::string const & name) = 0;
        virtual void addSkyBox(std::vector<std::string> const & skyboxImages) = 0;
        virtual void addTexture(std::string const & name, std::string const & path, bool isPublic = false) = 0;
        virtual inline std::unordered_map<std::string, std::string> getPaths() const = 0;
        virtual inline const Texture getSkyboxTexture() const = 0;
        virtual inline std::unordered_map<std::string, Texture> getTextures() = 0;
        virtual std::function<void()> load() = 0;
        virtual std::function<void()> loadSkybox(std::string_view) = 0;
    };
}
