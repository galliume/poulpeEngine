#pragma once

#include "AudioManager.hpp"
#include "ComponentManager.hpp"
#include "ConfigManager.hpp"
#include "DestroyManager.hpp"
#include "EntityManager.hpp"
#include "ShaderManager.hpp"
#include "SpriteAnimationManager.hpp"
#include "TextureManager.hpp"
#include "LightManager.hpp"

#include "Poulpe/Component/Camera.hpp"
#include "Poulpe/Component/Mesh.hpp"

#include "Poulpe/GUI/Window.hpp"

#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    class IRenderManager
    {
    public:
        IRenderManager() = default;
        ~IRenderManager() = default;

        virtual void init() = 0;
        virtual void renderScene(float const deltaTime) = 0;
        virtual void draw() = 0;
        virtual bool isLoaded() = 0;
        virtual void setIsLoaded(bool loaded = true) = 0;
        
        virtual Window* getWindow() = 0;
        virtual Camera* getCamera() = 0;
        virtual VulkanAdapter* getRendererAdapter() = 0;
        virtual TextureManager* getTextureManager() = 0;
        virtual SpriteAnimationManager* getSpriteAnimationManager() = 0;
        virtual EntityManager* getEntityManager() = 0;
        virtual ShaderManager* getShaderManager() = 0;
        virtual AudioManager* getAudioManager() = 0;
        virtual ConfigManager* getConfigManager() = 0;
        virtual DestroyManager* getDestroyManager() = 0;
        virtual ComponentManager* getComponentManager() = 0;
        virtual LightManager* getLightManager() = 0;

        virtual void cleanUp() = 0;
        virtual void refresh(uint32_t levelIndex, bool showBbox = false, std::string_view skybox = "debug") = 0;
    };
}
