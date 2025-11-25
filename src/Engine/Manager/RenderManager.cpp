module;


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

#include <nlohmann/json_fwd.hpp>

module Engine.Managers.RenderManager;

import std;

import Engine.Animation.AnimationScript;
import Engine.Animation.AnimationTypes;

import Engine.Component.Camera;
import Engine.Component.Components;
import Engine.Component.Entity;

import Engine.Core.Logger;

import Engine.GUI.Window;

import Engine.Managers.ComponentManager;
import Engine.Managers.ConfigManagerLocator;
import Engine.Managers.InputManagerLocator;

import Engine.Renderer;
import Engine.Renderer.Renderers;
import Engine.Renderer.RendererComponent;
import Engine.Renderer.RendererComponentTypes;
import Engine.Renderer.RendererComponentFactory;
import Engine.Renderer.Vulkan.Mesh;

import Engine.Utils.ScopedTimer;

namespace Poulpe
{
  RenderManager::RenderManager(Window* const window)
  {
    _window = std::unique_ptr<Window>(window);
    _camera = std::make_unique<Camera>();
    _renderer = std::make_unique<Renderer>(_window.get());

    _component_manager = std::make_unique<ComponentManager>();
    _light_manager = std::make_unique<LightManager>();
    _texture_manager = std::make_unique<TextureManager>();

    _audio_manager = std::make_unique<AudioManager>();
    _shader_manager = std::make_unique<ShaderManager>();
    _destroy_manager = std::make_unique<DestroyManager>();

    _renderer->init();
    _destroy_manager->setRenderer(_renderer.get());
    _destroy_manager->addMemoryPool(_renderer->getAPI()->getDeviceMemoryPool());

    _light_buffers.resize(2);
    for (std::size_t i = 0; i < _light_buffers.size(); ++i) {
        LightObjectBuffer light_object_buffer{};
        _light_buffers[i] = _renderer->getAPI()->createStorageBuffers(light_object_buffer);
    }

    _entity_manager = std::make_unique<EntityManager>(
      _component_manager.get(),
      _light_manager.get(),
      _texture_manager.get(),
      _light_buffers.at(0)
    );

    _font_manager = std::make_unique<FontManager>();
    _font_manager->addRenderer(_renderer.get());
    auto atlas = _font_manager->load();

    _texture_manager->addTexture(atlas);

    //@todo, those managers should not have the usage of the renderer...
    _entity_manager->addRenderer(_renderer.get());
    _shader_manager->addRenderer(_renderer.get());

    InputManagerLocator::get()->setCamera(_camera.get());

    //end @todo

    //_audio_manager->startAmbient(0);s
  }

  void RenderManager::cleanUp()
  {
    // _destroy_manager->cleanShaders(_shader_manager->getShaders()->shaders);
    // _destroy_manager->cleanTextures(_texture_manager->getTextures());
    _audio_manager->clear();
    _texture_manager->clear();
    _entity_manager->clear();
    _shader_manager->clear();
    _renderer->clear();

    _component_manager->clear();
  }

  void RenderManager::init()
  {
    auto * const configManager = ConfigManagerLocator::get();
    auto const& appConfig { configManager->appConfig() };
    _audio_manager->init();
    _audio_manager->load(configManager->soundConfig());

    InputManagerLocator::get()->init(appConfig["input"]);

    if (appConfig["defaultLevel"].empty()) {
      Logger::warn("defaultLevel conf not set.");
    }

    _current_level = appConfig["defaultLevel"].get<std::string>();

    loadData(_current_level);

    if (appConfig["ambientMusic"].get<bool>()) {
      _audio_manager->startAmbient();
    }

    if (!_camera->isInit()) {
      auto const& lvl_config{ configManager->lvlConfig() };
      auto const& camera{ lvl_config["camera"] };
      glm::vec3 const start_pos = {
        camera["position"]["x"].template get<float>(),
        camera["position"]["y"].template get<float>(),
        camera["position"]["z"].template get<float>() };

      _camera->init(start_pos);
      _camera->forward();
      _camera->move();
    }

    auto const camera_view_matrix = _camera->getView();

    updateComponentRenderingInfo();
    updateRendererInfo(camera_view_matrix);

    prepareShadowMap();
    prepareSkybox();
    prepareTerrain();
    prepareWater();

    FontManager::Text text {
      .name = "_plp_title",
      .text = "@ € $ 0 7 é è ù ü ä ö π ∞ β Æ ‰ Ü Γ Đ Ặ Ω ! ₲ ◀ MrPoulpe ▶™",
      .position = glm::vec3(0.0f, 100.0f, 0.0f),
      .color = glm::vec3(1.0f, 0.2f, 0.6f),
      .scale = 0.2f,
      .flat = false
    };
    addText(text);
    //prepareHUD();
  }

