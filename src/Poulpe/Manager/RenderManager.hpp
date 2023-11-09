#pragma once

#include "IRenderManager.hpp"

#include "Poulpe/Core/ThreadPool.hpp"

namespace Poulpe
{
    class RenderManager : IRenderManager
    {
    public:
        RenderManager(
            std::shared_ptr<Window> window,
            std::shared_ptr<VulkanAdapter> renderer,
            std::shared_ptr<ConfigManager> configManager,
            std::shared_ptr<InputManager> inputManager,
            std::shared_ptr<AudioManager> audioManager,
            std::shared_ptr<TextureManager> textureManager,
            std::shared_ptr<EntityManager> entityManager,
            std::shared_ptr<ShaderManager> shaderManager,
            std::shared_ptr<SpriteAnimationManager> spriteAnimationManager,
            std::shared_ptr<Poulpe::DestroyManager> destroyManager,
            std::shared_ptr<Poulpe::Camera> camera,
            std::shared_ptr<Poulpe::CommandQueue> cmdQueue
        );
        virtual ~RenderManager() = default;

        virtual void init() override;
        virtual void renderScene() override;
        virtual void draw() override;
        virtual bool isLoaded()  override { return m_IsLoaded; }
        virtual void setIsLoaded(bool loaded = true) override { m_IsLoaded = loaded; }
        virtual void refresh(uint32_t levelIndex, bool showBbox = false, std::string_view skybox = "debug") override;

        virtual std::shared_ptr<Window> getWindow() override { return  m_Window; }
        virtual std::shared_ptr<Poulpe::Camera> getCamera() override { return m_Camera; }
        virtual std::shared_ptr<Poulpe::InputManager> getInputManager() override { return m_InputManager; }
        virtual std::shared_ptr<Poulpe::VulkanAdapter> getRendererAdapter() override { return m_Renderer; }
        virtual std::shared_ptr<Poulpe::TextureManager> getTextureManager() override { return m_TextureManager; }
        virtual std::shared_ptr<Poulpe::SpriteAnimationManager> getSpriteAnimationManager() override { return m_SpriteAnimationManager; }
        virtual std::shared_ptr<Poulpe::EntityManager> getEntityManager() override { return m_EntityManager; }
        virtual std::shared_ptr<Poulpe::ShaderManager> getShaderManager() override { return m_ShaderManager; }
        virtual std::shared_ptr<Poulpe::AudioManager> getAudioManager() override { return m_AudioManager; }
        virtual std::shared_ptr<Poulpe::ConfigManager> getConfigManager() override { return m_ConfigManager; }
        virtual std::shared_ptr<Poulpe::DestroyManager> getDestroyManager() override { return m_DestroyManager; }
        virtual void cleanUp() override;

        void setDrawBbox(bool draw) { m_Renderer->setDrawBbox(draw); };
        void forceRefresh() { m_Refresh = true; };

    private:
        void prepareSplashScreen();
        void prepareEntity();
        void prepareHUD();
        void prepareSkybox();
        void loadData(const std::string& level);

    private:
        std::shared_ptr<Window> m_Window;
        std::shared_ptr<VulkanAdapter> m_Renderer;
        std::shared_ptr<Poulpe::Camera> m_Camera;
        std::shared_ptr<Poulpe::InputManager> m_InputManager;
        std::shared_ptr<Poulpe::TextureManager> m_TextureManager;
        std::shared_ptr<Poulpe::SpriteAnimationManager> m_SpriteAnimationManager;
        std::shared_ptr<Poulpe::EntityManager> m_EntityManager;
        std::shared_ptr<Poulpe::ShaderManager> m_ShaderManager;
        std::shared_ptr<Poulpe::AudioManager> m_AudioManager;
        std::shared_ptr<Poulpe::ConfigManager> m_ConfigManager;
        std::shared_ptr<Poulpe::DestroyManager> m_DestroyManager;
        std::shared_ptr<Poulpe::CommandQueue> m_CommandQueue;

        bool m_IsLoaded = false;
        bool m_Refresh = false;
        bool m_ShowBbox = false;
        std::string m_CurrentLevel;
        std::string m_CurrentSkybox;

        //@todo should not be impl
        std::vector<VkDescriptorPool> m_DescriptorPools;
    };
}
