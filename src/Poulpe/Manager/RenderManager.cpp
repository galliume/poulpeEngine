#include "RenderManager.hpp"

#include "Poulpe/Renderer/Vulkan/EntityFactory.hpp"

namespace Poulpe
{    
    RenderManager::RenderManager(
        std::shared_ptr<Window> window,
        std::shared_ptr<VulkanAdapter> renderer,
        std::shared_ptr<ConfigManager> configManager,
        std::shared_ptr<InputManager> inputManager,
        std::shared_ptr<AudioManager> audioManager,
        std::shared_ptr<TextureManager> textureManager,
        std::shared_ptr<EntityManager> entityManager,
        std::shared_ptr<ShaderManager> shaderManager,
        std::shared_ptr<SpriteAnimationManager> spriteAnimationManager,
        std::shared_ptr<Poulpe::DestroyManager> destroyManager,
        std::shared_ptr<Poulpe::Camera> camera,
        std::shared_ptr<Poulpe::CommandQueue> cmdQueue
    )
    {
        m_Window = window;
        m_Renderer = renderer;
        m_ConfigManager = configManager;
        m_InputManager = inputManager;
        m_AudioManager = audioManager;
        m_TextureManager = textureManager;
        m_EntityManager = entityManager;
        m_ShaderManager = shaderManager;
        m_SpriteAnimationManager = spriteAnimationManager;
        m_DestroyManager = destroyManager;
        m_Camera = camera;
        m_CommandQueue = cmdQueue;

        m_Camera->init();
        m_Renderer->init();
        m_Renderer->addCamera(m_Camera);
        m_DestroyManager->setRenderer(m_Renderer->rdr());
        m_DestroyManager->addMemoryPool(m_Renderer->rdr()->getDeviceMemoryPool());
        m_TextureManager->addConfig(m_ConfigManager->texturesConfig());

        nlohmann::json appConfig = m_ConfigManager->appConfig();
        if (appConfig["defaultLevel"].empty()) {
            PLP_WARN("defaultLevel conf not set.");
        }
        
        m_CurrentLevel = static_cast<std::string>(appConfig["defaultLevel"]);

        m_InputManager->init(appConfig["input"]);

        //@todo, those managers should not have the usage of the renderer...
        m_TextureManager->addRenderer(m_Renderer);
        m_EntityManager->addRenderer(m_Renderer);
        m_ShaderManager->addRenderer(m_Renderer);
        m_InputManager->addRenderer(m_Renderer);
        m_InputManager->setCamera(m_Camera);
        //end @todo
    }

    void RenderManager::cleanUp()
    {
        m_DestroyManager->cleanEntities(*m_EntityManager->getEntities());
        m_DestroyManager->cleanEntities(m_EntityManager->getHUD());
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
    }

    void RenderManager::init()
    {
        if (m_Refresh) {
            m_Renderer->rdr()->waitIdle();
            m_Renderer->stopRendering();
            m_Renderer->clearRendererScreen();
            cleanUp();
            m_Renderer->recreateSwapChain();
            setIsLoaded(false);
            m_Refresh = false;
        }
       
        nlohmann::json appConfig = m_ConfigManager->appConfig();
        nlohmann::json textureConfig = m_ConfigManager->texturesConfig();

        m_AudioManager->init();
        m_AudioManager->load(m_ConfigManager->soundConfig());

        std::vector<std::string> splashSprites{};
        for (auto& texture : textureConfig["splash"].items()) {
            splashSprites.emplace_back(texture.value());
        }

        m_SpriteAnimationManager->add("splashAnim", splashSprites);
        m_ShaderManager->addShader("splashscreen", "assets/shaders/spv/2d_vert.spv", "assets/shaders/spv/2d_frag.spv");

        prepareSplashScreen();

        if (static_cast<bool>(appConfig["splashScreenMusic"]))
            m_AudioManager->startSplash();

        std::thread loading([=, this]() {
            while (!isLoaded()) {
                m_Renderer->drawSplashScreen();
                std::this_thread::sleep_for(std::chrono::milliseconds(33));
            }
        });

        loadData(m_CurrentLevel);
        loading.join();
        m_Renderer->clearSplashScreen();

        m_AudioManager->stopSplash();

        if (static_cast<bool>(appConfig["ambientMusic"]))
            m_AudioManager->startAmbient();

        prepareEntity();
        prepareSkybox();
        prepareHUD();
    }

    void RenderManager::loadData(const std::string& level)
    {
        nlohmann::json appConfig = m_ConfigManager->appConfig();
        std::string_view threadQueueName{ "loading" };
        std::condition_variable cv;

        const auto levelData = m_ConfigManager->entityConfig(level);
        std::function<void()> entityFutures = m_EntityManager->load(levelData, cv);
        std::function<void()> textureFuture = m_TextureManager->load(cv);

        const std::string sb = (m_CurrentSkybox.empty()) ? static_cast<std::string>(appConfig["defaultSkybox"]) : m_CurrentSkybox;
        std::function<void()> skyboxFuture = m_TextureManager->loadSkybox(sb, cv);
        std::function<void()> shaderFuture = m_ShaderManager->load(m_ConfigManager->shaderConfig(), cv);

        Poulpe::Locator::getThreadPool()->submit(threadQueueName, entityFutures);
        Poulpe::Locator::getThreadPool()->submit(threadQueueName, textureFuture);
        Poulpe::Locator::getThreadPool()->submit(threadQueueName, skyboxFuture);
        Poulpe::Locator::getThreadPool()->submit(threadQueueName, shaderFuture);

        std::mutex loading;

        {
            std::unique_lock<std::mutex> lock(loading);
            cv.wait(lock, [=, this]() { return m_TextureManager->isTexturesLoadingDone(); });
        }
        {
            std::unique_lock<std::mutex> lock(loading);
            cv.wait(lock, [=, this]() { return m_TextureManager->isSkyboxLoadingDone(); });
        }
        {
            std::unique_lock<std::mutex> lock(loading);
            cv.wait(lock, [=, this]() { return m_ShaderManager->isLoadingDone(); });
        }
        {
            std::unique_lock<std::mutex> lock(loading);
            cv.wait(lock, [=, this]() { return m_EntityManager->IsLoadingDone(); });
        }

        setIsLoaded();

        m_Renderer->addEntities(m_EntityManager->getEntities());
    }

