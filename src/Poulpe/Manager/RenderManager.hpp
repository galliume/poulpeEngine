#pragma once

//#include "Poulpe/Core/ThreadPool.hpp"

#include "Poulpe/Manager/AudioManager.hpp"
#include "Poulpe/Manager/ComponentManager.hpp"
#include "Poulpe/Manager/DestroyManager.hpp"
#include "Poulpe/Manager/EntityManager.hpp"
#include "Poulpe/Manager/LightManager.hpp"
#include "Poulpe/Manager/ShaderManager.hpp"
#include "Poulpe/Manager/TextureManager.hpp"

#include "Poulpe/Component/Camera.hpp"

#include "Poulpe/Renderer/Vulkan/Renderer.hpp"

#include "Poulpe/GUI/Window.hpp"

#include <vulkan/vulkan.h>

namespace Poulpe
{
  //@todo rename to SceneManager?
  class RenderManager
  {
  public:
    RenderManager(Window* window);

    void cleanUp();
    inline void forceRefresh() { m_Refresh = true; }
    inline uint32_t getAppHeight() { return getRenderer()->getSwapChainExtent().height; }
    inline uint32_t getAppWidth() { return getRenderer()->getSwapChainExtent().width; }
    inline AudioManager* getAudioManager() { return m_AudioManager.get(); }
    inline Camera* getCamera() { return m_Camera.get(); }
    inline ComponentManager* getManager() { return m_ComponentManager.get(); }
    inline DestroyManager* getDestroyManager() { return m_DestroyManager.get(); }
    inline EntityManager* getEntityManager() { return m_EntityManager.get(); }
    inline LightManager* getLightManager() { return m_LightManager.get(); }
    inline Renderer* getRenderer() { return m_Renderer.get(); }
    inline ShaderManager* getShaderManager() { return m_ShaderManager.get(); }
    inline TextureManager* getTextureManager() { return m_TextureManager.get(); }
    inline Window* getWindow() { return m_Window.get(); }
    void init();
    inline bool isLoaded()  { return m_IsLoaded; }
    void refresh(uint32_t levelIndex, bool showBbox = false, std::string_view skybox = "debug");
    void renderScene(std::chrono::duration<float> deltaTime);
    inline void setDrawBbox(bool draw) { m_Renderer->setDrawBbox(draw); }
    inline void setIsLoaded(bool loaded = true) { m_IsLoaded = loaded; }

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
    std::unique_ptr<Renderer> m_Renderer;
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
