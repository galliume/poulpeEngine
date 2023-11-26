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

        Window* getWindow() override { return m_Window.get(); }

        Camera* getCamera() override { return m_Camera.get(); }
        VulkanAdapter* getRendererAdapter() override { return m_Renderer.get(); }
        TextureManager* getTextureManager() override { return m_TextureManager.get(); }
        SpriteAnimationManager* getSpriteAnimationManager() override { return m_SpriteAnimationManager.get(); }
        EntityManager* getEntityManager() override { return m_EntityManager.get(); }
        ShaderManager* getShaderManager() override { return m_ShaderManager.get(); }
        AudioManager* getAudioManager() override { return m_AudioManager.get(); }
        ConfigManager* getConfigManager() override { return m_ConfigManager.get(); }
        DestroyManager* getDestroyManager() override { return m_DestroyManager.get(); }
        ComponentManager* getComponentManager() override { return m_ComponentManager.get(); }


        void cleanUp() override;
        void forceRefresh() { m_Refresh = true; };
        
        uint32_t getAppHeight() { return getRendererAdapter()->rdr()->getSwapChainExtent().height; };
        uint32_t getAppWidth() { return getRendererAdapter()->rdr()->getSwapChainExtent().width; };

        void setDrawBbox(bool draw) { m_Renderer->setDrawBbox(draw); };

    private:
        void loadData(std::string const & level);
        void prepareEntity();
        void prepareHUD();
        void prepareSkybox();
        void prepareSplashScreen();

    private:
        std::unique_ptr<Poulpe::Camera> m_Camera;
        std::unique_ptr<VulkanAdapter> m_Renderer;
        std::unique_ptr<Window> m_Window;

        std::unique_ptr<Poulpe::AudioManager> m_AudioManager;
        std::unique_ptr<Poulpe::ComponentManager> m_ComponentManager;
        std::unique_ptr<Poulpe::ConfigManager> m_ConfigManager;
        std::unique_ptr<Poulpe::DestroyManager> m_DestroyManager;
        std::unique_ptr<Poulpe::EntityManager> m_EntityManager;
        std::unique_ptr<Poulpe::ShaderManager> m_ShaderManager;
        std::unique_ptr<Poulpe::SpriteAnimationManager> m_SpriteAnimationManager;
        std::unique_ptr<Poulpe::TextureManager> m_TextureManager;

        bool m_IsLoaded{ false };
        bool m_Refresh{ false };
        bool m_ShowBbox{ false };

        std::string m_CurrentLevel;
        std::string m_CurrentSkybox;

        //@todo should not be impl
        std::vector<VkDescriptorPool> m_DescriptorPools;
    };
}
