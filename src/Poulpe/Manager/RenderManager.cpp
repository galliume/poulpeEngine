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
        m_Window = std::unique_ptr<Window>(window);
        m_ComponentManager = std::make_unique<ComponentManager>();
        m_LightManager = std::make_unique<LightManager>();
        m_TextureManager = std::make_unique<TextureManager>();
        m_EntityManager = std::make_unique<EntityManager>(
          m_ComponentManager.get(),
          m_LightManager.get(),
          m_TextureManager.get());
        
        m_Renderer = std::make_unique<Renderer>(
          m_Window.get(),
          m_EntityManager.get(),
          m_ComponentManager.get(),
          m_LightManager.get(),
          m_TextureManager.get());

        m_AudioManager = std::make_unique<AudioManager>();
        m_ShaderManager = std::make_unique<ShaderManager>();
        m_DestroyManager = std::make_unique<DestroyManager>();
        m_Camera = std::make_unique<Camera>();

        m_Camera->init();
        m_Renderer->init();
        m_Renderer->addCamera(m_Camera.get());
        m_DestroyManager->setRenderer(m_Renderer.get());
        m_DestroyManager->addMemoryPool(m_Renderer->getDeviceMemoryPool());

        nlohmann::json appConfig = Poulpe::Locator::getConfigManager()->appConfig();
        if (appConfig["defaultLevel"].empty()) {
            PLP_WARN("defaultLevel conf not set.");
        }
        
        m_CurrentLevel = static_cast<std::string>(appConfig["defaultLevel"]);

        Locator::getInputManager()->init(appConfig["input"]);

        //@todo, those managers should not have the usage of the renderer...
        m_TextureManager->addRenderer(m_Renderer.get());
        m_EntityManager->addRenderer(m_Renderer.get());
        m_ShaderManager->addRenderer(m_Renderer.get());
        
        Locator::getInputManager()->setCamera(m_Camera.get());
        //end @todo
    }

    void RenderManager::cleanUp()
    {
        //m_DestroyManager->cleanEntities(*m_EntityManager->getEntities());

        //auto hud = m_EntityManager->getHUD();
        //m_DestroyManager->cleanEntities(*hud);

        m_DestroyManager->cleanShaders(m_ShaderManager->getShaders()->shaders);
        m_DestroyManager->cleanTextures(m_TextureManager->getTextures());
        m_DestroyManager->cleanTexture(m_TextureManager->getSkyboxTexture());
        m_AudioManager->clear();
        m_TextureManager->clear();
        m_EntityManager->clear();
        m_ShaderManager->clear();
        m_Renderer->clear();
        //m_DestroyManager->CleanDeviceMemory();
        //m_Renderer->InitMemoryPool();
        m_ComponentManager->clear();
    }

    void RenderManager::init()
    {
        //@todo clean all thoses

        if (m_Refresh) {
            m_Renderer->stopRendering();
            m_Renderer->waitIdle();
            cleanUp();
            m_Renderer->recreateSwapChain();
            setIsLoaded(false);
            m_Refresh = false;
            m_EntityManager->initWorldGraph();
        }
       
        auto * const configManager = Poulpe::Locator::getConfigManager();

        nlohmann::json const& appConfig = configManager->appConfig();
        nlohmann::json const& textureConfig = configManager->texturesConfig();

        m_AudioManager->init();
        m_AudioManager->load(configManager->soundConfig());

        loadData(m_CurrentLevel);

        if (static_cast<bool>(appConfig["ambientMusic"])) {
          m_AudioManager->startAmbient();
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
      m_CurrentLevel = Poulpe::Locator::getConfigManager()->listLevels().at(levelIndex);
      m_CurrentSkybox = skybox;
      m_IsLoaded = false;
      m_Refresh = true;
      m_ShowBbox = showBbox;
    }

    void RenderManager::renderScene(std::chrono::duration<float> deltaTime)
    {
      m_Renderer->renderScene();

      //@todo animate light
      //m_LightManager->animateAmbientLight(deltaTime);
      auto* worldNode = m_EntityManager->getWorldNode();

      std::ranges::for_each(worldNode->getChildren(), [&](const auto& leafNode) {
        std::ranges::for_each(leafNode->getChildren(), [&](const auto& entityNode) {
          
          auto const& entity = entityNode->getEntity();

          auto* meshComponent = m_ComponentManager->getComponent<MeshComponent>(entity->getID());
          auto mesh = meshComponent->template hasImpl<Mesh>();
        
          if (mesh) {
            auto basicRdrImpl = m_ComponentManager->getComponent<RenderComponent>(entity->getID());
            if (mesh->isDirty() && basicRdrImpl) {
              basicRdrImpl->visit(deltaTime, mesh);
            }

            auto* animationComponent = m_ComponentManager->getComponent<AnimationComponent>(entity->getID());
            if (animationComponent) {
              animationComponent->visit(deltaTime, mesh);
            }

            auto* boneAnimationComponent = m_ComponentManager->getComponent<BoneAnimationComponent>(entity->getID());
            if (boneAnimationComponent) {
              //boneAnimationComponent->visit(deltaTime, mesh);
              //mesh->setIsDirty(true);

              /*if (mesh->hasBufferStorage()) {
                auto buffer{ mesh->getStorageBuffers()->at(0) };
                ObjectBuffer* objectBuffer = mesh->getObjectBuffer();

                objectBuffer->boneIds = {};
                objectBuffer->weights = {};

                m_Renderer->updateStorageBuffer(buffer, *objectBuffer);
              }*/
            }
          }
        });
      });

      if (m_Refresh) {
        m_Renderer->setDrawBbox(m_ShowBbox);
        init();
        m_Refresh = false;
        m_ShowBbox = false;
      }

    }

    void RenderManager::loadData(std::string const & level)
    {
      auto * const configManager = Poulpe::Locator::getConfigManager();

      nlohmann::json const& appConfig = configManager->appConfig();
      std::string_view threadQueueName{ "loading" };

      auto const& levelData = configManager->loadLevelData(level);
      m_TextureManager->addConfig(configManager->texturesConfig());

      std::string const sb{ (m_CurrentSkybox.empty()) ? static_cast<std::string>(appConfig["defaultSkybox"])
        : m_CurrentSkybox };

      std::latch count_down{ 3 };

      std::jthread textures(std::move(std::bind(m_TextureManager->load(), std::ref(count_down))));
      textures.detach();
      std::jthread skybox(std::move(std::bind(m_TextureManager->loadSkybox(sb), std::ref(count_down))));
      skybox.detach();
      std::jthread shaders(std::move(std::bind(m_ShaderManager->load(configManager->shaderConfig()), std::ref(count_down))));
      shaders.detach();
      count_down.wait();

      setIsLoaded();

      std::jthread entities(std::move(m_EntityManager->load(levelData)));
      entities.detach();
    }

    void RenderManager::prepareHUD()
    {
      //@todo clean raw pointers
      auto* gridEntity{ new Entity() };
      auto* gridMesh{ new Mesh() };
      auto gridRdrImpl{ RendererFactory::create<Grid>() };
      gridRdrImpl->init(m_Renderer.get(), m_TextureManager.get(), nullptr);

      auto renderGridComponent{ m_ComponentManager->addComponent<RenderComponent>(gridEntity->getID(), gridRdrImpl) };
      auto deltaTime{ std::chrono::duration<float, std::milli>(0) };
      renderGridComponent.visit(deltaTime, gridMesh);
      m_ComponentManager->addComponent<MeshComponent>(gridEntity->getID(), gridMesh);

      auto* chEntity{ new Entity() };
      auto* chMesh{ new Mesh() };
      auto chRdrImpl{ RendererFactory::create<Crosshair>() };
      chRdrImpl->init(m_Renderer.get(), m_TextureManager.get(), nullptr);

      auto renderCrosshairComponent{ m_ComponentManager->addComponent<RenderComponent>(chEntity->getID(), chRdrImpl) };
      renderCrosshairComponent.visit(deltaTime, chMesh);
      m_ComponentManager->addComponent<MeshComponent>(chEntity->getID(), chMesh);

      m_EntityManager->addHUD(gridEntity);
      m_EntityManager->addHUD(chEntity);
    }

    void RenderManager::prepareSkybox()
    {
      auto* skyboxEntity{ new Entity() };
      auto* skyboxMesh{new Mesh()};
      skyboxMesh->setHasShadow(false);
      skyboxMesh->setIsIndexed(false);

      auto* skyRdrImpl{ new Skybox() };
      skyRdrImpl->init(m_Renderer.get(), m_TextureManager.get(), nullptr);

      auto renderComponent{ m_ComponentManager->addComponent<RenderComponent>(skyboxEntity->getID(), skyRdrImpl) };
      auto deltaTime = std::chrono::duration<float, std::milli>(0);
      renderComponent.visit(deltaTime, skyboxMesh);

      m_ComponentManager->addComponent<MeshComponent>(skyboxEntity->getID(), skyboxMesh);
      m_EntityManager->setSkybox(skyboxEntity);
    }
}
