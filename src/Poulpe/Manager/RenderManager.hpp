#pragma once

#include "IRenderManager.hpp"

#include "Poulpe/Core/ThreadPool.hpp"

namespace Poulpe
{
    class RenderManager : IRenderManager
    {
    public:
        RenderManager(Window* window);

        void cleanUp() override;
        void draw() override;
        inline void forceRefresh() { m_Refresh = true; };
        inline uint32_t getAppHeight() { return getRendererAdapter()->rdr()->getSwapChainExtent().height; };
        inline uint32_t getAppWidth() { return getRendererAdapter()->rdr()->getSwapChainExtent().width; };
        inline AudioManager* getAudioManager() override { return m_AudioManager.get(); }
        inline Camera* getCamera() override { return m_Camera.get(); }
        inline ComponentManager* getComponentManager() override { return m_ComponentManager.get(); }
        inline ConfigManager* getConfigManager() override { return m_ConfigManager.get(); }
        inline DestroyManager* getDestroyManager() override { return m_DestroyManager.get(); }
        inline EntityManager* getEntityManager() override { return m_EntityManager.get(); }
        inline LightManager* getLightManager() override { return m_LightManager.get(); }
        inline VulkanAdapter* getRendererAdapter() override { return m_Renderer.get(); }
        inline ShaderManager* getShaderManager() override { return m_ShaderManager.get(); }
        inline TextureManager* getTextureManager() override { return m_TextureManager.get(); }
        inline Window* getWindow() override { return m_Window.get(); }
        void init() override;
        inline bool isLoaded()  override { return m_IsLoaded; }
        inline void setDrawBbox(bool draw) { m_Renderer->setDrawBbox(draw); };
        inline void setIsLoaded(bool loaded = true) override { m_IsLoaded = loaded; }
        void refresh(uint32_t levelIndex, bool showBbox = false, std::string_view skybox = "debug") override;
        void renderScene(float const deltaTime) override;

    private:
        void loadData(std::string const & level);

        //@todo move to EntityManager
        void prepareHUD();
        void prepareSkybox();

    private:
        std::string m_CurrentLevel;
        std::string m_CurrentSkybox;

        bool m_IsLoaded{ false };
        bool m_Refresh{ false };
        bool m_ShowBbox{ false };

        std::unique_ptr<Poulpe::Camera> m_Camera;
        std::unique_ptr<VulkanAdapter> m_Renderer;
        std::unique_ptr<Window> m_Window;

        std::unique_ptr<Poulpe::AudioManager> m_AudioManager;
        std::unique_ptr<Poulpe::ComponentManager> m_ComponentManager;
        std::unique_ptr<Poulpe::ConfigManager> m_ConfigManager;
        std::unique_ptr<Poulpe::DestroyManager> m_DestroyManager;
        std::unique_ptr<Poulpe::EntityManager> m_EntityManager;
        std::unique_ptr<Poulpe::LightManager> m_LightManager;
        std::unique_ptr<Poulpe::ShaderManager> m_ShaderManager;
        std::unique_ptr<Poulpe::TextureManager> m_TextureManager;

        //@todo should not be impl
        std::vector<VkDescriptorPool> m_DescriptorPools;
    };
}
