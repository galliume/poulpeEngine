#pragma once

#include "ConfigManager.hpp"
#include "InputManager.hpp"
#include "AudioManager.hpp"
#include "TextureManager.hpp"
#include "EntityManager.hpp"
#include "SpriteAnimationManager.hpp"
#include "ShaderManager.hpp"
#include "DestroyManager.hpp"
#include "Poulpe/Component/Mesh.hpp"
#include "Poulpe/Component/Camera.hpp"
#include "Poulpe/GUI/Window.hpp"
#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"
#include "Poulpe/Core/CommandQueue.hpp"

namespace Poulpe
{
    class IRenderManager
    {
    public:
        IRenderManager() = default;
        ~IRenderManager() = default;

        virtual void Init() = 0;
        virtual void RenderScene() = 0;
        virtual void Draw() = 0;
        virtual bool IsLoaded() = 0;
        virtual void SetIsLoaded(bool loaded = true) = 0;
        virtual std::shared_ptr<Window> GetWindow() = 0;
        virtual std::shared_ptr<Poulpe::Camera> GetCamera() = 0;
        virtual std::shared_ptr<Poulpe::InputManager> GetInputManager() = 0;
        virtual std::shared_ptr<Poulpe::VulkanAdapter> GetRendererAdapter() = 0;
        virtual std::shared_ptr<Poulpe::TextureManager> GetTextureManager() = 0;
        virtual std::shared_ptr<Poulpe::SpriteAnimationManager> GetSpriteAnimationManager() = 0;
        virtual std::shared_ptr<Poulpe::EntityManager> GetEntityManager() = 0;
        virtual std::shared_ptr<Poulpe::ShaderManager> GetShaderManager() = 0;
        virtual std::shared_ptr<Poulpe::AudioManager> GetAudioManager() = 0;
        virtual std::shared_ptr<Poulpe::ConfigManager> GetConfigManager() = 0;
        virtual std::shared_ptr<Poulpe::DestroyManager> GetDestroyManager() = 0;
        virtual void CleanUp() = 0;

        virtual void Refresh(uint32_t levelIndex, bool showBbox = false, std::string_view skybox = "debug") = 0;
    };
}