    void RenderManager::draw()
    {
        m_Renderer->draw();
    }

    void RenderManager::renderScene()
    {
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

    void RenderManager::prepareSplashScreen()
    {
        std::vector<VkDescriptorPoolSize> poolSizes{};
        VkDescriptorPoolSize cp1;
        cp1.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        cp1.descriptorCount = 1000;
        VkDescriptorPoolSize cp2;
        cp2.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cp2.descriptorCount = 1000;
        poolSizes.emplace_back(cp1);
        poolSizes.emplace_back(cp2);

        VkDescriptorPool descriptorPool = m_Renderer->rdr()->createDescriptorPool(poolSizes, 1000);
        m_DescriptorPools.emplace_back(descriptorPool);

        std::shared_ptr<Splash> splashVulkanisator = std::make_shared<Splash>(
            m_Renderer,
            m_EntityManager,
            m_ShaderManager,
            m_TextureManager,
            m_SpriteAnimationManager,
            descriptorPool
        );
        auto splash = std::make_shared<Mesh2D>();
        splash->accept(splashVulkanisator);

        std::vector<std::shared_ptr<Mesh>> splashs{ splash };

        m_Renderer->addSplash(splashs);
    }

    void RenderManager::prepareEntity()
    {
        m_DescriptorPools.clear();

        std::vector<VkDescriptorPoolSize> poolSizes{};
        VkDescriptorPoolSize cp1;
        cp1.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        cp1.descriptorCount = 1000;
        VkDescriptorPoolSize cp2;
        cp2.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cp2.descriptorCount = 1000;
        poolSizes.emplace_back(cp1);
        poolSizes.emplace_back(cp2);

        VkDescriptorPool descriptorPool = m_Renderer->rdr()->createDescriptorPool(poolSizes, 1000);
        m_DescriptorPools.emplace_back(descriptorPool);

        std::shared_ptr<Basic> vulkanisator = std::make_shared<Basic>(
            m_Renderer,
            m_EntityManager,
            m_ShaderManager,
            m_TextureManager,
            descriptorPool
        );

        for (std::shared_ptr<Entity>& entity : *m_EntityManager->getEntities()) {
            entity->accept(vulkanisator);
        }
    }

    void RenderManager::prepareHUD()
    {
        std::vector<VkDescriptorPoolSize> poolSizes{};
        VkDescriptorPoolSize cp1;
        cp1.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        cp1.descriptorCount = 10;
        VkDescriptorPoolSize cp2;
        cp2.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cp2.descriptorCount = 10;
        poolSizes.emplace_back(cp1);
        poolSizes.emplace_back(cp2);

        std::vector<std::shared_ptr<Mesh>> hud{};

        VkDescriptorPool descriptorPool = m_Renderer->rdr()->createDescriptorPool(poolSizes, 10);
        m_DescriptorPools.emplace_back(descriptorPool);

        auto entityG = std::make_shared<Grid>(EntityFactory::create<Grid>(
            m_Renderer, m_EntityManager, m_ShaderManager, m_TextureManager, descriptorPool));

        auto grid = std::make_shared<Mesh>();
        grid->accept(entityG);
        hud.emplace_back(grid);

        auto entityC = std::make_shared<Crosshair>(EntityFactory::create<Crosshair>(
            m_Renderer, m_EntityManager, m_ShaderManager, m_TextureManager, descriptorPool));

        auto crossHair = std::make_shared<Mesh2D>();
        crossHair->accept(entityC);
        hud.emplace_back(crossHair);

        m_EntityManager->addHUD(hud);
        m_Renderer->addHUD(m_EntityManager->getHUD());
    }

    void RenderManager::prepareSkybox()
    {
        std::vector<VkDescriptorPoolSize> poolSizes{};
        VkDescriptorPoolSize cp1;
        cp1.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        cp1.descriptorCount = 10;
        VkDescriptorPoolSize cp2;
        cp2.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cp2.descriptorCount = 10;
        poolSizes.emplace_back(cp1);
        poolSizes.emplace_back(cp2);

        VkDescriptorPool descriptorPool = m_Renderer->rdr()->createDescriptorPool(poolSizes, 10, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
        m_DescriptorPools.emplace_back(descriptorPool);

        auto entity = std::make_shared<Skybox>(EntityFactory::create<Skybox>(
            m_Renderer, m_EntityManager, m_ShaderManager, m_TextureManager));

        auto skyboxMesh = std::make_shared<Mesh>();
        skyboxMesh->m_DescriptorPool = descriptorPool;

        skyboxMesh->accept(entity);
        m_EntityManager->setSkybox(skyboxMesh);
        m_Renderer->addSkybox(skyboxMesh);
    }
}
