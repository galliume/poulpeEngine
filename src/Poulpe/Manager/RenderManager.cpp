module;

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

#include <nlohmann/json_fwd.hpp>

#include <algorithm>
#include <latch>
#include <filesystem>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <thread>

module Poulpe.Managers.RenderManager;

import Poulpe.Animation.AnimationScript;
import Poulpe.Animation.AnimationTypes;
import Poulpe.Component.Camera;
import Poulpe.Component.Components;
import Poulpe.Component.Entity;
import Poulpe.Core.Logger;
import Poulpe.GUI.Window;
import Poulpe.Managers.ComponentManager;
import Poulpe.Managers.ConfigManagerLocator;
import Poulpe.Managers.InputManagerLocator;
import Poulpe.Renderer;
import Poulpe.Renderer.Renderers;
import Poulpe.Renderer.RendererComponent;
import Poulpe.Renderer.RendererComponentTypes;
import Poulpe.Renderer.RendererComponentFactory;
import Poulpe.Renderer.Vulkan.Mesh;

namespace Poulpe
{
  RenderManager::RenderManager(Window* const window)
  {
    _window = std::unique_ptr<Window>(window);
    _renderer = std::make_unique<Renderer>(_window.get());

    _component_manager = std::make_unique<ComponentManager>();
    _light_manager = std::make_unique<LightManager>();
    _texture_manager = std::make_unique<TextureManager>();
    _entity_manager = std::make_unique<EntityManager>(
      _component_manager.get(),
      _light_manager.get(),
      _texture_manager.get());

    _audio_manager = std::make_unique<AudioManager>();
    _shader_manager = std::make_unique<ShaderManager>();
    _destroy_manager = std::make_unique<DestroyManager>();
    _camera = std::make_unique<Camera>();

    _renderer->init();
    _destroy_manager->setRenderer(_renderer.get());
    _destroy_manager->addMemoryPool(_renderer->getAPI()->getDeviceMemoryPool());

    _font_manager = std::make_unique<FontManager>();
    _font_manager->addRenderer(_renderer.get());
    auto atlas = _font_manager->load();

    _texture_manager->addTexture(atlas);

    //@todo, those managers should not have the usage of the renderer...
    _entity_manager->addRenderer(_renderer.get());
    _shader_manager->addRenderer(_renderer.get());

    InputManagerLocator::get()->setCamera(_camera.get());
    //end @todo
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
    }

    prepareSkybox();
    prepareTerrain();
    prepareWater();

