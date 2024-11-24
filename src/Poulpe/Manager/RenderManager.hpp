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
    inline void forceRefresh() { _refresh = true; }
    inline uint32_t getAppHeight() { return getRenderer()->getAPI()->getSwapChainExtent().height; }
    inline uint32_t getAppWidth() { return getRenderer()->getAPI()->getSwapChainExtent().width; }
    inline AudioManager* getAudioManager() { return _audio_manager.get(); }
    inline Camera* getCamera() { return _camera.get(); }
    inline ComponentManager* getManager() { return _component_manager.get(); }
    inline DestroyManager* getDestroyManager() { return _destroy_manager.get(); }
    inline EntityManager* getEntityManager() { return _entity_manager.get(); }
    inline LightManager* getLightManager() { return _light_manager.get(); }
    inline Renderer* getRenderer() { return _renderer.get(); }
    inline ShaderManager* getShaderManager() { return _shader_manager.get(); }
    inline TextureManager* getTextureManager() { return _texture_manager.get(); }
    inline Window* getWindow() { return _window.get(); }
    void init();
    inline bool isLoaded()  { return _is_loaded; }
    void refresh(uint32_t levelIndex, bool showBbox = false, std::string_view skybox = "debug");
    void updateScene(double const delta_time);
    void renderScene();
    inline void setIsLoaded(bool loaded = true) { _is_loaded = loaded; }

  private:
    void loadData(std::string const & level);

    //@todo move to EntityManager
    void prepareHUD();
    void prepareSkybox();

  private:
    std::string _current_level;
    std::string _current_skybox;

    bool _is_loaded{ false };
    bool _refresh{ false };

    std::unique_ptr<Camera> _camera;
    std::unique_ptr<Renderer> _renderer;
    std::unique_ptr<Window> _window;

    std::unique_ptr<AudioManager> _audio_manager;
    std::unique_ptr<ComponentManager> _component_manager;
    std::unique_ptr<DestroyManager> _destroy_manager;
    std::unique_ptr<EntityManager> _entity_manager;
    std::unique_ptr<LightManager> _light_manager;
    std::unique_ptr<ShaderManager> _shader_manager;
    std::unique_ptr<TextureManager> _texture_manager;

    std::vector<VkDescriptorPool> _descriptor_pools;
  };
}
