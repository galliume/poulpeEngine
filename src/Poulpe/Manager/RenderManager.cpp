#include "RenderManager.hpp"

#include "Poulpe/Component/RenderComponent.hpp"
#include "Poulpe/Component/AnimationComponent.hpp"


#include "Poulpe/Renderer/Vulkan/EntityFactory.hpp"

namespace Poulpe
{    
    RenderManager::RenderManager(Window* window)
    {
        m_Window = std::unique_ptr< Window>(window);
        m_EntityManager = std::make_unique<Poulpe::EntityManager>();
        m_ComponentManager = std::make_unique<Poulpe::ComponentManager>();
        m_LightManager = std::make_unique<Poulpe::LightManager>();
        m_TextureManager = std::make_unique<Poulpe::TextureManager>();
        
        m_Renderer = std::make_unique<Poulpe::VulkanAdapter>(
          m_Window.get(),
          m_EntityManager.get(),
          m_ComponentManager.get(),
          m_LightManager.get(),
          m_TextureManager.get());

        m_ConfigManager = std::make_unique<Poulpe::ConfigManager>();
        m_AudioManager = std::make_unique<Poulpe::AudioManager>();
        m_ShaderManager = std::make_unique<Poulpe::ShaderManager>();
        m_SpriteAnimationManager = std::make_unique<Poulpe::SpriteAnimationManager>();
        m_DestroyManager = std::make_unique<Poulpe::DestroyManager>();
        m_Camera = std::make_unique<Poulpe::Camera>();

        m_Camera->init();
        m_Renderer->init();
        m_Renderer->addCamera(m_Camera.get());
        m_DestroyManager->setRenderer(m_Renderer->rdr());
        m_DestroyManager->addMemoryPool(m_Renderer->rdr()->getDeviceMemoryPool());

        nlohmann::json appConfig = m_ConfigManager->appConfig();
        if (appConfig["defaultLevel"].empty()) {
            PLP_WARN("defaultLevel conf not set.");
        }
        
        m_CurrentLevel = static_cast<std::string>(appConfig["defaultLevel"]);

        Poulpe::Locator::getInputManager()->init(appConfig["input"]);

        //@todo, those managers should not have the usage of the renderer...
        m_TextureManager->addRenderer(m_Renderer.get());
        m_EntityManager->addRenderer(m_Renderer.get());
        m_ShaderManager->addRenderer(m_Renderer.get());
        Poulpe::Locator::getInputManager()->addRenderer(m_Renderer.get());
        Poulpe::Locator::getInputManager()->setCamera(m_Camera.get());
        //end @todo
    }

    void RenderManager::cleanUp()
    {
        m_DestroyManager->cleanEntities(*m_EntityManager->getEntities());

        auto hud = m_EntityManager->getHUD();
        m_DestroyManager->cleanEntities(*hud);

        m_DestroyManager->cleanShaders(m_ShaderManager->getShaders()->shaders);
        m_DestroyManager->cleanTextures(m_TextureManager->getTextures());
        m_DestroyManager->cleanTexture(m_TextureManager->getSkyboxTexture());
        m_AudioManager->clear();
        m_SpriteAnimationManager->clear();
        m_TextureManager->clear();
        m_EntityManager->clear();
        m_ShaderManager->clear();
        m_Renderer->clear();
        //m_DestroyManager->CleanDeviceMemory();
        //m_Renderer->Rdr()->InitMemoryPool();
        m_ComponentManager->clear();
    }

