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

        virtual void init() = 0;
        virtual void renderScene() = 0;
        virtual void draw() = 0;
        virtual bool isLoaded() = 0;
        virtual void setIsLoaded(bool loaded = true) = 0;
        virtual std::shared_ptr<Window> getWindow() = 0;
        virtual std::shared_ptr<Poulpe::Camera> getCamera() = 0;
        virtual std::shared_ptr<Poulpe::InputManager> getInputManager() = 0;
        virtual std::shared_ptr<Poulpe::VulkanAdapter> getRendererAdapter() = 0;
        virtual std::shared_ptr<Poulpe::TextureManager> getTextureManager() = 0;
        virtual std::shared_ptr<Poulpe::SpriteAnimationManager> getSpriteAnimationManager() = 0;
        virtual std::shared_ptr<Poulpe::EntityManager> getEntityManager() = 0;
        virtual std::shared_ptr<Poulpe::ShaderManager> getShaderManager() = 0;
        virtual std::shared_ptr<Poulpe::AudioManager> getAudioManager() = 0;
        virtual std::shared_ptr<Poulpe::ConfigManager> getConfigManager() = 0;
        virtual std::shared_ptr<Poulpe::DestroyManager> getDestroyManager() = 0;
        virtual void cleanUp() = 0;

        virtual void refresh(uint32_t levelIndex, bool showBbox = false, std::string_view skybox = "debug") = 0;
    };
}