  template <typename T>
  T lerp(T const& startValue, T const& endValue, float const& t) {
    return ((1.0f - t) * startValue) + (t * endValue);
  }

  void RenderManager::renderScene(double const delta_time)
  {
    //@todo animate light
    //_light_manager->animateAmbientLight(delta_time);
    {
      auto const camera_view_matrix = _camera->getView();

      std::lock_guard<std::shared_mutex> guard(_entity_manager->lockWorldNode());

      _light_manager->computeCSM(camera_view_matrix, _renderer->getPerspective());
      //Logger::debug("x {} y {} z {}", camera_pos.x, camera_pos.y, camera_pos.z);

      updateComponentRenderingInfo();
      updateRendererInfo(camera_view_matrix);

      //auto renderer_info { getRendererInfo() };

      LightObjectBuffer light_object_buffer{};
      light_object_buffer.point_lights[0] = _light_manager->getPointLights()[0];
      light_object_buffer.point_lights[1] = _light_manager->getPointLights()[1];
      light_object_buffer.spot_light = _light_manager->getSpotLights()[0];
      light_object_buffer.sun_light = _light_manager->getSunLight();

      _renderer->getAPI()->updateStorageBuffer(
        _light_buffers[_renderer->getCurrentFrameIndex()], light_object_buffer);

      std::future<void> async_skybox_render;
      std::future<void> async_water_render;
      std::future<void> async_terrain_render;
      std::vector<std::future<void>> async_texts_render{};
      async_texts_render.reserve(_entity_manager->getTexts().size());
      std::vector<std::future<void>> async_entities_render;
      std::vector<std::future<void>> async_transparent_entities_render;

      auto * const config_manager { ConfigManagerLocator::get() };
      auto const & root_path { config_manager->rootPath() };

      //@todo improve this draft for simple shader hot reload
      if (config_manager->reloadShaders()) {
        std::filesystem::path p = std::filesystem::current_path();
        auto cmd{ root_path + "/bin/shaders_compil.sh" };
        std::system(cmd.c_str());

        std::latch count_down{ 1 };
        _shader_manager->load(config_manager->shaderConfig())(count_down);

        config_manager->setReloadShaders(false);
      }

      auto const skybox_entity = _entity_manager->getSkybox();
      if (skybox_entity != nullptr) {
        async_skybox_render = std::async(std::launch::deferred, [&]() {
          renderEntity(skybox_entity->getID(), delta_time);
        });
      }
      auto const terrain_entity = _entity_manager->getTerrain();
      if (terrain_entity != nullptr) {
        async_terrain_render = std::async(std::launch::deferred, [&]() {
          renderEntity(terrain_entity->getID(), delta_time);
        });
      }
      auto const water_entity = _entity_manager->getWater();
      if (water_entity != nullptr) {
        auto* mesh_component = _component_manager->get<MeshComponent>(water_entity->getID());
        auto mesh = mesh_component->template has<Mesh>();
       if (mesh) {
          glm::vec4 options{ getElapsedTime(), 0.0f, 0.0f, 0.0f};
          mesh->setOptions(options);
        }
        async_water_render = std::async(std::launch::deferred, [&]() {
          renderEntity(water_entity->getID(), delta_time);
        });
      }

      auto const& entities = _entity_manager->getEntities();
      auto const& transparent_entities = _entity_manager->getTransparentEntities();

      std::ranges::for_each(entities, [&](const auto& entity) {
        async_entities_render.push_back(std::async(std::launch::deferred, [&]() {
          renderEntity(entity->getID(), delta_time);
        }));
      });

      std::ranges::for_each(transparent_entities, [&](const auto& entity) {
        async_transparent_entities_render.push_back(std::async(std::launch::deferred, [&]() {
          renderEntity(entity->getID(), delta_time);
        }));
      });

      std::ranges::for_each(_entity_manager->getTexts(), [&](auto const& text_entity) {
        async_texts_render.push_back(
          std::async(std::launch::deferred, [&]() {
            renderEntity(text_entity->getID(), delta_time);
        }));
      });

      async_skybox_render.wait();
      async_terrain_render.wait();
      async_water_render.wait();

      for (auto& future : async_entities_render) {
        future.wait();
      }
      for (auto& future : async_transparent_entities_render) {
        future.wait();
      }
      for (auto& future : async_texts_render) {
        future.wait();
      }

      _renderer->start();
      _renderer->startShadowMap(SHADOW_TYPE::CSM);

      std::ranges::for_each(entities, [&](const auto& entity) {
        drawShadowMap(entity->getID(), SHADOW_TYPE::CSM, camera_view_matrix);
      });
      _renderer->endShadowMap(SHADOW_TYPE::CSM);


      _renderer->startShadowMap(SHADOW_TYPE::SPOT_LIGHT);

      std::ranges::for_each(entities, [&](const auto& entity) {
        drawShadowMap(entity->getID(), SHADOW_TYPE::SPOT_LIGHT, camera_view_matrix);
      });
      _renderer->endShadowMap(SHADOW_TYPE::SPOT_LIGHT);

      _renderer->startRender();

      if (skybox_entity != nullptr) {
        drawEntity(skybox_entity->getID(), camera_view_matrix);
      }

      if (terrain_entity != nullptr) {
        drawEntity(terrain_entity->getID(), camera_view_matrix);
      }

      std::ranges::for_each(entities, [&](const auto& entity) {
        drawEntity(entity->getID(), camera_view_matrix, true);
      });
      std::ranges::for_each(transparent_entities, [&](const auto& entity) {
        drawEntity(entity->getID(), camera_view_matrix, true);
      });

      if (water_entity != nullptr) {
        drawEntity(water_entity->getID(), camera_view_matrix);
      }

      std::ranges::for_each(_entity_manager->getTexts(), [&](auto const& text_entity) {
        drawEntity(text_entity->getID(), camera_view_matrix, true);
      });

      _renderer->endRender();
      _renderer->submit();
    }
  }

