#include "RenderManager.hpp"

#include "Poulpe/Component/AnimationComponent.hpp"
#include "Poulpe/Component/BoneAnimationComponent.hpp"
#include "Poulpe/Component/MeshComponent.hpp"
#include "Poulpe/Component/RenderComponent.hpp"

#include "Poulpe/Component/Renderer/RendererFactory.hpp"

#include "Poulpe/Renderer/Vulkan/Renderer.hpp"

#include <latch>

namespace Poulpe
{
  RenderManager::RenderManager(Window* const window)
  {
    _window = std::unique_ptr<Window>(window);
    _component_manager = std::make_unique<ComponentManager>();
    _light_manager = std::make_unique<LightManager>();
    _texture_manager = std::make_unique<TextureManager>();
    _entity_manager = std::make_unique<EntityManager>(
      _component_manager.get(),
      _light_manager.get(),
      _texture_manager.get());

    _renderer = std::make_unique<Renderer>(
      _window.get(),
      _entity_manager.get(),
      _component_manager.get(),
      _light_manager.get(),
      _texture_manager.get());

    _audio_manager = std::make_unique<AudioManager>();
    _shader_manager = std::make_unique<ShaderManager>();
    _destroy_manager = std::make_unique<DestroyManager>();
    _camera = std::make_unique<Camera>();

    _renderer->init();
    _renderer->addCamera(_camera.get());
    _destroy_manager->setRenderer(_renderer.get());
    _destroy_manager->addMemoryPool(_renderer->getAPI()->getDeviceMemoryPool());

    _font_manager = std::make_unique<FontManager>();
    _font_manager->addRenderer(_renderer.get());
    auto atlas = _font_manager->load();

    _texture_manager->addTexture(atlas);

    //@todo, those managers should not have the usage of the renderer...
    _texture_manager->addRenderer(_renderer.get());
    _entity_manager->addRenderer(_renderer.get());
    _shader_manager->addRenderer(_renderer.get());

    Locator::getInputManager()->setCamera(_camera.get());
    //end @todo
  }

  void RenderManager::cleanUp()
  {
    _destroy_manager->cleanShaders(_shader_manager->getShaders()->shaders);
    _destroy_manager->cleanTextures(_texture_manager->getTextures());
    _destroy_manager->cleanTexture(_texture_manager->getSkyboxTexture());
    _audio_manager->clear();
    _texture_manager->clear();
    _entity_manager->clear();
    _shader_manager->clear();
    _renderer->clear();

    _component_manager->clear();
  }

  void RenderManager::init()
  {
    auto * const configManager = Poulpe::Locator::getConfigManager();

    nlohmann::json const& appConfig = configManager->appConfig();

    _audio_manager->init();
    _audio_manager->load(configManager->soundConfig());

    Locator::getInputManager()->init(appConfig["input"]);
    
    if (appConfig["defaultLevel"].empty()) {
      PLP_WARN("defaultLevel conf not set.");
    }

    _current_level = appConfig["defaultLevel"].get<std::string>();

    loadData(_current_level);

    if (appConfig["ambientMusic"].get<bool>()) {
      _audio_manager->startAmbient();
    }

    if (!_camera->isInit()) {
      auto const& lvl_config{ configManager->lvlConfig() };
      auto const& camera{ lvl_config["camera"] };

      glm::vec3 const start_pos{
        camera["position"]["x"].get<float>(),
        camera["position"]["y"].get<float>(),
        camera["position"]["z"].get<float>() };

      _camera->init(start_pos);
    }

    prepareSkybox();
    prepareTerrain();
    prepareWater();

    FontManager::Text text {
      .name = "_plp_title",
      .text = "@ € $ 0 7 é è ù ü ä ö π ∞ β Æ ‰ Ü Γ Đ Ặ Ω ! ₲ ◀ MrPoulpe ▶™",
      .position = glm::vec3(-200.0f, -100.0f, 0.0f), //@todo fix this -
      .color = glm::vec3(1.0f, 0.2f, 0.6f),
      .scale = 0.2,
      .flat = false
    };
    addText(text);
    //prepareHUD();
  }

