#pragma once

#include "ConfigManager.hpp"
#include "InputManager.hpp"
#include "AudioManager.hpp"
#include "TextureManager.hpp"
#include "EntityManager.hpp"
#include "SpriteAnimationManager.hpp"
#include "ShaderManager.hpp"
#include "DestroyManager.hpp"
#include "Rebulk/Component/Mesh.hpp"
#include "Rebulk/Component/Camera.hpp"
#include "Rebulk/GUI/Window.hpp"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.hpp"

namespace Rbk
{
    class IRenderManager
    {
    public:
        IRenderManager() = default;
        ~IRenderManager() = default;

        virtual void Init() = 0;
        virtual void Draw() = 0;
        virtual bool IsLoaded() = 0;
        virtual void SetIsLoaded(bool loaded = true) = 0;
        virtual std::shared_ptr<Window> GetWindow() = 0;
        virtual std::shared_ptr<Rbk::Camera> GetCamera() = 0;
        virtual std::shared_ptr<Rbk::InputManager> GetInputManager() = 0;
        virtual std::shared_ptr<Rbk::VulkanAdapter> GetRendererAdapter() = 0;
        virtual std::shared_ptr<Rbk::TextureManager> GetTextureManager() = 0;
        virtual std::shared_ptr<Rbk::SpriteAnimationManager> GetSpriteAnimationManager() = 0;
        virtual std::shared_ptr<Rbk::EntityManager> GetEntityManager() = 0;
        virtual std::shared_ptr<Rbk::ShaderManager> GetShaderManager() = 0;
        virtual std::shared_ptr<Rbk::AudioManager> GetAudioManager() = 0;
        virtual std::shared_ptr<Rbk::ConfigManager> GetConfigManager() = 0;
        virtual std::shared_ptr<Rbk::DestroyManager> GetDestroyManager() = 0;
        virtual void CleanUp() = 0;

        virtual void Refresh(uint32_t levelIndex, bool showBbox = false, std::string_view skybox = "debug") = 0;
    };
}
