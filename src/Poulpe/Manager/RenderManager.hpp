#pragma once

#include "IRenderManager.hpp"

#include "Poulpe/Core/ThreadPool.hpp"

namespace Poulpe
{
    class RenderManager : IRenderManager
    {
    public:
        RenderManager(Window* window);
        virtual ~RenderManager() = default;

        void init() override;
        void renderScene() override;
        void draw() override;
        bool isLoaded()  override { return m_IsLoaded; }
        void setIsLoaded(bool loaded = true) override { m_IsLoaded = loaded; }
        void refresh(uint32_t levelIndex, bool showBbox = false, std::string_view skybox = "debug") override;

        Window* getWindow() override { return m_Window; }

        Camera* getCamera() override { return m_Camera.get(); }
        InputManager* getInputManager() override { return m_InputManager.get(); }
        VulkanAdapter* getRendererAdapter() override { return m_Renderer.get(); }
        TextureManager* getTextureManager() override { return m_TextureManager.get(); }
        SpriteAnimationManager* getSpriteAnimationManager() override { return m_SpriteAnimationManager.get(); }
        EntityManager* getEntityManager() override { return m_EntityManager.get(); }
        ShaderManager* getShaderManager() override { return m_ShaderManager.get(); }
        AudioManager* getAudioManager() override { return m_AudioManager.get(); }
        ConfigManager* getConfigManager() override { return m_ConfigManager.get(); }
        DestroyManager* getDestroyManager() override { return m_DestroyManager.get(); }


        void cleanUp() override;
        void forceRefresh() { m_Refresh = true; };
        
        uint32_t getAppHeight() { return getRendererAdapter()->rdr()->getSwapChainExtent().height; };
        uint32_t getAppWidth() { return getRendererAdapter()->rdr()->getSwapChainExtent().width; };

        void setDrawBbox(bool draw) { m_Renderer->setDrawBbox(draw); };

    private:
        void prepareSplashScreen();
        void prepareEntity();
        void prepareHUD();
        void prepareSkybox();
        void loadData(std::string const & level);

    private:
        Window* m_Window;

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

        bool m_IsLoaded{ false };
        bool m_Refresh{ false };
        bool m_ShowBbox{ false };

        std::string m_CurrentLevel;
        std::string m_CurrentSkybox;

        //@todo should not be impl
        std::vector<VkDescriptorPool> m_DescriptorPools;
    };
}