    void RenderManager::init()
    {
        //@todo clean all thoses
        TinyObjLoader::m_TinyObjMaterials.clear();

        if (m_Refresh) {
            m_Renderer->stopRendering();
            m_Renderer->rdr()->waitIdle();
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

        prepareEntity();
        prepareSkybox();
        prepareHUD();
    }

    void RenderManager::loadData(std::string const & level)
    {
        nlohmann::json appConfig = m_ConfigManager->appConfig();
        std::string_view threadQueueName{ "loading" };

        auto const levelData = m_ConfigManager->loadLevelData(level);
        m_TextureManager->addConfig(m_ConfigManager->texturesConfig());

        std::function<void()> entityFutures = m_EntityManager->load(levelData);
        std::function<void()> textureFuture = m_TextureManager->load();

        std::string const  sb = (m_CurrentSkybox.empty()) ? static_cast<std::string>(appConfig["defaultSkybox"])
            : m_CurrentSkybox;

        std::function<void()> skyboxFuture = m_TextureManager->loadSkybox(sb);
        std::function<void()> shaderFuture = m_ShaderManager->load(m_ConfigManager->shaderConfig());

        Poulpe::Locator::getThreadPool()->submit(threadQueueName, entityFutures);
        Poulpe::Locator::getThreadPool()->submit(threadQueueName, textureFuture);
        Poulpe::Locator::getThreadPool()->submit(threadQueueName, skyboxFuture);
        Poulpe::Locator::getThreadPool()->submit(threadQueueName, shaderFuture);

        while (!m_TextureManager->isTexturesLoadingDone()) {
            //PLP_WARN("loading {}", m_TextureManager->isTexturesLoadingDone());
        }
        while (!m_TextureManager->isSkyboxLoadingDone()) {
            //PLP_WARN("loading {}", m_TextureManager->isSkyboxLoadingDone());
        }
        while (!m_ShaderManager->isLoadingDone()) {
            //PLP_WARN("loading {}", m_ShaderManager->isLoadingDone());
        }
        while (!m_EntityManager->IsLoadingDone()) {
            //PLP_WARN("loading {}", m_EntityManager->IsLoadingDone());
        }
        setIsLoaded();
    }

    void RenderManager::draw()
    {
        m_Renderer->draw();
    }

    template <typename T>
    T lerp(T const& startValue, T const& endValue, float const& t) {
      return ((1.0f - t) * startValue) + (t * endValue);
    }

    void RenderManager::renderScene(float const deltaTime)
    {
        for (auto& entity : *m_EntityManager->getEntities()) {
            auto* mesh = entity->getMesh();
            if (!mesh->hasAnimation()) continue;
            auto compo = m_ComponentManager->GetComponent<AnimationComponent>(entity->getID());
            if (compo) compo->visit(deltaTime, mesh);
        }

        m_Renderer->renderScene();

        if (m_Refresh) {
            m_Renderer->setDrawBbox(m_ShowBbox);
            init();
            m_Refresh = false;
            m_ShowBbox = false;
        }
    }

    void RenderManager::refresh(uint32_t levelIndex, bool showBbox, std::string_view skybox)
    {
        m_CurrentLevel = m_ConfigManager->listLevels().at(levelIndex);
        m_CurrentSkybox = skybox;
        m_IsLoaded = false;
        m_Refresh = true;
        m_ShowBbox = showBbox;
    }

    void RenderManager::prepareEntity()
    {
      auto* basicRdrImpl = new Basic(m_Renderer.get(),
          m_TextureManager.get(),
          m_LightManager.get());

      for (auto& entity : *m_EntityManager->getEntities()) {
        m_ComponentManager->addComponent<RenderComponent>(entity->getID(), basicRdrImpl);
      }

      //@todo temp until lua scripting
      class AnimImpl : public IVisitor
      {
      public:
          AnimImpl(VulkanAdapter* adapter)
          {
              m_Adapter = adapter;
          }

          void visit(float const deltaTime, Mesh* mesh) override
          {
              for (auto& ubo : mesh->getData()->m_Ubos) {

                  //mesh->getData()->m_CurrentPos.x -= 0.0001;
                  //mesh->getData()->m_CurrentPos.y -= 0.0001;
                  //mesh->getData()->m_CurrentPos.z -= 0.0001;
                  //if (!reverse) {
                  //    animationDuration -= 1.f;
                  //}
                  //else {
                  //    animationDuration += 1.f;
                  //}

                  //if (0 > animationDuration) {
                  //    animationDuration = 0.f;
                  //    reverse = true;
                  //}
                  //else if (3 < animationDuration) {
                  //    animationDuration = 3.f;
                  //    reverse = false;
                  //}

                  //auto scale = lerp(startScale, endScale, elapsedTime / animationDuration);

                  //elapsedTime += deltaTime;

                  //ubo.model = glm::mat4(1.0f);
                  //ubo.model = glm::translate(ubo.model, mesh->getData()->m_OriginPos);
                  //ubo.model = glm::scale(ubo.model, mesh->getData()->m_OriginScale);

                  auto angle = deltaTime * (std::rand() % 20);

                  ubo.model = glm::rotate(ubo.model, glm::radians(angle), mesh->getData()->m_OriginPos);

                  m_Adapter->rdr()->updateUniformBuffer(mesh->getUniformBuffers()->at(0), &mesh->getData()->m_Ubos);
              }

          }

          VulkanAdapter* m_Adapter;
          float animationDuration = 3.f;
          float elapsedTime = 0.f;
          bool reverse = false;
          glm::vec3 startScale = glm::vec3(0.001, 0.001, 0.001);
          glm::vec3 endScale = glm::vec3(0.12, 0.12, 0.12);
      };

      auto* animImpl = new AnimImpl(m_Renderer.get());

      for (auto& entity : *m_EntityManager->getEntities()) {
        if (entity->getMesh()->isDirty()) {
          auto comp = m_ComponentManager->GetComponent<RenderComponent>(entity->getID());
          entity->accept(0, comp);
        }

        //@todo temp until lua scripting
        if (entity->getMesh()->hasAnimation()) {
            m_ComponentManager->addComponent<AnimationComponent>(entity->getID(), animImpl);
        }
      }
    }

    void RenderManager::prepareHUD()
    {
        //auto* gridMesh = new Mesh();
        //auto* gridEntity = new Entity();
        //auto* gridRdrImpl = new Grid(m_Renderer.get(), m_TextureManager.get());

        //gridEntity->setMesh(gridMesh);
        //m_EntityManager->addHUD(gridEntity);
        //m_ComponentManager->addComponent<RenderComponent>(gridEntity->getID(), gridRdrImpl);

        auto* chMesh = new Mesh();
        auto* chEntity = new Entity();
        auto* chRdrImpl = new Crosshair(m_Renderer.get(), m_TextureManager.get());

        chEntity->setMesh(chMesh);
        m_EntityManager->addHUD(chEntity);
        m_ComponentManager->addComponent<RenderComponent>(chEntity->getID(), chRdrImpl);

        for (auto& entity : *m_EntityManager->getHUD()) {
          if (entity->getMesh()->isDirty()) {
            auto comp = m_ComponentManager->GetComponent<RenderComponent>(entity->getID());
            entity->accept(0, comp);
          }
        }
    }

    void RenderManager::prepareSkybox()
    {
        auto* skyboxMesh = new Mesh();
        auto* skyboxEntity = new Entity();
        auto* skyRdrImpl = new Skybox(m_Renderer.get(), m_TextureManager.get());

        skyboxEntity->setMesh(skyboxMesh);
        m_EntityManager->setSkybox(skyboxEntity);
        m_ComponentManager->addComponent<RenderComponent>(skyboxEntity->getID(), skyRdrImpl);

        auto skybox = m_EntityManager->getSkybox();
        if (skybox && skybox->getMesh()->isDirty()) {
          auto comp = m_ComponentManager->GetComponent<RenderComponent>(skybox->getID());
          skybox->accept(0, comp);
        }
    }
}