  void RenderManager::renderEntity(
    IDType const entity_id,
    double const delta_time
  )
  {
    auto* mesh_component = _component_manager->get<MeshComponent>(entity_id);
    auto mesh = mesh_component->template has<Mesh>();
    auto rdr_impl = _component_manager->get<RendererComponent>(entity_id);

    if (mesh && rdr_impl) {
      if (mesh->isDirty()) {
        (*rdr_impl)(_renderer.get(), getComponentRenderingInfo(mesh));
      }

      AnimationInfo const animation_info {
        .delta_time = delta_time,
        .data = mesh->getData()
      };

      auto* animation_component = _component_manager->get<AnimationComponent>(entity_id);
      if (animation_component) {
        (*animation_component)(animation_info);
      }

      auto* boneAnimationComponent = _component_manager->get<BoneAnimationComponent>(entity_id);
      if (boneAnimationComponent) {
        (*boneAnimationComponent)(animation_info);
        mesh->setIsDirty(true);
      }
    }
  }

  void RenderManager::drawEntity(
    IDType const entity_id,
    glm::mat4 const& camera_view_matrix,
    bool const has_alpha_blend)
  {
    auto* mesh_component = _component_manager->get<MeshComponent>(entity_id);
    auto mesh = mesh_component->template has<Mesh>();
    auto rdr_impl = _component_manager->get<RendererComponent>(entity_id);

    if (mesh && rdr_impl) {
      RendererInfo renderer_info  = getRendererInfo(mesh, camera_view_matrix);
      renderer_info.stage_flag_bits = rdr_impl->getShaderStageFlags();
      renderer_info.has_alpha_blend = has_alpha_blend;

      _renderer->draw(renderer_info);
    }
  }

    void RenderManager::drawShadowMap(
    IDType const entity_id,
    SHADOW_TYPE const shadow_type,
    glm::mat4 const& camera_view_matrix,
    bool const has_alpha_blend)
  {
    auto* mesh_component { _component_manager->get<MeshComponent>(entity_id) };
    auto mesh { mesh_component->template has<Mesh>() };

    if (!mesh->hasShadow()) {
      return;
    }

    auto rdr_impl = _component_manager->get<RendererComponent>(entity_id);

    if (mesh && rdr_impl) {
      RendererInfo renderer_info  = getRendererInfo(mesh, camera_view_matrix);
      renderer_info.stage_flag_bits = rdr_impl->getShaderStageFlags();
      renderer_info.has_alpha_blend = has_alpha_blend;

      auto const component_rendering_info { getComponentRenderingInfo(mesh) };

      _renderer->drawShadowMap(renderer_info, shadow_type);
    }
  }