  template <typename T>
  T lerp(T const& startValue, T const& endValue, float const& t) {
    return ((1.0f - t) * startValue) + (t * endValue);
  }

  void RenderManager::updateScene(double const delta_time)
  {
    //@todo animate light
    //_light_manager->animateAmbientLight(delta_time);

    {
      std::lock_guard guard(_entity_manager->lockWorldNode());

      auto * const config_manager = Poulpe::Locator::getConfigManager();

      //@todo improve this draft for simple shader hot reload
      if (config_manager->reloadShaders()) {
        std::filesystem::path p = std::filesystem::current_path();
        auto cmd{ p.string() + "/bin/shaders_compil.sh" };
        std::system(cmd.c_str());

        std::latch count_down{ 1 };
        _shader_manager->load(config_manager->shaderConfig())(count_down);

        config_manager->setReloadShaders(false);
      }

      auto entity = _entity_manager->getWater();
      if (entity != nullptr) {
        auto* mesh_component = _component_manager->get<MeshComponent>(entity->getID());
        auto mesh = mesh_component->template has<Mesh>();
        if (mesh) {
          auto* animation_component = _component_manager->get<AnimationComponent>(entity->getID());
          if (animation_component) {
            (*animation_component)(delta_time, mesh);
          }
        }
      }

      std::ranges::for_each(_entity_manager->getTexts(), [&](auto const& entity) {
        auto* mesh_component = _component_manager->get<MeshComponent>(entity->getID());
        auto mesh = mesh_component->template has<Mesh>();

        if (mesh) {
          auto rdr_impl = _component_manager->get<RenderComponent>(entity->getID());
          if (mesh->isDirty() && rdr_impl) {
            (*rdr_impl)(delta_time, mesh);
          }
        }
      });

      auto* world_node = _entity_manager->getWorldNode();

      std::ranges::for_each(world_node->getChildren(), [&](const auto& leaf_node) {
        std::ranges::for_each(leaf_node->getChildren(), [&](const auto& entity_node) {

          auto const& entity = entity_node->getEntity();

          auto* mesh_component = _component_manager->get<MeshComponent>(entity->getID());
          auto mesh = mesh_component->template has<Mesh>();

          if (mesh) {
            //if (mesh->hasBufferStorage()) {
            //  auto objectBuffer = mesh->getObjectBuffer();
            //  objectBuffer->point_lights[0] = _light_manager->getPointLights().at(0);

            //  _renderer->getAPI()->updateStorageBuffer(mesh->getStorageBuffers()->at(0), *objectBuffer);
            //}

            auto rdr_impl = _component_manager->get<RenderComponent>(entity->getID());
            if (mesh->isDirty() && rdr_impl) {
              (*rdr_impl)(delta_time, mesh);
            }

            auto* animation_component = _component_manager->get<AnimationComponent>(leaf_node->getEntity()->getID());
            if (animation_component && leaf_node->isLoaded()) {
              (*animation_component)(delta_time, mesh);
            }

            auto* boneAnimationComponent = _component_manager->get<BoneAnimationComponent>(leaf_node->getEntity()->getID());
            if (boneAnimationComponent) {
              (*boneAnimationComponent)(delta_time, mesh);
              mesh->setIsDirty(true);

              auto buffer{ mesh->getObjectStorageBuffer() };
             if (buffer) {
                BonesBuffer* bones_buffer = mesh->getBonesBuffer();
                //bones_buffer->bone_matrices = mesh->getData()->_bone_matrices;
                bones_buffer->bone_matrices = {glm::mat4(1.0f), glm::mat4(231.0f)};

                //_renderer->getAPI()->updateStorageBuffer<BonesBuffer>(*buffer, *bones_buffer);
              }
            }
          }
        });
      });

      
    }

    //if (_refresh) {
    //  init();
    //  _refresh = false;
    //}
  }
  void RenderManager::renderScene()
  {
    _renderer->renderScene();
  }

