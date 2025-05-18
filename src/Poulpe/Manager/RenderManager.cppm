module;

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <chrono>
#include <latch>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

export module Poulpe.Managers:RenderManager;

import :AudioManager;
import :DestroyManager;
import :EntityManager;
import :FontManager;
import :LightManager;
import :ShaderManager;
import :TextureManager;

import Poulpe.Component.Camera;
import Poulpe.Core.PlpTypedef;
import Poulpe.Core.Tools;
import Poulpe.GUI.Window;
import Poulpe.Managers.ComponentManager;
import Poulpe.Renderer.RendererComponentFactory;
import Poulpe.Renderer;
import Poulpe.Utils.IDHelper;

namespace Poulpe
{
  //@todo rename to SceneManager?
  export class RenderManager
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
    Window* getWindow();
    void init();
    inline bool isLoaded()  { return _is_loaded; }
    void updateScene(double const delta_time);
    void renderScene();
    inline void setIsLoaded(bool loaded = true) { _is_loaded = loaded; }
    void addText(FontManager::Text const& text);

    void updateText(std::string const& name, std::string const& text);
    void updateTextColor(std::string const& name, glm::vec3 const& color);

  private:
    void loadData(std::string const & level);

    //@todo move to EntityManager
    void prepareHUD();
    void prepareSkybox();
    void prepareTerrain();
    void prepareWater();

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
    std::unique_ptr<FontManager> _font_manager;
    std::unique_ptr<LightManager> _light_manager;
    std::unique_ptr<ShaderManager> _shader_manager;
    std::unique_ptr<TextureManager> _texture_manager;

    std::unordered_map<std::string, IDType> _texts;

    std::vector<VkDescriptorPool> _descriptor_pools;

    std::chrono::time_point<std::chrono::system_clock> _last_reload;
  };
}