  void RenderManager::loadData(std::string const & level)
  {
    auto * const config_manager = ConfigManagerLocator::get();
    auto const& app_config{ config_manager->appConfig() };

    auto const& lvl_data = config_manager->loadLevelData(level);
    _texture_manager->addConfig(config_manager->texturesConfig());

    std::string const sb{ (_current_skybox.empty()) ? static_cast<std::string>(app_config["defaultSkybox"])
      : _current_skybox };

    std::latch count_down{ 3 };

    std::jthread textures(std::bind(_texture_manager->load(_renderer.get()), std::ref(count_down)));
    textures.detach();
    std::jthread skybox(std::bind(_texture_manager->loadSkybox(sb, _renderer.get()), std::ref(count_down)));
    skybox.detach();
    std::jthread shaders(std::bind(_shader_manager->load(config_manager->shaderConfig()), std::ref(count_down)));
    shaders.detach();
    count_down.wait();

    setIsLoaded();

    std::jthread entities(_entity_manager->load(lvl_data));
    entities.detach();
  }

  void RenderManager::prepareHUD()
  {
    auto grid_entity = std::make_unique<Entity>();
    auto grid_mesh = std::make_unique<Mesh>();
    auto grid_rdr_impl{ RendererComponentFactory::create<Grid>() };

    (*grid_rdr_impl)(_renderer.get(), getComponentRenderingInfo(grid_mesh.get()));

    _component_manager->add<RendererComponent>(grid_entity->getID(), std::move(grid_rdr_impl));
    _component_manager->add<MeshComponent>(grid_entity->getID(), std::move(grid_mesh));

    auto crosshair_entity = std::make_unique<Entity>();
    auto mesh = std::make_unique<Mesh>();

    auto crosshair_rdr_impl{ RendererComponentFactory::create<Crosshair>() };
    (*crosshair_rdr_impl)(_renderer.get(), getComponentRenderingInfo(mesh.get()));

    _component_manager->add<RendererComponent>(crosshair_entity->getID(), std::move(crosshair_rdr_impl));
    _component_manager->add<MeshComponent>(crosshair_entity->getID(), std::move(mesh));

    _entity_manager->addHUD(std::move(grid_entity));
    _entity_manager->addHUD(std::move(crosshair_entity));
  }

  void RenderManager::prepareSkybox()
  {
    auto entity = std::make_unique<Entity>();
    auto mesh = std::make_unique<Mesh>();
    mesh->setName("skybox");

    auto rdr_impl{ RendererComponentFactory::create<Skybox>() };
    (*rdr_impl)(_renderer.get(), getComponentRenderingInfo(mesh.get()));

    _component_manager->add<RendererComponent>(entity->getID(), std::move(rdr_impl));
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

    auto rdr_impl{ RendererComponentFactory::create<Terrain>() };
    (*rdr_impl)(_renderer.get(), getComponentRenderingInfo(mesh.get()));

    _component_manager->add<RendererComponent>(entity->getID(), std::move(rdr_impl));
    _component_manager->add<MeshComponent>(entity->getID(), std::move(mesh));
    _entity_manager->setTerrain(std::move(entity));
  }

  void RenderManager::prepareWater()
  {
    auto entity = std::make_unique<Entity>();
    auto mesh = std::make_unique<Mesh>();
    mesh->setHasShadow(false);
    mesh->setIsIndexed(false);
    mesh->setShaderName("water");
    mesh->setName("_plp_water");

    auto rdr_impl{ RendererComponentFactory::create<Water>() };
    (*rdr_impl)(_renderer.get(), getComponentRenderingInfo(mesh.get()));

    _component_manager->add<RendererComponent>(entity->getID(), std::move(rdr_impl));
    _component_manager->add<MeshComponent>(entity->getID(), std::move(mesh));
    _entity_manager->setWater(std::move(entity));
  }

