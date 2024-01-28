#include "RenderManager.hpp"

#include "AudioManager.hpp"
#include "ComponentManager.hpp"
#include "ConfigManager.hpp"
#include "EntityManager.hpp"
#include "LightManager.hpp"
#include "ShaderManager.hpp"
#include "TextureManager.hpp"

#include "Poulpe/Component/AnimationComponent.hpp"
#include "Poulpe/Component/MeshComponent.hpp"
#include "Poulpe/Component/RenderComponent.hpp"

#include "Poulpe/Component/Renderer/RendererFactory.hpp"

#include "Poulpe/Core/TinyObjLoader.hpp"

#include "Poulpe/Renderer/Vulkan/Renderer.hpp"

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

        m_ConfigManager = std::make_unique<ConfigManager>();
        m_AudioManager = std::make_unique<AudioManager>();
        m_ShaderManager = std::make_unique<ShaderManager>();
        m_DestroyManager = std::make_unique<DestroyManager>();
        m_Camera = std::make_unique<Camera>();

        m_Camera->init();
        m_Renderer->init();
        m_Renderer->addCamera(m_Camera.get());
        m_DestroyManager->setRenderer(m_Renderer.get());
        m_DestroyManager->addMemoryPool(m_Renderer->getDeviceMemoryPool());

        nlohmann::json appConfig = m_ConfigManager->appConfig();
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

    void RenderManager::draw()
    {
      m_Renderer->draw();
    }

    void RenderManager::init()
    {
        //@todo clean all thoses
        TinyObjLoader::m_TinyObjMaterials.clear();

        if (m_Refresh) {
            m_Renderer->stopRendering();
            m_Renderer->waitIdle();
            cleanUp();
            m_Renderer->recreateSwapChain();
            setIsLoaded(false);
            m_Refresh = false;
            m_EntityManager->initWorldGraph();
        }
       
        nlohmann::json appConfig = m_ConfigManager->appConfig();
        nlohmann::json textureConfig = m_ConfigManager->texturesConfig();

        m_AudioManager->init();
        m_AudioManager->load(m_ConfigManager->soundConfig());

        loadData(m_CurrentLevel);

        if (static_cast<bool>(appConfig["ambientMusic"]))
            m_AudioManager->startAmbient();

        prepareSkybox();
        prepareHUD();
    }

    template <typename T>
    T lerp(T const& startValue, T const& endValue, float const& t) {
      return ((1.0f - t) * startValue) + (t * endValue);
    }

    void RenderManager::loadLevel(uint32_t levelIndex)
    {
      m_CurrentLevel = m_ConfigManager->listLevels().at(levelIndex);
      m_IsLoaded = false;
      m_Refresh = true;
    }

    void RenderManager::refresh(uint32_t levelIndex, bool showBbox, std::string_view skybox)
    {
      m_CurrentLevel = m_ConfigManager->listLevels().at(levelIndex);
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

        //@todo refactor with recursion when needed
        for (auto leafNode : worldNode->getChildren()) {
          for (auto entityNode : leafNode->getChildren()) {
            auto entity = entityNode->getEntity();
            auto* meshComponent = m_ComponentManager->getComponent<MeshComponent>(entity->getID());
            auto* animationComponent = m_ComponentManager->getComponent<AnimationComponent>(entity->getID());
            Mesh* mesh = meshComponent->hasImpl<Mesh>();

            if (animationComponent && mesh) {
              animationComponent->visit(deltaTime, mesh);
            }
          }
        }

        if (m_Refresh) {
            m_Renderer->setDrawBbox(m_ShowBbox);
            init();
            m_Refresh = false;
            m_ShowBbox = false;
        }
    }

    void RenderManager::loadData(std::string const & level)
    {
      nlohmann::json appConfig = m_ConfigManager->appConfig();
      std::string_view threadQueueName{ "loading" };

      auto const levelData = m_ConfigManager->loadLevelData(level);
      m_TextureManager->addConfig(m_ConfigManager->texturesConfig());

      std::function<void()> textureFuture = m_TextureManager->load();

      std::string const  sb = (m_CurrentSkybox.empty()) ? static_cast<std::string>(appConfig["defaultSkybox"])
        : m_CurrentSkybox;

      std::function<void()> entityFutures = m_EntityManager->load(levelData);
      std::function<void()> skyboxFuture = m_TextureManager->loadSkybox(sb);
      std::function<void()> shaderFuture = m_ShaderManager->load(m_ConfigManager->shaderConfig());

      Locator::getThreadPool()->submit(threadQueueName, textureFuture);
      Locator::getThreadPool()->submit(threadQueueName, skyboxFuture);
      Locator::getThreadPool()->submit(threadQueueName, shaderFuture);

      while (!m_TextureManager->isTexturesLoadingDone()) {
        //PLP_WARN("loading {}", m_TextureManager->isTexturesLoadingDone());
      }
      while (!m_TextureManager->isSkyboxLoadingDone()) {
        //PLP_WARN("loading {}", m_TextureManager->isSkyboxLoadingDone());
      }
      while (!m_ShaderManager->isLoadingDone()) {
        //PLP_WARN("loading {}", m_ShaderManager->isLoadingDone());
      }

      //while (!m_EntityManager->isLoadingDone()) {
      //    //PLP_WARN("loading {}", m_EntityManager->IsLoadingDone());
      //}
      setIsLoaded();
      Locator::getThreadPool()->submit(threadQueueName, entityFutures);
    }

    void RenderManager::prepareHUD()
    {
        auto* gridEntity = new Entity();
        auto* gridMesh = new Mesh();
        auto gridRdrImpl = RendererFactory::create<Grid>();
        gridRdrImpl->init(m_Renderer.get(), m_TextureManager.get(), nullptr);

        auto renderGridComponent = m_ComponentManager->addComponent<RenderComponent>(gridEntity->getID(), gridRdrImpl);
        auto deltaTime = std::chrono::duration<float, std::milli>(0);
        renderGridComponent.visit(deltaTime, gridMesh);
        m_ComponentManager->addComponent<MeshComponent>(gridEntity->getID(), gridMesh);

        auto* chEntity = new Entity();
        auto* chMesh = new Mesh();
        auto chRdrImpl = RendererFactory::create<Crosshair>();
        chRdrImpl->init(m_Renderer.get(), m_TextureManager.get(), nullptr);

        auto renderCrosshairComponent = m_ComponentManager->addComponent<RenderComponent>(chEntity->getID(), chRdrImpl);
        renderCrosshairComponent.visit(deltaTime, chMesh);
        m_ComponentManager->addComponent<MeshComponent>(chEntity->getID(), chMesh);

        m_EntityManager->addHUD(gridEntity);
        m_EntityManager->addHUD(chEntity);
    }

    void RenderManager::prepareSkybox()
    {
        auto* skyboxEntity = new Entity();
        auto* skyboxMesh = new Mesh();
        auto* skyRdrImpl = new Skybox();
        skyRdrImpl->init(m_Renderer.get(), m_TextureManager.get(), nullptr);

        auto renderComponent = m_ComponentManager->addComponent<RenderComponent>(skyboxEntity->getID(), skyRdrImpl);
        auto deltaTime = std::chrono::duration<float, std::milli>(0);
        renderComponent.visit(deltaTime, skyboxMesh);
        m_ComponentManager->addComponent<MeshComponent>(skyboxEntity->getID(), skyboxMesh);

        m_EntityManager->setSkybox(skyboxEntity);
    }
}
