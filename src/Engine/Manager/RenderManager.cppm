export module Engine.Managers.RenderManager;

import std;

import Engine.Component.Entity;
import Engine.Component.EntityNode;
import Engine.Component.Mesh;

import Engine.Core.Camera;
import Engine.Core.GLM;
import Engine.Core.LightTypes;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;
import Engine.Core.Tools;
import Engine.Core.Volk;
import Engine.GUI.Window;

import Engine.Managers.AudioManager;
import Engine.Managers.ComponentManager;
import Engine.Managers.DestroyManager;
import Engine.Managers.EntityManager;
import Engine.Managers.FontManager;
import Engine.Managers.LightManager;
import Engine.Managers.PlayerManager;
import Engine.Managers.ShaderManager;
import Engine.Managers.TextureManager;

import Engine.Renderer.RendererComponentFactory;
import Engine.Renderer;
import Engine.Renderer.RendererComponentTypes;

import Engine.Utils.IDHelper;

namespace Poulpe
{
  struct FrameContext
  {
    double delta_time{};
    std::string root_path{};
    std::uint8_t camera_index{ 0 };

    glm::mat4 camera_view_matrix{ 1.0f };
    glm::mat4 perspective{ 1.0f };

    std::vector<glm::vec4> frustum_planes{};
    std::vector<IDType> visible_ids{};
    std::vector<IDType> transparent_ids{};

    std::shared_ptr<Entity> skybox_entity{};
    std::shared_ptr<Entity> terrain_entity{};
    std::shared_ptr<Entity> water_entity{};

    std::span<std::shared_ptr<Entity>> texts{};
  };

  export class RenderManager : public std::enable_shared_from_this<RenderManager>
  {
  public:
    RenderManager(Window* window);

    void cleanUp();
    inline void forceRefresh() { _refresh = true; }
    inline std::uint32_t getAppHeight() { return getRenderer()->getAPI()->getSwapChainExtent().height; }
    inline std::uint32_t getAppWidth() { return getRenderer()->getAPI()->getSwapChainExtent().width; }
    inline AudioManager* getAudioManager() { return _audio_manager.get(); }
    inline Camera* getCamera() const { return _cameras.at(_current_camera).get(); }
    inline ComponentManager* getComponentManager() { return _component_manager.get(); }
    inline DestroyManager* getDestroyManager() { return _destroy_manager.get(); }
    inline EntityManager* getEntityManager() { return _entity_manager.get(); }
    inline LightManager* getLightManager() { return _light_manager.get(); }
    inline Renderer* getRenderer() { return _renderer.get(); }
    inline ShaderManager* getShaderManager() { return _shader_manager.get(); }
    inline FontManager* getFontManager() { return _font_manager.get(); }
    inline TextureManager* getTextureManager() { return _texture_manager.get(); }
    Window* getWindow();
    void init();
    inline bool isLoaded()  { return _is_loaded; }
    //void updateScene(double const delta_time);
    void renderScene(double const delta_time);
    inline void setIsLoaded(bool loaded = true) { _is_loaded = loaded; }
    void addText(FontManager::Text & text);

    void updateText(IDType const entity_id, std::string const& text);
    void updateTextColor(IDType const entity_id, glm::vec3 const& color);

    void setElapsedTime(double const elapsed_time) { _elapsed_time = elapsed_time;}
    double getElapsedTime() const { return _elapsed_time; }

    Buffer getLightBuffer();

    RendererContext& getRendererContext(glm::mat4 const& camera_view = glm::mat4(1.0f));
    void updateRendererContext(glm::mat4 const& camera_view);

  private:
    void loadData(std::string const & level);

    bool isClipped(
    IDType const entity_id,
    std::span<glm::vec4 const> frustum_planes) const;

  void renderEntity(
    IDType const entity_id,
    double const delta_time,
    glm::mat4 const& camera_view_matrix);

  void drawEntity(
    IDType const entity_id,
    glm::mat4 const& camera_view_matrix,
    bool const has_alpha_blend = false);

  void drawShadowMap(
    IDType const entity_id,
    SHADOW_TYPE const shadow_type,
    glm::mat4 const& camera_view_matrix,
    bool const has_alpha_blend = false);

  auto get_visible_ids(
    std::ranges::range auto const& entities,
    std::vector<glm::vec4> const& frustum_planes) const
    {
      return entities
          | std::views::filter([&](auto const& e) { return !isClipped(e->getID(), frustum_planes); })
          | std::views::transform([](auto const& e) { return e->getID(); })
          | std::ranges::to<std::vector>();
    }

    //@todo move to EntityManager
    void prepareSkybox();
    void prepareTerrain();
    void prepareWater();

    FrameContext buildFrameContext(
      std::string const& root_path,
      std::uint8_t const camera_index,
      double const delta_time) const;

    void updateEntities(FrameContext const& frame_context);
    void renderShadowmap(FrameContext const& frame_context);
    void renderEntities(FrameContext const& frame_context);
    void updateBuffers();
    void updatePlayer();

  private:
    std::string _current_level;
    std::string _current_skybox;
    std::uint32_t _env_options {0};

    bool _is_loaded{ false };
    bool _refresh{ false };
    double _elapsed_time{ 0.0 };
    std::vector<std::unique_ptr<Camera>> _cameras;
    std::uint8_t _current_camera { 0 };
    std::unique_ptr<Renderer> _renderer;
    std::unique_ptr<Window> _window;

    std::unique_ptr<AudioManager> _audio_manager;
    std::unique_ptr<ComponentManager> _component_manager;
    std::unique_ptr<DestroyManager> _destroy_manager;
    std::unique_ptr<EntityManager> _entity_manager;
    std::unique_ptr<FontManager> _font_manager;
    std::unique_ptr<LightManager> _light_manager;
    std::unique_ptr<PlayerManager> _player_manager;
    std::unique_ptr<ShaderManager> _shader_manager;
    std::unique_ptr<TextureManager> _texture_manager;

    std::vector<VkDescriptorPool> _descriptor_pools;

    std::chrono::time_point<std::chrono::system_clock> _last_reload;

    std::vector<Buffer> _light_buffers;

    RendererContext _rendering_context;

    std::vector<std::future<void>> _tasks{};
  };
}