  void RenderManager::loadData(std::string const & level)
  {
    auto * const config_manager = Poulpe::Locator::getConfigManager();

    nlohmann::json const& app_config = config_manager->appConfig();

    auto const& lvl_data = config_manager->loadLevelData(level);
    _texture_manager->addConfig(config_manager->texturesConfig());

    std::string const sb{ (_current_skybox.empty()) ? static_cast<std::string>(app_config["defaultSkybox"])
      : _current_skybox };

    std::latch count_down{ 3 };

    std::jthread textures(std::move(std::bind(_texture_manager->load(), std::ref(count_down))));
    textures.detach();
    std::jthread skybox(std::move(std::bind(_texture_manager->loadSkybox(sb), std::ref(count_down))));
    skybox.detach();
    std::jthread shaders(std::move(std::bind(_shader_manager->load(config_manager->shaderConfig()), std::ref(count_down))));
    shaders.detach();
    count_down.wait();

    setIsLoaded();

    std::jthread entities(std::move(_entity_manager->load(lvl_data)));
    entities.detach();
  }

  void RenderManager::prepareHUD()
  {
    double const delta_time{ 0.0 };

    auto grid_entity = std::make_unique<Entity>();
    auto grid_mesh = std::make_unique<Mesh>();
    auto grid_rdr_impl{ RendererFactory::create<Grid>() };
    grid_rdr_impl->init(_renderer.get(), _texture_manager.get(), nullptr);

    (*grid_rdr_impl)(delta_time, grid_mesh.get());

    _component_manager->add<RenderComponent>(grid_entity->getID(), std::move(grid_rdr_impl));
    _component_manager->add<MeshComponent>(grid_entity->getID(), std::move(grid_mesh));

    auto crosshair_entity = std::make_unique<Entity>();
    auto crosshair_mesh = std::make_unique<Mesh>();
    auto crosshair_rdr_impl{ RendererFactory::create<Crosshair>() };
    crosshair_rdr_impl->init(_renderer.get(), _texture_manager.get(), nullptr);

    (*crosshair_rdr_impl)(delta_time, crosshair_mesh.get());
    _component_manager->add<RenderComponent>(crosshair_entity->getID(), std::move(crosshair_rdr_impl));
    _component_manager->add<MeshComponent>(crosshair_entity->getID(), std::move(crosshair_mesh));

    _entity_manager->addHUD(std::move(grid_entity));
    _entity_manager->addHUD(std::move(crosshair_entity));
  }

  void RenderManager::prepareSkybox()
  {
    auto entity = std::make_unique<Entity>();
    auto mesh = std::make_unique<Mesh>();
    mesh->setHasShadow(false);
    mesh->setIsIndexed(false);

    auto rdr_impl{ RendererFactory::create<Skybox>() };
    rdr_impl->init(_renderer.get(), _texture_manager.get(), nullptr);

    double const delta_time{ 0.0 };
    (*rdr_impl)(delta_time, mesh.get());

    _component_manager->add<RenderComponent>(entity->getID(), std::move(rdr_impl));
    _component_manager->add<MeshComponent>(entity->getID(), std::move(mesh));
    _entity_manager->setSkybox(std::move(entity));
  }

  void RenderManager::prepareTerrain()
  {
    auto entity = std::make_unique<Entity>();
    auto mesh = std::make_unique<Mesh>();
    mesh->setHasShadow(false);
    mesh->setIsIndexed(false);
    mesh->setShaderName("terrain");
    auto rdr_impl{ RendererFactory::create<Terrain>() };
    rdr_impl->init(_renderer.get(), _texture_manager.get(), nullptr);

    double const delta_time{ 0.0 };
    (*rdr_impl)(delta_time, mesh.get());

    _component_manager->add<RenderComponent>(entity->getID(), std::move(rdr_impl));
    _component_manager->add<MeshComponent>(entity->getID(), std::move(mesh));
    _entity_manager->setTerrain(std::move(entity));

    _renderer->addEntity(_entity_manager->getTerrain(), true);
  }