  void RenderManager::prepareShadowMap()
  {
    // auto entity = std::make_unique<Entity>();
    // auto mesh = std::make_unique<Mesh>();
    // mesh->setHasShadow(false);
    // mesh->setIsIndexed(false);
    // mesh->setShaderName("shadow_map");
    // mesh->setName("_plp_shadow_map");

    // auto rdr_impl{ RendererComponentFactory::create<ShadowMap>() };
    // (*rdr_impl)(_renderer.get(), getComponentRenderingInfo(mesh.get()));

    // _entity_manager->setShadowMap(std::move(entity));
  }

  void RenderManager::addText(FontManager::Text const& text)
  {
    auto entity = std::make_unique<Entity>();
    auto mesh = std::make_unique<Mesh>();
    mesh->setHasShadow(false);
    mesh->setIsIndexed(false);
    mesh->setShaderName("text");
    mesh->setName("_plp_text_" + std::to_string(entity->getID()));

    auto rdr_impl{ RendererComponentFactory::create<Text>() };
    rdr_impl->setText(text.text);
    rdr_impl->setPosition(text.position);
    rdr_impl->setColor(text.color);
    rdr_impl->setScale(text.scale);
    rdr_impl->setFlat(text.flat);

    (*rdr_impl)(_renderer.get(), getComponentRenderingInfo(mesh.get()));

    _component_manager->add<RendererComponent>(entity->getID(), std::move(rdr_impl));
    _component_manager->add<MeshComponent>(entity->getID(), std::move(mesh));

    _texts[text.name] = entity->getID();

    _entity_manager->addText(std::move(entity));
  }

  void RenderManager::updateText(std::string const& name, std::string const& text)
  {
    auto const entity_id = _texts[name];

    auto mesh_component = _component_manager->get<MeshComponent>(entity_id);
    if (mesh_component) {
      auto mesh = mesh_component->has<Mesh>();
      auto rdr_impl = _component_manager->get<RendererComponent>(entity_id);
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
      auto rdr_impl = _component_manager->get<RendererComponent>(entity_id);
      if (rdr_impl) {
        auto text_rdr = rdr_impl->has<Text>();

        if (text_rdr) {
          text_rdr->setColor(color);
          mesh->setIsDirty(true);
        }
      }
    }
  }

  Window* RenderManager::getWindow()
  {
    return _window.get();
  }

  void RenderManager::updateComponentRenderingInfo()
  {
    _rendering_info.mesh = nullptr;
    _rendering_info.textures = _texture_manager->getTextures();
    _rendering_info.skybox_name = _texture_manager->getSkyboxTexture();
    _rendering_info.terrain_name = _texture_manager->getTerrainTexture();
    _rendering_info.water_name = _texture_manager->getWaterTexture();
    _rendering_info.sun_light = _light_manager->getSunLight();
    _rendering_info.point_lights = _light_manager->getPointLights();
    _rendering_info.spot_lights = _light_manager->getSpotLights();
    _rendering_info.characters = _font_manager->getCharacters();
    _rendering_info.face = _font_manager->getFace();
    _rendering_info.atlas_width = _font_manager->getAtlasWidth();
    _rendering_info.atlas_height = _font_manager->getAtlasHeight();
    _rendering_info.light_buffer = _light_buffers[_renderer->getCurrentFrameIndex()];
  }

  ComponentRenderingInfo& RenderManager::getComponentRenderingInfo(Mesh * mesh)
  {
    _rendering_info.mesh = mesh;
    return _rendering_info;
  }

  void RenderManager::updateRendererInfo(glm::mat4 const& camera_view)
  {
    _renderer_info.mesh = nullptr;
    _renderer_info.camera = getCamera();
    _renderer_info.camera_view = camera_view;
    _renderer_info.sun_light = _light_manager->getSunLight();
    _renderer_info.point_lights = _light_manager->getPointLights();
    _renderer_info.spot_lights = _light_manager->getSpotLights();
    _renderer_info.elapsed_time = _elapsed_time;
    _renderer_info.stage_flag_bits = 1 | 16;
    _renderer_info.normal_debug = ConfigManagerLocator::get()->normalDebug();
    _renderer_info.has_alpha_blend = false;
  }

  RendererInfo& RenderManager::getRendererInfo(Mesh * mesh, glm::mat4 const& camera_view)
  {
    _renderer_info.mesh = mesh;
    _renderer_info.camera_view = camera_view;
    return _renderer_info;
  }

  Buffer RenderManager::getLightBuffer()
  {
    return _light_buffers[_renderer->getCurrentFrameIndex()];
  }
}
