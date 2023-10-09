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
    ) :
        m_Window(window),
        m_Renderer(renderer),
        m_ConfigManager(configManager),
        m_InputManager(inputManager),
        m_AudioManager(audioManager),
        m_TextureManager(textureManager),
        m_EntityManager(entityManager),
        m_ShaderManager(shaderManager),
        m_SpriteAnimationManager(spriteAnimationManager),
        m_DestroyManager(destroyManager),
        m_Camera(camera),
        m_CommandQueue(cmdQueue)
    {
        m_Camera->Init();
        m_Renderer->Init();
        m_Renderer->AddCamera(m_Camera);
        m_DestroyManager->SetRenderer(m_Renderer->Rdr());
        m_DestroyManager->AddMemoryPool(m_Renderer->Rdr()->GetDeviceMemoryPool());
        m_TextureManager->AddConfig(m_ConfigManager->TexturesConfig());

        nlohmann::json appConfig = m_ConfigManager->AppConfig();
        if (appConfig["defaultLevel"].empty()) {
            PLP_WARN("defaultLevel conf not set.");
        }
        
        m_CurrentLevel = static_cast<std::string>(appConfig["defaultLevel"]);

        m_InputManager->Init(appConfig["input"]);

        //@todo, those managers should not have the usage of the renderer...
        m_TextureManager->AddRenderer(m_Renderer);
        m_EntityManager->AddRenderer(m_Renderer);
        m_ShaderManager->AddRenderer(m_Renderer);
        m_InputManager->AddRenderer(m_Renderer);
        m_InputManager->SetCamera(m_Camera);
        //end @todo
    }

    RenderManager::~RenderManager()
    {

    }

    void RenderManager::CleanUp()
    {
        m_DestroyManager->CleanEntities(*m_EntityManager->GetEntities());
        m_DestroyManager->CleanEntities(m_EntityManager->GetHUD());
        m_DestroyManager->CleanShaders(m_ShaderManager->GetShaders()->shaders);
        m_DestroyManager->CleanTextures(m_TextureManager->GetTextures());
        m_DestroyManager->CleanTexture(m_TextureManager->GetSkyboxTexture());
        m_AudioManager->Clear();
        m_SpriteAnimationManager->Clear();
        m_TextureManager->Clear();
        m_EntityManager->Clear();
        m_ShaderManager->Clear();
        m_Renderer->Clear();
        m_DestroyManager->CleanDeviceMemory();
        m_Renderer->Rdr()->InitMemoryPool();
    }

    void RenderManager::Init()
    {
        if (m_Refresh) {
            m_Renderer->StopRendering();
            m_Renderer->ClearRendererScreen();
            m_Renderer->Rdr()->WaitIdle();
            CleanUp();
            m_Renderer->RecreateSwapChain();
            SetIsLoaded(false);
            m_Refresh = false;
        }
       
        nlohmann::json appConfig = m_ConfigManager->AppConfig();
        nlohmann::json textureConfig = m_ConfigManager->TexturesConfig();

        m_AudioManager->Init();
        m_AudioManager->Load(m_ConfigManager->SoundConfig());

        std::vector<std::string> splashSprites{};
        for (auto& texture : textureConfig["splash"].items()) {
            splashSprites.emplace_back(texture.value());
        }

        m_SpriteAnimationManager->Add("splashAnim", splashSprites);
        m_ShaderManager->AddShader("splashscreen", "assets/shaders/spv/2d_vert.spv", "assets/shaders/spv/2d_frag.spv");

        PrepareSplashScreen();

        if (static_cast<bool>(appConfig["splashScreenMusic"]))
            m_AudioManager->StartSplash();

        std::thread loading([=, this]() {
            while (!IsLoaded()) {
                m_Renderer->DrawSplashScreen();
                std::this_thread::sleep_for(std::chrono::milliseconds(33));
            }
        });

        LoadData(m_CurrentLevel);
        loading.join();
        m_Renderer->ClearSplashScreen();

        m_AudioManager->StopSplash();

        if (static_cast<bool>(appConfig["ambientMusic"]))
            m_AudioManager->StartAmbient();

        PrepareEntity();
        PrepareSkybox();
        PrepareHUD();
    }

    void RenderManager::LoadData(const std::string& level)
    {
        nlohmann::json appConfig = m_ConfigManager->AppConfig();
        std::string_view threadQueueName{ "loading" };
        std::condition_variable cv;

        const auto levelData = m_ConfigManager->EntityConfig(level);
        std::function<void()> entityFutures = m_EntityManager->Load(levelData, cv);
        std::function<void()> textureFuture = m_TextureManager->Load(cv);

        const std::string sb = (m_CurrentSkybox.empty()) ? static_cast<std::string>(appConfig["defaultSkybox"]) : m_CurrentSkybox;
        std::function<void()> skyboxFuture = m_TextureManager->LoadSkybox(sb, cv);
        std::function<void()> shaderFuture = m_ShaderManager->Load(m_ConfigManager->ShaderConfig(), cv);

        Poulpe::Locator::getThreadPool()->Submit(threadQueueName, entityFutures);
        Poulpe::Locator::getThreadPool()->Submit(threadQueueName, textureFuture);
        Poulpe::Locator::getThreadPool()->Submit(threadQueueName, skyboxFuture);
        Poulpe::Locator::getThreadPool()->Submit(threadQueueName, shaderFuture);

        std::mutex loading;

        {
            std::unique_lock<std::mutex> lock(loading);
            cv.wait(lock, [=, this]() { return m_TextureManager->IsTexturesLoadingDone(); });
        }
        {
            std::unique_lock<std::mutex> lock(loading);
            cv.wait(lock, [=, this]() { return m_TextureManager->IsSkyboxLoadingDone(); });
        }
        {
            std::unique_lock<std::mutex> lock(loading);
            cv.wait(lock, [=, this]() { return m_ShaderManager->IsLoadingDone(); });
        }
        {
            std::unique_lock<std::mutex> lock(loading);
            cv.wait(lock, [=, this]() { return m_EntityManager->IsLoadingDone(); });
        }

        SetIsLoaded();

        m_Renderer->AddEntities(m_EntityManager->GetEntities());
    }

    void RenderManager::Draw()
    {
        m_Renderer->Draw();
    }

    void RenderManager::RenderScene()
    {
        m_CommandQueue->ExecRequest();

        m_Renderer->RenderScene();

        if (m_Refresh) {
            m_Renderer->SetDrawBbox(m_ShowBbox);
            Init();
            m_Refresh = false;
            m_ShowBbox = false;
        }
    }

    void RenderManager::Refresh(uint32_t levelIndex, bool showBbox, std::string_view skybox)
    {
        m_CurrentLevel = m_ConfigManager->ListLevels().at(levelIndex);
        m_CurrentSkybox = skybox;
        m_IsLoaded = false;
        m_Refresh = true;
        m_ShowBbox = showBbox;
    }

    void RenderManager::PrepareSplashScreen()
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

        VkDescriptorPool descriptorPool = m_Renderer->Rdr()->CreateDescriptorPool(poolSizes, 1000);
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
        splash->Accept(splashVulkanisator);

        std::vector<std::shared_ptr<Mesh>> splashs{ splash };

        m_Renderer->AddSplash(splashs);
    }

    void RenderManager::PrepareEntity()
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

        VkDescriptorPool descriptorPool = m_Renderer->Rdr()->CreateDescriptorPool(poolSizes, 1000);
        m_DescriptorPools.emplace_back(descriptorPool);

        std::shared_ptr<Basic> vulkanisator = std::make_shared<Basic>(
            m_Renderer,
            m_EntityManager,
            m_ShaderManager,
            m_TextureManager,
            descriptorPool
        );

        for (std::shared_ptr<Entity>& entity : *m_EntityManager->GetEntities()) {
            entity->Accept(vulkanisator);
        }
    }

    void RenderManager::PrepareHUD()
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

        VkDescriptorPool descriptorPool = m_Renderer->Rdr()->CreateDescriptorPool(poolSizes, 10);
        m_DescriptorPools.emplace_back(descriptorPool);

        auto entityG = std::make_shared<Grid>(EntityFactory::create<Grid>(
            m_Renderer, m_EntityManager, m_ShaderManager, m_TextureManager, descriptorPool));

        auto grid = std::make_shared<Mesh>();
        grid->Accept(entityG);
        hud.emplace_back(grid);

        auto entityC = std::make_shared<Crosshair>(EntityFactory::create<Crosshair>(
            m_Renderer, m_EntityManager, m_ShaderManager, m_TextureManager, descriptorPool));

        auto crossHair = std::make_shared<Mesh2D>();
        crossHair->Accept(entityC);
        hud.emplace_back(crossHair);

        m_EntityManager->AddHUD(hud);
        m_Renderer->AddHUD(m_EntityManager->GetHUD());
    }

    void RenderManager::PrepareSkybox()
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

        VkDescriptorPool descriptorPool = m_Renderer->Rdr()->CreateDescriptorPool(poolSizes, 10, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
        m_DescriptorPools.emplace_back(descriptorPool);

        auto entity = std::make_shared<Skybox>(EntityFactory::create<Skybox>(
            m_Renderer, m_EntityManager, m_ShaderManager, m_TextureManager, descriptorPool));

        auto skyboxMesh = std::make_shared<Mesh>();
        skyboxMesh->m_DescriptorPool = descriptorPool;

        skyboxMesh->Accept(entity);
        m_EntityManager->SetSkybox(skyboxMesh);
        m_Renderer->AddSkybox(skyboxMesh);
    }
}