  void RenderManager::prepareWater()
  {
    auto entity = std::make_unique<Entity>();
    auto mesh = std::make_unique<Mesh>();
    mesh->setHasShadow(false);
    mesh->setIsIndexed(false);
    mesh->setShaderName("water");
    mesh->setName("_plp_water");

    auto rdr_impl{ RendererFactory::create<Water>() };
    rdr_impl->init(_renderer.get(), _texture_manager.get(), nullptr);

    double const delta_time{ 0.0 };
    (*rdr_impl)(delta_time, mesh.get());

    _component_manager->add<RenderComponent>(entity->getID(), std::move(rdr_impl));
    _component_manager->add<MeshComponent>(entity->getID(), std::move(mesh));

    auto animationScript = std::make_unique<AnimationScript>("assets/scripts/animation/water/anim0.lua");
    animationScript->init(_renderer.get(), nullptr, nullptr);
    //_component_manager->add<AnimationComponent>(entity->getID(), std::move(animationScript));

    _entity_manager->setWater(std::move(entity));

    _renderer->addTransparentEntity(_entity_manager->getWater(), true);
  }

  void RenderManager::addText(FontManager::Text const& text)
  {
    auto entity = std::make_unique<Entity>();
    auto mesh = std::make_unique<Mesh>();
    mesh->setHasShadow(false);
    mesh->setIsIndexed(false);
    mesh->setShaderName("text");
    mesh->setName("_plp_text_" + std::to_string(entity->getID()));

    auto rdr_impl{ RendererFactory::create<Text>() };
    rdr_impl->init(_renderer.get(), _texture_manager.get(), nullptr);
    rdr_impl->addFontManager(_font_manager.get());
    rdr_impl->setText(text.text);
    rdr_impl->setPosition(text.position);
    rdr_impl->setColor(text.color);
    rdr_impl->setScale(text.scale);
    rdr_impl->setFlat(text.flat);
    
    double const delta_time{ 0.0 };
    (*rdr_impl)(delta_time, mesh.get());

    _component_manager->add<RenderComponent>(entity->getID(), std::move(rdr_impl));
    _component_manager->add<MeshComponent>(entity->getID(), std::move(mesh));

    _texts[text.name] = entity->getID();

    auto index = _entity_manager->addText(std::move(entity));

    _renderer->addTextEntity(_entity_manager->getText(index), true);
  }

  void RenderManager::updateText(std::string const& name, std::string const& text)
  {
    auto const entity_id = _texts[name];

    auto mesh_component = _component_manager->get<MeshComponent>(entity_id);
    if (mesh_component) {
      auto mesh = mesh_component->has<Mesh>();
      auto rdr_impl = _component_manager->get<RenderComponent>(entity_id);
      if (rdr_impl) {
        auto text_rdr = rdr_impl->has<Text>();

        if (text_rdr) {
          text_rdr->setText(text);
          mesh->setIsDirty(true);
        }
      }
    }
  }
  
  //@todo better way to update text
  void RenderManager::updateTextColor(std::string const& name, glm::vec3 const& color)
  {
    auto const entity_id = _texts[name];

    auto mesh_component = _component_manager->get<MeshComponent>(entity_id);
    if (mesh_component) {
      auto mesh = mesh_component->has<Mesh>();
      auto rdr_impl = _component_manager->get<RenderComponent>(entity_id);
      if (rdr_impl) {
        auto text_rdr = rdr_impl->has<Text>();

        if (text_rdr) {
          text_rdr->setColor(color);
          mesh->setIsDirty(true);
        }
      }
    }
  }
}
