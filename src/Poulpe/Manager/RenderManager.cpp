#include "RenderManager.hpp"

#include "AudioManager.hpp"
#include "ComponentManager.hpp"
#include "EntityManager.hpp"
#include "LightManager.hpp"
#include "ShaderManager.hpp"
#include "TextureManager.hpp"

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

    _camera->init();
    _renderer->init();
    _renderer->addCamera(_camera.get());
    _destroy_manager->setRenderer(_renderer.get());
    _destroy_manager->addMemoryPool(_renderer->getDeviceMemoryPool());

    nlohmann::json appConfig = Poulpe::Locator::getConfigManager()->appConfig();
    if (appConfig["defaultLevel"].empty()) {
        PLP_WARN("defaultLevel conf not set.");
    }

    _current_level = static_cast<std::string>(appConfig["defaultLevel"]);

    Locator::getInputManager()->init(appConfig["input"]);

    //@todo, those managers should not have the usage of the renderer...
    _texture_manager->addRenderer(_renderer.get());
    _entity_manager->addRenderer(_renderer.get());
    _shader_manager->addRenderer(_renderer.get());

    Locator::getInputManager()->setCamera(_camera.get());
    //end @todo
  }

  void RenderManager::cleanUp()
  {
    //_destroy_manager->cleanEntities(*_entity_manager->getEntities());

    //auto hud = _entity_manager->getHUD();
    //_destroy_manager->cleanEntities(*hud);

    _destroy_manager->cleanShaders(_shader_manager->getShaders()->shaders);
    _destroy_manager->cleanTextures(_texture_manager->getTextures());
    _destroy_manager->cleanTexture(_texture_manager->getSkyboxTexture());
    _audio_manager->clear();
    _texture_manager->clear();
    _entity_manager->clear();
    _shader_manager->clear();
    _renderer->clear();
    //_destroy_manager->CleanDeviceMemory();
    //_renderer->InitMemoryPool();
    _component_manager->clear();
  }

  void RenderManager::init()
  {
    //@todo clean all thoses
    if (_refresh) {
        _renderer->waitIdle();
        cleanUp();
        setIsLoaded(false);
        _refresh = false;
        _entity_manager->initWorldGraph();
    }

    auto * const configManager = Poulpe::Locator::getConfigManager();

    nlohmann::json const& appConfig = configManager->appConfig();
    nlohmann::json const& textureConfig = configManager->texturesConfig();

    _audio_manager->init();
    _audio_manager->load(configManager->soundConfig());

    loadData(_current_level);

    if (static_cast<bool>(appConfig["ambientMusic"])) {
      _audio_manager->startAmbient();
    }

    prepareSkybox();
    prepareHUD();
  }

  template <typename T>
  T lerp(T const& startValue, T const& endValue, float const& t) {
    return ((1.0f - t) * startValue) + (t * endValue);
  }

  void RenderManager::refresh(uint32_t levelIndex, bool showBbox, std::string_view skybox)
  {
    _current_level = Poulpe::Locator::getConfigManager()->listLevels().at(levelIndex);
    _current_skybox = skybox;
    _is_loaded = false;
    _refresh = true;
  }

  void RenderManager::renderScene(std::chrono::duration<float> deltaTime)
  {
    _renderer->renderScene();

    //@todo animate light
    //_light_manager->animateAmbientLight(deltaTime);
    auto* worldNode = _entity_manager->getWorldNode();

    std::ranges::for_each(worldNode->getChildren(), [&](const auto& leafNode) {
      std::ranges::for_each(leafNode->getChildren(), [&](const auto& entityNode) {

        auto const& entity = entityNode->getEntity();

        auto* mesh_component = _component_manager->get<MeshComponent>(entity->getID());
        auto mesh = mesh_component->template has<Mesh>();

        if (mesh) {
          auto basicRdrImpl = _component_manager->get<RenderComponent>(entity->getID());
          if (mesh->isDirty() && basicRdrImpl) {
            (*basicRdrImpl)(deltaTime, mesh);
          }

          //auto* animationComponent = _component_manager->get<AnimationComponent>(entity->getID());
          //if (animationComponent) {
          //  (*animationComponent)(deltaTime, mesh);
          //}

          //auto* boneAnimationComponent = _component_manager->get<BoneAnimationComponent>(entity->getID());
          //if (boneAnimationComponent) {
            //boneAnimationComponent->visit(deltaTime, mesh);
            //mesh->setIsDirty(true);

            /*if (mesh->hasBufferStorage()) {
              auto buffer{ mesh->getStorageBuffers()->at(0) };
              ObjectBuffer* objectBuffer = mesh->getObjectBuffer();

              objectBuffer->boneIds = {};
              objectBuffer->weights = {};

              _renderer->updateStorageBuffer(buffer, *objectBuffer);
            }*/
          //}
        }
      });
    });

    if (_refresh) {
      init();
      _refresh = false;
    }
  }

  void RenderManager::loadData(std::string const & level)
  {
    auto * const configManager = Poulpe::Locator::getConfigManager();

    nlohmann::json const& appConfig = configManager->appConfig();
    std::string_view threadQueueName{ "loading" };

    auto const& levelData = configManager->loadLevelData(level);
    _texture_manager->addConfig(configManager->texturesConfig());

    std::string const sb{ (_current_skybox.empty()) ? static_cast<std::string>(appConfig["defaultSkybox"])
      : _current_skybox };

    std::latch count_down{ 3 };

    std::jthread textures(std::move(std::bind(_texture_manager->load(), std::ref(count_down))));
    textures.detach();
    std::jthread skybox(std::move(std::bind(_texture_manager->loadSkybox(sb), std::ref(count_down))));
    skybox.detach();
    std::jthread shaders(std::move(std::bind(_shader_manager->load(configManager->shaderConfig()), std::ref(count_down))));
    shaders.detach();
    count_down.wait();

    setIsLoaded();

    std::jthread entities(std::move(_entity_manager->load(levelData)));
    entities.detach();
  }

  void RenderManager::prepareHUD()
  {
    auto const deltaTime{ std::chrono::duration<float, std::milli>(0) };

    auto gridEntity = std::make_unique<Entity>();
    auto gridMesh = std::make_unique<Mesh>();
    auto gridRdrImpl{ RendererFactory::create<Grid>() };
    gridRdrImpl->init(_renderer.get(), _texture_manager.get(), nullptr);

    (*gridRdrImpl)(deltaTime, gridMesh.get());

    _component_manager->add<RenderComponent>(gridEntity->getID(), std::move(gridRdrImpl));
    _component_manager->add<MeshComponent>(gridEntity->getID(), std::move(gridMesh));

    auto chEntity = std::make_unique<Entity>();
    auto chMesh = std::make_unique<Mesh>();
    auto chRdrImpl{ RendererFactory::create<Crosshair>() };
    chRdrImpl->init(_renderer.get(), _texture_manager.get(), nullptr);

    (*chRdrImpl)(deltaTime, chMesh.get());
    _component_manager->add<RenderComponent>(chEntity->getID(), std::move(chRdrImpl));
    _component_manager->add<MeshComponent>(chEntity->getID(), std::move(chMesh));

    _entity_manager->addHUD(std::move(gridEntity));
    _entity_manager->addHUD(std::move(chEntity));
  }

  void RenderManager::prepareSkybox()
  {
    auto skyboxEntity = std::make_unique<Entity>();
    auto skyboxMesh = std::make_unique<Mesh>();
    skyboxMesh->setHasShadow(false);
    skyboxMesh->setIsIndexed(false);

    auto skyRdrImpl{ RendererFactory::create<Skybox>() };
    skyRdrImpl->init(_renderer.get(), _texture_manager.get(), nullptr);

    auto deltaTime = std::chrono::duration<float, std::milli>(0);
    (*skyRdrImpl)(deltaTime, skyboxMesh.get());

    _component_manager->add<RenderComponent>(skyboxEntity->getID(), std::move(skyRdrImpl));
    _component_manager->add<MeshComponent>(skyboxEntity->getID(), std::move(skyboxMesh));
    _entity_manager->setSkybox(std::move(skyboxEntity));
  }
}