    FontManager::Text text {
      .name = "_plp_title",
      .text = "@ € $ 0 7 é è ù ü ä ö π ∞ β Æ ‰ Ü Γ Đ Ặ Ω ! ₲ ◀ MrPoulpe ▶™",
      .position = glm::vec3(-200.0f, -100.0f, 0.0f), //@todo fix this -
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
    RendererInfo renderer_info {
      .mesh = nullptr,
      .camera = getCamera(),
      .sun_light = _light_manager->getSunLight(),
      .point_lights = _light_manager->getPointLights(),
      .spot_lights = _light_manager->getSpotLights(),
      .elapsed_time = _elapsed_time,
      .stage_flag_bits = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      .normal_debug = false
    };

    ComponentRenderingInfo rendering_info {
      .mesh = nullptr,
      .textures = _texture_manager->getTextures(),
      .skybox_name = _texture_manager->getSkyboxTexture(),
      .terrain_name = _texture_manager->getTerrainTexture(),
      .water_name = _texture_manager->getWaterTexture(),
      .sun_light = _light_manager->getSunLight(),
      .point_lights = _light_manager->getPointLights(),
      .spot_lights = _light_manager->getSpotLights(),
      .characters = _font_manager->getCharacters(),
      .face = _font_manager->getFace(),
      .atlas_width = _font_manager->getAtlasWidth(),
      .atlas_height = _font_manager->getAtlasHeight()
    };

    {
      std::lock_guard<std::shared_mutex> guard(_entity_manager->lockWorldNode());

      auto * const config_manager = ConfigManagerLocator::get();

      //@todo improve this draft for simple shader hot reload
      if (config_manager->reloadShaders()) {
        std::filesystem::path p = std::filesystem::current_path();
        auto cmd{ p.string() + "/bin/shaders_compil.sh" };
        std::system(cmd.c_str());

        std::latch count_down{ 1 };
        _shader_manager->load(config_manager->shaderConfig())(count_down);

        config_manager->setReloadShaders(false);
      }

      _renderer->startRender();

      auto water_entity = _entity_manager->getWater();
      if (water_entity != nullptr) {
        auto* mesh_component = _component_manager->get<MeshComponent>(water_entity->getID());
        auto mesh = mesh_component->template has<Mesh>();
        if (mesh) {
          auto rdr_impl = _component_manager->get<RendererComponent>(water_entity->getID());
          renderer_info.stage_flag_bits = rdr_impl->getShaderStageFlags();
          renderer_info.mesh = mesh;
          if (mesh->isDirty() && rdr_impl) {
            (*rdr_impl)(_renderer.get(), rendering_info);
          }
          _renderer->draw(renderer_info);
        }
      }

      std::ranges::for_each(_entity_manager->getTexts(), [&](auto const& data_entity) {
        auto* mesh_component = _component_manager->get<MeshComponent>(data_entity->getID());
        auto mesh = mesh_component->template has<Mesh>();

        if (mesh) {
          auto rdr_impl = _component_manager->get<RendererComponent>(data_entity->getID());
          
          rendering_info.mesh = mesh;
          renderer_info.mesh = mesh;
          renderer_info.stage_flag_bits = rdr_impl->getShaderStageFlags();

          if (mesh->isDirty() && rdr_impl) {
            (*rdr_impl)(_renderer.get(), rendering_info);
          }
          _renderer->draw(renderer_info);
        }
      });

      auto* world_node = _entity_manager->getWorldNode();

      std::ranges::for_each(world_node->getChildren(), [&](const auto& leaf_node) {
        std::ranges::for_each(leaf_node->getChildren(), [&](const auto& entity_node) {

          auto const& data_entity = entity_node->getEntity();

          auto* mesh_component = _component_manager->get<MeshComponent>(data_entity->getID());
          auto mesh = mesh_component->template has<Mesh>();

          if (mesh) {
            rendering_info.mesh = mesh;
            renderer_info.mesh = mesh;

            AnimationInfo const animation_info {
              .delta_time = delta_time,
              .data = mesh->getData()
            };
            //if (mesh->hasBufferStorage()) {
            //  auto objectBuffer = mesh->getObjectBuffer();
            //  objectBuffer->point_lights[0] = _light_manager->getPointLights().at(0);

            //  _renderer->getAPI()->updateStorageBuffer(mesh->getStorageBuffers()->at(0), *objectBuffer);
            //}

            auto rdr_impl = _component_manager->get<RendererComponent>(data_entity->getID());
            if (mesh->isDirty() && rdr_impl) {
              (*rdr_impl)(_renderer.get(), rendering_info);
            }

            auto* animation_component = _component_manager->get<AnimationComponent>(leaf_node->getEntity()->getID());
            if (animation_component && leaf_node->isLoaded()) {
              (*animation_component)(animation_info);
            }

            auto* boneAnimationComponent = _component_manager->get<BoneAnimationComponent>(leaf_node->getEntity()->getID());
            if (boneAnimationComponent) {

              (*boneAnimationComponent)(animation_info);
              //mesh->setIsDirty(true);
              /*if (mesh->hasBufferStorage()) {
                auto buffer{ mesh->getStorageBuffers()->at(0) };
                ObjectBuffer* objectBuffer = mesh->getObjectBuffer();

                objectBuffer->boneIds = {};
                objectBuffer->weights = {};

                _renderer->updateStorageBuffer(buffer, *objectBuffer);
              }*/
            }

            if (rdr_impl) {
              renderer_info.stage_flag_bits = rdr_impl->getShaderStageFlags();
              renderer_info.mesh = mesh;
              _renderer->draw(renderer_info);
            }
          }
        });
      });

      _renderer->endRender();
    }

    //if (_refresh) {
    //  init();
    //  _refresh = false;
    //}
  }
  // void RenderManager::renderScene()
  // {
  //   _renderer->renderScene(_component_manager);
  // }

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
    //entities.detach();
  }

  void RenderManager::prepareHUD()
  {
    auto grid_entity = std::make_unique<Entity>();
    auto grid_mesh = std::make_unique<Mesh>();
    auto grid_rdr_impl{ RendererComponentFactory::create<Grid>() };

    ComponentRenderingInfo rendering_info {
      .mesh = grid_mesh.get(),
      .textures = _texture_manager->getTextures(),
      .skybox_name = _texture_manager->getSkyboxTexture(),
      .terrain_name = _texture_manager->getTerrainTexture(),
      .water_name = _texture_manager->getWaterTexture(),
      .sun_light = _light_manager->getSunLight(),
      .point_lights = _light_manager->getPointLights(),
      .spot_lights = _light_manager->getSpotLights(),
      .characters = _font_manager->getCharacters(),
      .face = _font_manager->getFace(),
      .atlas_width = _font_manager->getAtlasWidth(),
      .atlas_height = _font_manager->getAtlasHeight()
    };

    (*grid_rdr_impl)(_renderer.get(), rendering_info);

    _component_manager->add<RendererComponent>(grid_entity->getID(), std::move(grid_rdr_impl));
    _component_manager->add<MeshComponent>(grid_entity->getID(), std::move(grid_mesh));

    auto crosshair_entity = std::make_unique<Entity>();
    auto mesh = std::make_unique<Mesh>();

    ComponentRenderingInfo crosshair_rendering_info {
      .mesh = mesh.get(),
      .textures = _texture_manager->getTextures(),
      .skybox_name = _texture_manager->getSkyboxTexture(),
      .terrain_name = _texture_manager->getTerrainTexture(),
      .water_name = _texture_manager->getWaterTexture(),
      .sun_light = _light_manager->getSunLight(),
      .point_lights = _light_manager->getPointLights(),
      .spot_lights = _light_manager->getSpotLights(),
      .characters = _font_manager->getCharacters(),
      .face = _font_manager->getFace(),
      .atlas_width = _font_manager->getAtlasWidth(),
      .atlas_height = _font_manager->getAtlasHeight()
    };

    auto crosshair_rdr_impl{ RendererComponentFactory::create<Crosshair>() }; 
    (*crosshair_rdr_impl)(_renderer.get(), crosshair_rendering_info);

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
    mesh->setHasShadow(false);
    mesh->setIsIndexed(false);

    ComponentRenderingInfo rendering_info {
      .mesh = mesh.get(),
      .textures = _texture_manager->getTextures(),
      .skybox_name = _texture_manager->getSkyboxTexture(),
      .terrain_name = _texture_manager->getTerrainTexture(),
      .water_name = _texture_manager->getWaterTexture(),
      .sun_light = _light_manager->getSunLight(),
      .point_lights = _light_manager->getPointLights(),
      .spot_lights = _light_manager->getSpotLights(),
      .characters = _font_manager->getCharacters(),
      .face = _font_manager->getFace(),
      .atlas_width = _font_manager->getAtlasWidth(),
      .atlas_height = _font_manager->getAtlasHeight()
    };

    auto rdr_impl{ RendererComponentFactory::create<Skybox>() };
    (*rdr_impl)(_renderer.get(), rendering_info);

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
    
    ComponentRenderingInfo rendering_info {
      .mesh = mesh.get(),
      .textures = _texture_manager->getTextures(),
      .skybox_name = _texture_manager->getSkyboxTexture(),
      .terrain_name = _texture_manager->getTerrainTexture(),
      .water_name = _texture_manager->getWaterTexture(),
      .sun_light = _light_manager->getSunLight(),
      .point_lights = _light_manager->getPointLights(),
      .spot_lights = _light_manager->getSpotLights(),
      .characters = _font_manager->getCharacters(),
      .face = _font_manager->getFace(),
      .atlas_width = _font_manager->getAtlasWidth(),
      .atlas_height = _font_manager->getAtlasHeight()
    };

    auto rdr_impl{ RendererComponentFactory::create<Terrain>() };
    (*rdr_impl)(_renderer.get(), rendering_info);

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

    ComponentRenderingInfo rendering_info {
      .mesh = mesh.get(),
      .textures = _texture_manager->getTextures(),
      .skybox_name = _texture_manager->getSkyboxTexture(),
      .terrain_name = _texture_manager->getTerrainTexture(),
      .water_name = _texture_manager->getWaterTexture(),
      .sun_light = _light_manager->getSunLight(),
      .point_lights = _light_manager->getPointLights(),
      .spot_lights = _light_manager->getSpotLights(),
      .characters = _font_manager->getCharacters(),
      .face = _font_manager->getFace(),
      .atlas_width = _font_manager->getAtlasWidth(),
      .atlas_height = _font_manager->getAtlasHeight()
    };
    
    auto rdr_impl{ RendererComponentFactory::create<Water>() };
    (*rdr_impl)(_renderer.get(), rendering_info);

    _component_manager->add<RendererComponent>(entity->getID(), std::move(rdr_impl));
    _component_manager->add<MeshComponent>(entity->getID(), std::move(mesh));
    _entity_manager->setWater(std::move(entity));
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
    
    ComponentRenderingInfo rendering_info {
      .mesh = mesh.get(),
      .textures = _texture_manager->getTextures(),
      .skybox_name = _texture_manager->getSkyboxTexture(),
      .terrain_name = _texture_manager->getTerrainTexture(),
      .water_name = _texture_manager->getWaterTexture(),
      .sun_light = _light_manager->getSunLight(),
      .point_lights = _light_manager->getPointLights(),
      .spot_lights = _light_manager->getSpotLights(),
      .characters = _font_manager->getCharacters(),
      .face = _font_manager->getFace(),
      .atlas_width = _font_manager->getAtlasWidth(),
      .atlas_height = _font_manager->getAtlasHeight()
    };

    (*rdr_impl)(_renderer.get(), rendering_info);

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
}
