#pragma once

#include "IRenderManager.hpp"

#include "Poulpe/Core/ThreadPool.hpp"

namespace Poulpe
{
    //@todo rename to SceneManager?
    class RenderManager : public IRenderManager
    {
    public:
        RenderManager(Window* window);

        void cleanUp() override;
        void draw() override;
        inline void forceRefresh() { m_Refresh = true; }
        inline uint32_t getAppHeight() { return getRenderer()->getSwapChainExtent().height; }
        inline uint32_t getAppWidth() { return getRenderer()->getSwapChainExtent().width; }
        inline AudioManager* getAudioManager() override { return m_AudioManager.get(); }
        inline Camera* getCamera() override { return m_Camera.get(); }
        inline ComponentManager* getComponentManager() override { return m_ComponentManager.get(); }
        inline DestroyManager* getDestroyManager() override { return m_DestroyManager.get(); }
        inline EntityManager* getEntityManager() override { return m_EntityManager.get(); }
        inline LightManager* getLightManager() override { return m_LightManager.get(); }
        inline IRenderer* getRenderer() override { return m_Renderer.get(); }
        inline ShaderManager* getShaderManager() override { return m_ShaderManager.get(); }
        inline TextureManager* getTextureManager() override { return m_TextureManager.get(); }
        inline Window* getWindow() override { return m_Window.get(); }
        void init() override;
        inline bool isLoaded()  override { return m_IsLoaded; }
        void refresh(uint32_t levelIndex, bool showBbox = false, std::string_view skybox = "debug") override;
        void renderScene(std::chrono::duration<float> deltaTime) override;
        inline void setDrawBbox(bool draw) { m_Renderer->setDrawBbox(draw); }
        inline void setIsLoaded(bool loaded = true) override { m_IsLoaded = loaded; }

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

        std::unique_ptr<Camera> m_Camera;
        std::unique_ptr<IRenderer> m_Renderer;
        std::unique_ptr<Window> m_Window;

        std::unique_ptr<AudioManager> m_AudioManager;
        std::unique_ptr<ComponentManager> m_ComponentManager;
        std::unique_ptr<DestroyManager> m_DestroyManager;
        std::unique_ptr<EntityManager> m_EntityManager;
        std::unique_ptr<LightManager> m_LightManager;
        std::unique_ptr<ShaderManager> m_ShaderManager;
        std::unique_ptr<TextureManager> m_TextureManager;

        std::vector<VkDescriptorPool> m_DescriptorPools;
    };
}
