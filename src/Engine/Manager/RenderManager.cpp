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
    _renderer = std::make_unique<Renderer>();
    _renderer->init(_window->getGlfwWindow());

    _component_manager = std::make_unique<ComponentManager>();

    _component_manager->registerComponent<MeshComponent>();
    _component_manager->registerComponent<RendererComponent>();
    _component_manager->registerComponent<BoneAnimationComponent>();
    _component_manager->registerComponent<AnimationComponent>();

    _light_manager = std::make_unique<LightManager>();
    _texture_manager = std::make_unique<TextureManager>(*_renderer);

    _audio_manager = std::make_unique<AudioManager>();
    _destroy_manager = std::make_unique<DestroyManager>();
    
    _destroy_manager->setRenderer(_renderer.get());
    _destroy_manager->addMemoryPool(_renderer->getAPI()->getDeviceMemoryPool());
    
    _light_buffers.resize(_renderer->getMaxFramesInFlight());
    
    _renderer->getAPI()->startCopyBuffer(_renderer->getCurrentFrameIndex());
    for (std::size_t i = 0; i < _light_buffers.size(); ++i) {
      LightObjectBuffer light_object_buffer{};
      _light_buffers[i] = _renderer->getAPI()->createStorageBuffers(light_object_buffer, _renderer->getCurrentFrameIndex());
    }
    
    _font_manager = std::make_unique<FontManager>(*_renderer);
    auto atlas { _font_manager->load() };
    
    _renderer->getAPI()->endCopyBuffer(_renderer->getCurrentFrameIndex());
    
    _texture_manager->addTexture(atlas);
    
    //@todo, those managers should not have the usage of the renderer...
    _shader_manager = std::make_unique<ShaderManager>(*_renderer);

    _entity_manager = std::make_unique<EntityManager>(
      *_component_manager,
      *_texture_manager,
      _light_buffers.at(0)
    );

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

    //_component_manager->clear();
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

    updateRendererContext(camera_view_matrix);

    _renderer->getAPI()->startCopyBuffer(_renderer->getCurrentFrameIndex());
    prepareSkybox();
    prepareTerrain();
    prepareWater();

    FontManager::Text text {
      .id = 0,
      .name = "_plp_title",
      .text = "₼ Vampyropoda-Studio ₼",
      .position = glm::vec3(0.0f, 100.0f, 0.0f),
      .color = glm::vec3(0.0f, 0.95, 1.f),
      .scale = 0.2f,
      .flat = false
    };
    addText(text);
    _renderer->getAPI()->endCopyBuffer(_renderer->getCurrentFrameIndex());
  }

  template <typename T>
  T lerp(T const& startValue, T const& endValue, float const& t) {
    return ((1.0f - t) * startValue) + (t * endValue);
  }

  void RenderManager::renderScene(double const delta_time)
  {
    {
      auto * const config_manager { ConfigManagerLocator::get() };
      if (_current_camera != config_manager->getCameraIndex()) {
        _current_camera = config_manager->getCameraIndex();
        InputManagerLocator::get()->setCamera(getCamera());
      }

      auto const frame_context { buildFrameContext(
        config_manager->rootPath(),
        _current_camera,
        delta_time) };

      InputManagerLocator::get()->processGamepad(_player_manager.get(), frame_context.delta_time);

      _light_manager->computeCSM(frame_context.camera_view_matrix, frame_context.perspective);

      updateRendererContext(frame_context.camera_view_matrix);

      _renderer->getAPI()->startCopyBuffer(_renderer->getCurrentFrameIndex());

      //@todo improve this draft for simple shader hot reload
      if (config_manager->reloadShaders()) {
        std::filesystem::path p = std::filesystem::current_path();
        auto cmd{ frame_context.root_path + "/bin/shaders_compil.sh" };
        std::system(cmd.c_str());

        std::latch count_down{ 1 };
        _shader_manager->load(config_manager->shaderConfig())(count_down);

        config_manager->setReloadShaders(false);
      }

      updateEntities(frame_context);
      renderShadowmap(frame_context);
      renderEntities(frame_context);

      _renderer->submit();

      updateBuffers();
      updatePlayer();

      _renderer->getAPI()->collectGarbage();
    }
  }

  void RenderManager::updatePlayer()
  {
    if (_player_manager->hasMoved() && _current_camera == std::to_underlying(CameraType::THIRD_PERSON)) {
      auto pos { _player_manager->getPosition() };
      pos.y += 10;
      pos.z -= 10;
      getCamera()->setPos(pos);
    }

    _player_manager->reset();
  }

  void RenderManager::updateEntities(FrameContext const& frame_context)
  {
    _tasks.clear();

    auto const optional_tasks {
      (frame_context.skybox_entity ? 1u : 0u)
      + (frame_context.terrain_entity ? 1u : 0u)
      + (frame_context.water_entity ? 1u : 0u) };

    _tasks.reserve(
      optional_tasks
      + static_cast<std::uint32_t>(frame_context.visible_ids.size())
      + static_cast<std::uint32_t>(frame_context.transparent_ids.size())
      + static_cast<std::uint32_t>(frame_context.texts.size()));

    auto const delta_time { frame_context.delta_time };
    auto const camera_view_matrix { frame_context.camera_view_matrix };

    if (frame_context.skybox_entity != nullptr) {
      auto const entity_id = frame_context.skybox_entity->getID();
      _tasks.push_back(std::async(std::launch::async, [this, entity_id, delta_time, camera_view_matrix]() {
        renderEntity(entity_id, delta_time, camera_view_matrix);
      }));
    }

    if (frame_context.terrain_entity != nullptr) {
      auto const entity_id { frame_context.terrain_entity->getID() };
      _tasks.push_back(std::async(std::launch::async, [this, entity_id, delta_time, camera_view_matrix]() {
        renderEntity(entity_id, delta_time, camera_view_matrix);
      }));
    }

    if (frame_context.water_entity != nullptr) {
      auto const entity_id { frame_context.water_entity->getID() };
      auto* mesh_component { _component_manager->get<MeshComponent>(entity_id) };
      auto mesh { mesh_component->template has<Mesh>() };

      if (mesh) {
        float const narrowed { static_cast<float>(getElapsedTime()) };
        std::uint32_t bitValue;
        std::memcpy(&bitValue, &narrowed, sizeof(float));

        mesh->setOptions(bitValue);
      }

      _tasks.push_back(std::async(std::launch::async, [this, entity_id, delta_time, camera_view_matrix]() {
        renderEntity(entity_id, delta_time, camera_view_matrix);
      }));
    }

    std::ranges::for_each(frame_context.visible_ids, [&](const auto& entity) {
      _tasks.push_back(std::async(std::launch::async, [this, entity, delta_time, camera_view_matrix]() {
        renderEntity(entity, delta_time, camera_view_matrix);
      }));
    });

    std::ranges::for_each(frame_context.transparent_ids, [&](const auto& entity) {
      _tasks.push_back(std::async(std::launch::async, [this, entity, delta_time, camera_view_matrix]() {
        renderEntity(entity, delta_time, camera_view_matrix);
      }));
    });

    std::ranges::for_each(frame_context.texts, [&](auto const& entity) {
      auto const entity_id { entity->getID() };
      _tasks.push_back(std::async(std::launch::async, [this, entity_id, delta_time, camera_view_matrix]() {
        renderEntity(entity_id, delta_time, camera_view_matrix);
      }));
    });

    std::ranges::for_each(_tasks, &std::future<void>::wait);
  }

  void RenderManager::renderShadowmap(FrameContext const& frame_context)
  {
    _renderer->start();
    _renderer->startShadowMap(SHADOW_TYPE::CSM);

    std::ranges::for_each(frame_context.visible_ids, [&](const auto& entity) {
      drawShadowMap(entity, SHADOW_TYPE::CSM, frame_context.camera_view_matrix);
    });

    _renderer->endShadowMap(SHADOW_TYPE::CSM);
    _renderer->startShadowMap(SHADOW_TYPE::SPOT_LIGHT);

    std::ranges::for_each(frame_context.visible_ids, [&](const auto& entity) {
      drawShadowMap(entity, SHADOW_TYPE::SPOT_LIGHT, frame_context.camera_view_matrix);
    });

    _renderer->endShadowMap(SHADOW_TYPE::SPOT_LIGHT);
  }

  void RenderManager::renderEntities(FrameContext const& frame_context)
  {
    _renderer->startRender();

    if (frame_context.skybox_entity != nullptr) {
      drawEntity(frame_context.skybox_entity->getID(), frame_context.camera_view_matrix);
    }

    if (frame_context.terrain_entity != nullptr) {
      drawEntity(frame_context.terrain_entity->getID(), frame_context.camera_view_matrix);
    }

    std::ranges::for_each(frame_context.visible_ids, [&](const auto& entity) {
      drawEntity(entity, frame_context.camera_view_matrix, false);
    });

    std::ranges::for_each(frame_context.transparent_ids, [&](const auto& entity) {
      drawEntity(entity, frame_context.camera_view_matrix, true);
    });

    if (frame_context.water_entity != nullptr) {
      drawEntity(frame_context.water_entity->getID(), frame_context.camera_view_matrix);
    }

    std::ranges::for_each(frame_context.texts, [&](auto const& text_entity) {
      drawEntity(text_entity->getID(), frame_context.camera_view_matrix, true);
    });

    _renderer->getAPI()->endCopyBuffer(_renderer->getCurrentFrameIndex());
    _renderer->endRender();
  }

  bool RenderManager::isClipped(
    IDType const entity_id,
    std::span<glm::vec4 const> frustum_planes) const
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
    double const delta_time,
    glm::mat4 const& camera_view_matrix)
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
        (*rdr_impl)(*_renderer.get(), *mesh, getRendererContext(camera_view_matrix));
      }
    }
  }

  void RenderManager::drawEntity(
    IDType const entity_id,
    glm::mat4 const& camera_view_matrix,
    bool const has_alpha_blend)
  {
    auto* mesh_component { _component_manager->get<MeshComponent>(entity_id) };
    auto mesh { mesh_component->template has<Mesh>() };
    auto rdr_impl { _component_manager->get<RendererComponent>(entity_id) };

    if (mesh && rdr_impl) {
      RendererContext renderer_info { getRendererContext(camera_view_matrix) };
      renderer_info.stage_flag_bits = rdr_impl->getShaderStageFlags();
      renderer_info.has_alpha_blend = has_alpha_blend;
      _renderer->draw(*mesh, renderer_info);
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
      RendererContext renderer_context { getRendererContext(camera_view_matrix) };
      renderer_context.stage_flag_bits = rdr_impl->getShaderStageFlags();
      renderer_context.has_alpha_blend = has_alpha_blend;

      _renderer->drawShadowMap(*mesh,renderer_context, shadow_type);
    }
  }

  void RenderManager::updateBuffers()
  {
    LightObjectBuffer light_object_buffer{};
    //light_object_buffer.lights[0] = _light_manager->getSpotLights()[0];
    light_object_buffer.lights[0] = _light_manager->getSunLight();
    light_object_buffer.lights[1] = _light_manager->getPointLights()[1];
    light_object_buffer.lights[2] = _light_manager->getPointLights()[0];

    _renderer->getAPI()->startCopyBuffer(_renderer->getCurrentFrameIndex());

    _renderer->getAPI()->updateStorageBuffer<LightObjectBuffer>(
      _light_buffers.at(_renderer->getCurrentFrameIndex()), light_object_buffer, _renderer->getCurrentFrameIndex());
    _renderer->getAPI()->endCopyBuffer(_renderer->getCurrentFrameIndex());
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

    std::jthread textures(std::bind(_texture_manager->load(*_renderer), std::ref(count_down)));
    std::jthread skybox(std::bind(_texture_manager->loadSkybox(sb, *_renderer), std::ref(count_down)));
    std::jthread shaders(std::bind(_shader_manager->load(config_manager->shaderConfig()), std::ref(count_down)));
    count_down.wait();

    std::jthread entities(_entity_manager->load(*_renderer, lvl_data));
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
    auto mesh_component { _component_manager->get<MeshComponent>(entity_id) };
    if (mesh_component) {
      auto mesh { mesh_component->has<Mesh>() };
      auto rdr_impl { _component_manager->get<RendererComponent>(entity_id) };
      if (rdr_impl) {
        auto text_rdr { rdr_impl->has<Text>() };

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
    auto mesh_component { _component_manager->get<MeshComponent>(entity_id) };
    if (mesh_component) {
      auto mesh { mesh_component->has<Mesh>() };
      auto rdr_impl { _component_manager->get<RendererComponent>(entity_id) };
      if (rdr_impl) {
        auto text_rdr { rdr_impl->has<Text>() };

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

  void RenderManager::updateRendererContext(glm::mat4 const& camera_view)
  {
    _rendering_context.camera = getCamera();
    _rendering_context.camera_view = camera_view;
    _rendering_context.textures = &_texture_manager->getTextures();
    _rendering_context.skybox = &_texture_manager->getSkyboxTexture();
    _rendering_context.terrain = &_texture_manager->getTerrainTexture();
    _rendering_context.water = &_texture_manager->getWaterTexture();
    _rendering_context.sun_light = _light_manager->getSunLight();
    _rendering_context.point_lights = _light_manager->getPointLights();
    _rendering_context.spot_lights = _light_manager->getSpotLights();
    _rendering_context.characters = _font_manager->getCharacters();
    _rendering_context.face = _font_manager->getFace();
    _rendering_context.atlas_width = _font_manager->getAtlasWidth();
    _rendering_context.atlas_height = _font_manager->getAtlasHeight();
    _rendering_context.elapsed_time = _elapsed_time;
    _rendering_context.stage_flag_bits = 1 | 16;
    _rendering_context.normal_debug = ConfigManagerLocator::get()->normalDebug();
    _rendering_context.has_alpha_blend = false;
    _rendering_context.light_buffer = _light_buffers[_renderer->getCurrentFrameIndex()];
  }

  RendererContext& RenderManager::getRendererContext(glm::mat4 const& camera_view)
  {
    _rendering_context.camera_view = camera_view;
    _rendering_context.env_options = _env_options;
    return _rendering_context;
  }

  Buffer RenderManager::getLightBuffer()
  {
    return _light_buffers[_renderer->getCurrentFrameIndex()];
  }


  FrameContext RenderManager::buildFrameContext(
    std::string const& root_path,
    std::uint8_t const camera_index,
    double const delta_time) const
  {
    auto const& perspective { _renderer->getPerspective() };
    auto const camera_view_matrix =
      (_current_camera == std::to_underlying(CameraType::THIRD_PERSON))
        ? getCamera()->getThirdPersonView(_player_manager->getPosition())
        : getCamera()->getView();

    auto const vp { perspective * camera_view_matrix };
    auto const frustum_planes { getCamera()->getFrustumPlanes(vp) };
    auto const visible_ids { get_visible_ids(_entity_manager->getEntities(), frustum_planes) };
    auto const transparent_ids { get_visible_ids(_entity_manager->getTransparentEntities(), frustum_planes) };

    return {
      .delta_time = delta_time,
      .root_path = root_path,
      .camera_index = camera_index,
      .camera_view_matrix = camera_view_matrix,
      .perspective = perspective,
      .frustum_planes = std::move(frustum_planes),
      .visible_ids = std::move(visible_ids),
      .transparent_ids = std::move(transparent_ids),
      .skybox_entity = _entity_manager->getSkybox(),
      .terrain_entity = _entity_manager->getTerrain(),
      .water_entity = _entity_manager->getWater(),
      .texts = _entity_manager->getTexts()
    };
  }
}
