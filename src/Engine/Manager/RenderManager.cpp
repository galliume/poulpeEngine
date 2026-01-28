module Engine.Managers.RenderManager;

import std;

import Engine.Animation.AnimationScript;

import Engine.Component.Components;
import Engine.Component.Entity;
import Engine.Component.Mesh;

import Engine.Core.AnimationTypes;
import Engine.Core.Camera;
import Engine.Core.Constants;
import Engine.Core.Json;
import Engine.Core.LightTypes;
import Engine.Core.Logger;
import Engine.Core.MaterialTypes;
import Engine.Core.GLM;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;

import Engine.GUI.Window;

import Engine.Managers.ComponentManager;
import Engine.Managers.ConfigManagerLocator;
import Engine.Managers.InputManagerLocator;

import Engine.Renderer;
import Engine.Renderer.RendererComponent;
import Engine.Renderer.RendererComponentTypes;
import Engine.Renderer.RendererComponentFactory;

import Engine.Renderer.Vulkan.Skybox;
import Engine.Renderer.Vulkan.Terrain;
import Engine.Renderer.Vulkan.Text;
import Engine.Renderer.Vulkan.Water;

import Engine.Utils.ScopedTimer;

namespace Poulpe
{
  RenderManager::RenderManager(Window* const window)
  {
    _window = std::unique_ptr<Window>(window);
    _cameras.emplace_back(std::make_unique<Camera>());
    _renderer = std::make_unique<Renderer>(_window->getGlfwWindow());

    _component_manager = std::make_unique<ComponentManager>();
    _light_manager = std::make_unique<LightManager>();
    _texture_manager = std::make_unique<TextureManager>();

    _audio_manager = std::make_unique<AudioManager>();
    _shader_manager = std::make_unique<ShaderManager>();
    _destroy_manager = std::make_unique<DestroyManager>();

    _renderer->init();
    _destroy_manager->setRenderer(_renderer.get());
    _destroy_manager->addMemoryPool(_renderer->getAPI()->getDeviceMemoryPool());

    _light_buffers.resize(_renderer->getMaxFramesInFlight());

    _renderer->getAPI()->startCopyBuffer(_renderer->getCurrentFrameIndex());
    for (std::size_t i = 0; i < _light_buffers.size(); ++i) {
      LightObjectBuffer light_object_buffer{};
      _light_buffers[i] = _renderer->getAPI()->createStorageBuffers(light_object_buffer, _renderer->getCurrentFrameIndex());
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

    _renderer->getAPI()->endCopyBuffer(_renderer->getCurrentFrameIndex());
    
    _texture_manager->addTexture(atlas);

    //@todo, those managers should not have the usage of the renderer...
    _entity_manager->addRenderer(_renderer.get());
    _shader_manager->addRenderer(_renderer.get());

    InputManagerLocator::get()->setCamera(getCamera());

    //end @todo

    //_audio_manager->startAmbient(1);
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
    
    auto const& shadowConfig { appConfig["shadow_resolution"] };
    _renderer->setShadowMapResolution(shadowConfig["width"].get<std::uint32_t>());

    if (appConfig["defaultLevel"].empty()) {
      Logger::warn("defaultLevel conf not set.");
    }

    _current_level = appConfig["defaultLevel"].get<std::string>();

    loadData(_current_level);

    if (appConfig["ambientMusic"].get<bool>()) {
      _audio_manager->startAmbient();
    }

    if (!getCamera()->isInit()) {
      auto const& lvl_config{ configManager->lvlConfig() };
      auto const& camera{ lvl_config["camera"] };
      glm::vec3 const start_pos = {
        camera["position"]["x"].template get<float>(),
        camera["position"]["y"].template get<float>(),
        camera["position"]["z"].template get<float>() };

      getCamera()->init(start_pos);
      getCamera()->forward();
      getCamera()->move();
    }

    auto const camera_view_matrix = (_current_camera == std::to_underlying(CameraType::THIRD_PERSON))
      ? getCamera()->getThirdPersonView(_player_manager->getPosition())
      : getCamera()->getView();

    updateComponentRenderingInfo();
    updateRendererInfo(camera_view_matrix);

    _renderer->getAPI()->startCopyBuffer(_renderer->getCurrentFrameIndex());
    prepareSkybox();
    prepareTerrain();
    prepareWater();

    FontManager::Text text {
      .id = 0,
      .name = "_plp_title",
      .text = "Poulpe Engine ™",
      .position = glm::vec3(0.0f, 100.0f, 0.0f),
      .color = glm::vec3(1.0f, 0.2f, 0.6f),
      .scale = 0.2f,
      .flat = false
    };
    addText(text);
    _renderer->getAPI()->endCopyBuffer(_renderer->getCurrentFrameIndex());
    
    //prepareHUD();
  }

  template <typename T>
  T lerp(T const& startValue, T const& endValue, float const& t) {
    return ((1.0f - t) * startValue) + (t * endValue);
  }

  void RenderManager::renderScene(double const delta_time)
  {
    {
      auto * const config_manager { ConfigManagerLocator::get() };
      auto const & root_path { config_manager->rootPath() };

      auto const camera_index { config_manager->getCameraIndex() };
      if (_current_camera != camera_index) {
        _current_camera = camera_index;
        InputManagerLocator::get()->setCamera(getCamera());
      }

      InputManagerLocator::get()->processGamepad(_player_manager.get(), delta_time);

      //@todo animate light
      //_light_manager->animateAmbientLight(delta_time);
      auto const camera_view_matrix = (_current_camera == std::to_underlying(CameraType::THIRD_PERSON))
        ? getCamera()->getThirdPersonView(_player_manager->getPosition())
        : getCamera()->getView();
      auto const& perspective { _renderer->getPerspective() };
      //std::lock_guard<std::shared_mutex> guard(_entity_manager->lockWorldNode());

      _light_manager->computeCSM(camera_view_matrix, perspective);
      //Logger::debug("x {} y {} z {}", camera_pos.x, camera_pos.y, camera_pos.z);

      updateComponentRenderingInfo();
      updateRendererInfo(camera_view_matrix);

      //auto renderer_info { getRendererInfo() };
      _renderer->getAPI()->startCopyBuffer(_renderer->getCurrentFrameIndex());

      std::future<void> async_skybox_render;
      std::future<void> async_water_render;
      std::future<void> async_terrain_render;
      std::vector<std::future<void>> async_texts_render{};
      async_texts_render.reserve(_entity_manager->getTexts().size());
      std::vector<std::future<void>> async_entities_render;
      std::vector<std::future<void>> async_transparent_entities_render;

      glm::mat4 const vp { perspective * camera_view_matrix };
      auto const frustum_planes { getCamera()->getFrustumPlanes(vp) };
      
      auto const& entities = _entity_manager->getEntities();
      auto const& transparent_entities = _entity_manager->getTransparentEntities();

      std::vector<IDType> sorted_entities{};
      std::vector<IDType> sorted_transparent_entities{};

      std::ranges::for_each(entities, [&](const auto& entity) {
        if (!isClipped(entity->getID(), frustum_planes)) {
          sorted_entities.emplace_back(entity->getID());
        }
      });

      std::ranges::for_each(transparent_entities, [&](const auto& entity) {
        if (!isClipped(entity->getID(), frustum_planes)) {
          sorted_transparent_entities.emplace_back(entity->getID());
        }
      });

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
          float const narrowed { static_cast<float>(getElapsedTime()) };
          std::uint32_t bitValue;
          std::memcpy(&bitValue, &narrowed, sizeof(float));

          mesh->setOptions(bitValue);
        }

        async_water_render = std::async(std::launch::deferred, [&]() {
          renderEntity(water_entity->getID(), delta_time);
        });
      }

      std::ranges::for_each(sorted_entities, [&](const auto& entity) {
        async_entities_render.push_back(std::async(std::launch::deferred, [&]() {
          renderEntity(entity, delta_time);
        }));
      });

      std::ranges::for_each(sorted_transparent_entities, [&](const auto& entity) {
        async_transparent_entities_render.push_back(std::async(std::launch::deferred, [&]() {
          renderEntity(entity, delta_time);
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
      _renderer->getAPI()->endCopyBuffer(_renderer->getCurrentFrameIndex());

      _renderer->start();
      _renderer->startShadowMap(SHADOW_TYPE::CSM);
      
      std::ranges::for_each(sorted_entities, [&](const auto& entity) {
        drawShadowMap(entity, SHADOW_TYPE::CSM, camera_view_matrix);
      });
      _renderer->endShadowMap(SHADOW_TYPE::CSM);
      
      
      _renderer->startShadowMap(SHADOW_TYPE::SPOT_LIGHT);
      
      std::ranges::for_each(sorted_entities, [&](const auto& entity) {
        drawShadowMap(entity, SHADOW_TYPE::SPOT_LIGHT, camera_view_matrix);
      });
      _renderer->endShadowMap(SHADOW_TYPE::SPOT_LIGHT);

      _renderer->startRender();

      if (skybox_entity != nullptr) {
        drawEntity(skybox_entity->getID(), camera_view_matrix);
      }

      if (terrain_entity != nullptr) {
        drawEntity(terrain_entity->getID(), camera_view_matrix);
      }

      std::ranges::for_each(sorted_entities, [&](const auto& entity) {
        drawEntity(entity, camera_view_matrix, false);
      });
      std::ranges::for_each(sorted_transparent_entities, [&](const auto& entity) {
        drawEntity(entity, camera_view_matrix, true);
      });

      if (water_entity != nullptr) {
        drawEntity(water_entity->getID(), camera_view_matrix);
      }

      std::ranges::for_each(_entity_manager->getTexts(), [&](auto const& text_entity) {
        drawEntity(text_entity->getID(), camera_view_matrix, true);
      });

      _renderer->endRender();
      _renderer->submit();

      LightObjectBuffer light_object_buffer{};
      //light_object_buffer.lights[0] = _light_manager->getSpotLights()[0];
      light_object_buffer.lights[0] = _light_manager->getSunLight();
      light_object_buffer.lights[1] = _light_manager->getPointLights()[1];
      light_object_buffer.lights[2] = _light_manager->getPointLights()[0];

      _renderer->getAPI()->startCopyBuffer(_renderer->getCurrentFrameIndex());

      _renderer->getAPI()->updateStorageBuffer(
        _light_buffers.at(_renderer->getCurrentFrameIndex()), light_object_buffer, _renderer->getCurrentFrameIndex());
      _renderer->getAPI()->endCopyBuffer(_renderer->getCurrentFrameIndex());

      if (_player_manager->hasMoved() && _current_camera == std::to_underlying(CameraType::THIRD_PERSON)) {
        auto pos { _player_manager->getPosition() };
        pos.y += 10;
        pos.z -= 10;
        getCamera()->setPos(pos);
      }

      _player_manager->reset();
    }
  }

  bool RenderManager::isClipped(
    IDType const entity_id,
    std::vector<glm::vec4> const& frustum_planes)
  {
    auto* mesh_component { _component_manager->get<MeshComponent>(entity_id) };
    auto mesh { mesh_component->template has<Mesh>() };

    if (mesh->hasBbox()) {
      glm::vec3 bmin { glm::vec3(glm::vec4(mesh->getData()->_bbox_min, 1.0f)) };
      glm::vec3 bmax { glm::vec3(glm::vec4(mesh->getData()->_bbox_max, 1.0f)) };
      auto inv = mesh->getData()->_inverse_transform_matrix;

      for (auto const& K : frustum_planes) {
        auto K_local { inv * K };
        glm::vec3 pos_side { bmin };
        if (K_local.x >= 0) pos_side.x = bmax.x;
        if (K_local.y >= 0) pos_side.y = bmax.y;
        if (K_local.z >= 0) pos_side.z = bmax.z;

        if (glm::dot(K_local, glm::vec4(pos_side, 1.0f)) < 0) {
          //Logger::debug("clipped : {}", mesh->getName());
          return false;//@todo to improve
        }
      }
    }
    return false;
  }

  void RenderManager::renderEntity(
    IDType const entity_id,
    double const delta_time)
  {
    auto* mesh_component { _component_manager->get<MeshComponent>(entity_id) };
    auto mesh { mesh_component->template has<Mesh>() };
    auto rdr_impl { _component_manager->get<RendererComponent>(entity_id) };

    if (mesh->isRoot()) {
      AnimationInfo animation_info {
        .delta_time = delta_time,
        .data = mesh->getData()
      };
      if (mesh->getName() == _player_manager->getPlayerName()) {
        _player_manager->setPlayerId(entity_id);
        animation_info.looping = false;
        if (_current_camera == std::to_underlying(CameraType::THIRD_PERSON) && !getCamera()->isInit()) {
          getCamera()->init(_player_manager->getPosition());
          getCamera()->forward();
          getCamera()->move();
        }
      }

      auto* animation_component { _component_manager->get<AnimationComponent>(entity_id) };
      if (animation_component) {
        (*animation_component)(animation_info);
        mesh->setIsDirty(true);
      }

      auto* boneAnimationComponent { _component_manager->get<BoneAnimationComponent>(entity_id) };
      if (boneAnimationComponent) {
        (*boneAnimationComponent)(animation_info);
        mesh->setIsDirty(true);
      }

      for (auto const& id : mesh->getChildren()) {
        auto* child_mesh_component { _component_manager->get<MeshComponent>(id) };
        auto child_mesh { child_mesh_component->template has<Mesh>() };
        if (child_mesh && boneAnimationComponent) {
          AnimationInfo child_animation_info {
            .delta_time = delta_time,
            .data = child_mesh->getData(),
            .looping = animation_info.looping
          };
          (*boneAnimationComponent)(child_animation_info);
          child_mesh->setIsDirty(true);
          // auto child_rdr_impl { _component_manager->get<RendererComponent>(id) };
          // (*child_rdr_impl)(_renderer.get(), getComponentRenderingInfo(child_mesh));
        }
      }
    }

    if (mesh && rdr_impl) {
      if (mesh->isDirty()) {
        (*rdr_impl)(_renderer.get(), getComponentRenderingInfo(mesh));
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

    auto rdr_impl { _component_manager->get<RendererComponent>(entity_id) };

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
    auto * const config_manager { ConfigManagerLocator::get() };
    auto const& app_config { config_manager->appConfig() };

    auto const& lvl_data { config_manager->loadLevelData(level) };

    if (lvl_data.contains("hasFog") && lvl_data["hasFog"]) {
      _env_options |= PLP_ENV_OPTIONS::HAS_FOG;
    }
    if (lvl_data.contains("player")) {
      _player_manager = std::make_unique<PlayerManager>(
      _component_manager.get(),
      lvl_data["player"].get<std::string>());

      _cameras.emplace_back(std::make_unique<Camera>());
    }
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

    std::jthread entities(_entity_manager->load(lvl_data));
    entities.detach();
    setIsLoaded();

  }

  void RenderManager::prepareSkybox()
  {
    auto entity = std::make_shared<Entity>();
    auto mesh = std::make_unique<Mesh>();
    mesh->setName("skybox");
    mesh->isSkybox(true);
    
    material_t material{};
    mesh->addMaterial(material);

    _env_options |= PLP_ENV_OPTIONS::HAS_IRRADIANCE;//@todo finish IBL with real map

    auto rdr_impl{ RendererComponentFactory::create<Skybox>() };
    //(*rdr_impl)(_renderer.get(), getComponentRenderingInfo(mesh.get()));

    _component_manager->add<RendererComponent>(entity->getID(), std::move(rdr_impl));
    _component_manager->add<MeshComponent>(entity->getID(), std::move(mesh));
    _entity_manager->setSkybox(entity);
  }

  void RenderManager::prepareTerrain()
  {
    auto entity = std::make_shared<Entity>();
    auto mesh = std::make_unique<Mesh>();
    mesh->setHasShadow(false);
    mesh->setIsIndexed(false);
    mesh->setShaderName("terrain");
    
    material_t material{};
    mesh->addMaterial(material);

    auto rdr_impl{ RendererComponentFactory::create<Terrain>() };
    //(*rdr_impl)(_renderer.get(), getComponentRenderingInfo(mesh.get()));

    _component_manager->add<RendererComponent>(entity->getID(), std::move(rdr_impl));
    _component_manager->add<MeshComponent>(entity->getID(), std::move(mesh));
    _entity_manager->setTerrain(entity);
  }

  void RenderManager::prepareWater()
  {
    auto entity = std::make_shared<Entity>();
    auto mesh = std::make_unique<Mesh>();
    mesh->setHasShadow(false);
    mesh->setIsIndexed(false);
    mesh->setShaderName("water");
    mesh->setName("_plp_water");

    material_t material{};
    mesh->addMaterial(material);

    auto rdr_impl{ RendererComponentFactory::create<Water>() };
    //(*rdr_impl)(_renderer.get(), getComponentRenderingInfo(mesh.get()));

    _component_manager->add<RendererComponent>(entity->getID(), std::move(rdr_impl));
    _component_manager->add<MeshComponent>(entity->getID(), std::move(mesh));
    _entity_manager->setWater(entity);
  }

  void RenderManager::addText(FontManager::Text & text)
  {
    auto entity = std::make_shared<Entity>();
    auto mesh = std::make_unique<Mesh>();
    mesh->setHasShadow(false);
    mesh->setIsIndexed(false);
    mesh->setShaderName("text");
    mesh->setName("_plp_text_" + std::to_string(entity->getID()));
    mesh->setRoot();

    material_t material{};
    material.double_sided = true;
    material.alpha_mode = 1.0;
    mesh->addMaterial(material);
    
    auto rdr_impl{ RendererComponentFactory::create<Text>() };
    rdr_impl->setText(text.text);
    rdr_impl->setPosition(text.position);
    rdr_impl->setColor(text.color);
    rdr_impl->setScale(text.scale);
    rdr_impl->setFlat(text.flat);

    //(*rdr_impl)(_renderer.get(), getComponentRenderingInfo(mesh.get()));

    _component_manager->add<RendererComponent>(entity->getID(), std::move(rdr_impl));
    _component_manager->add<MeshComponent>(entity->getID(), std::move(mesh));

    text.id = entity->getID();
   
    _entity_manager->addText(entity);
  }

  void RenderManager::updateText(IDType const entity_id, std::string const& text)
  {
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
  void RenderManager::updateTextColor(IDType const entity_id, glm::vec3 const& color)
  {
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
    _renderer_info.env_options = _env_options;
    return _renderer_info;
  }

  Buffer RenderManager::getLightBuffer()
  {
    return _light_buffers[_renderer->getCurrentFrameIndex()];
  }
}
