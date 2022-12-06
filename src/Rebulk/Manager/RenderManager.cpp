#include "rebulkpch.h"
#include "RenderManager.h"

#include "Rebulk/Renderer/Vulkan/Meshator/EntityMesh.h"
#include "Rebulk/Renderer/Vulkan/Meshator/Skybox.h"
#include "Rebulk/Renderer/Vulkan/Meshator/Crosshair.h"
#include "Rebulk/Renderer/Vulkan/Meshator/Grid.h"
#include "Rebulk/Renderer/Vulkan/Meshator/Splash.h"

namespace Rbk
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
        std::shared_ptr<Rbk::DestroyManager> destroyManager,
        std::shared_ptr<Rbk::Camera> camera
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
        m_Camera(camera)
    {
        m_Camera->Init();
        m_Renderer->Init();
        m_Renderer->AddCamera(m_Camera);
        m_DestroyManager->SetRenderer(m_Renderer->Rdr());
        m_DestroyManager->AddMemoryPool(m_Renderer->Rdr()->GetDeviceMemoryPool());
        m_TextureManager->AddConfig(m_ConfigManager->TexturesConfig());

        nlohmann::json appConfig = m_ConfigManager->AppConfig();
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
        m_DestroyManager->CleanEntities(*m_EntityManager->GetBBox());
        m_DestroyManager->CleanEntities(m_EntityManager->GetHUD());
        m_DestroyManager->CleanShaders(m_ShaderManager->GetShaders()->shaders);
        m_DestroyManager->CleanTextures(m_TextureManager->GetTextures());
        m_DestroyManager->CleanTexture(m_TextureManager->GetSkyboxTexture());
        m_DestroyManager->CleanDeviceMemory();
        m_AudioManager->Clear();
        m_SpriteAnimationManager->Clear();
        m_ShaderManager->Clear();
        m_Renderer->Clear();
        m_Renderer->Destroy();
        m_Renderer->Rdr()->Destroy();
    }

    void RenderManager::Init()
    {
        if (m_Refresh) {
            m_Renderer->Rdr()->WaitIdle();
            m_DestroyManager->CleanEntities(*m_EntityManager->GetEntities());
            m_DestroyManager->CleanEntities(*m_EntityManager->GetBBox());
            m_DestroyManager->CleanEntities(m_EntityManager->GetHUD());
            m_DestroyManager->CleanShaders(m_ShaderManager->GetShaders()->shaders);
            m_DestroyManager->CleanTextures(m_TextureManager->GetTextures());
            m_DestroyManager->CleanTexture(m_TextureManager->GetSkyboxTexture());
            m_DestroyManager->CleanDeviceMemory();
            m_Renderer->Rdr()->InitMemoryPool();
            m_TextureManager->Clear();
            m_EntityManager->Clear();
            m_AudioManager->Clear();
            m_SpriteAnimationManager->Clear();
            m_ShaderManager->Clear();
            m_Renderer->Clear();
            m_Renderer->RecreateSwapChain();
        }
       
        nlohmann::json appConfig = m_ConfigManager->AppConfig();
        nlohmann::json textureConfig = m_ConfigManager->TexturesConfig();

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

        std::thread loading([this]() {
            while (!IsLoaded()) {
                    m_Renderer->DrawSplashScreen();
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
        });

        LoadData(m_CurrentLevel);

        loading.join();

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

        std::vector<std::future<void>> entityFutures = m_EntityManager->Load(
            m_ConfigManager->EntityConfig(level)
        );

        for (auto& future : entityFutures) {
            future.wait();
        }

        std::future<void> textureFuture = m_TextureManager->Load();
        std::future<void> skyboxFuture = m_TextureManager->LoadSkybox(static_cast<std::string>(appConfig["defaultSkybox"]));
        std::future<void> shaderFuture = m_ShaderManager->Load(m_ConfigManager->ShaderConfig());

        textureFuture.wait();
        skyboxFuture.wait();
        shaderFuture.wait();

        SetIsLoaded();
        m_Renderer->AddEntities(m_EntityManager->GetEntities());
        m_Renderer->AddBbox(m_EntityManager->GetBBox());
    }

    void RenderManager::Draw()
    {
        if (m_Refresh) {
            Init();
            m_Refresh = false;
        }

        if (m_EntityManager->GetSkybox()->IsDirty()) {
            std::async(std::launch::async, [=]() {
                PrepareSkybox();
            });
        }
        
        m_Renderer->Draw();
    }

    void RenderManager::SetDeltatime(float deltaTime)
    {
        m_Renderer->SetDeltatime(deltaTime);
    }

    void RenderManager::Refresh(uint32_t levelIndex)
    {
        m_CurrentLevel = m_ConfigManager->ListLevels().at(levelIndex);
        m_IsLoaded = false;
        m_Refresh = true;
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
        cp1.descriptorCount = 100;
        VkDescriptorPoolSize cp2;
        cp2.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cp2.descriptorCount = 100;
        poolSizes.emplace_back(cp1);
        poolSizes.emplace_back(cp2);

        VkDescriptorPool descriptorPool = m_Renderer->Rdr()->CreateDescriptorPool(poolSizes, 100);
        m_DescriptorPools.emplace_back(descriptorPool);

        std::shared_ptr<EntityMesh> vulkanisator = std::make_shared<EntityMesh>(
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

        VkDescriptorPool HUDDescriptorPool = m_Renderer->Rdr()->CreateDescriptorPool(poolSizes, 10);
        m_DescriptorPools.emplace_back(HUDDescriptorPool);

        std::shared_ptr<Grid> gridVulkanisator = std::make_shared<Grid>(
            m_Renderer,
            m_EntityManager,
            m_ShaderManager,
            m_TextureManager,
            HUDDescriptorPool
            );

        auto grid = std::make_shared<Mesh>();
        grid->Accept(gridVulkanisator);
        hud.emplace_back(grid);

        std::shared_ptr<Crosshair> crosshairVulkanisator = std::make_shared<Crosshair>(
            m_Renderer,
            m_EntityManager,
            m_ShaderManager,
            m_TextureManager,
            HUDDescriptorPool
            );
        auto crossHair = std::make_shared<Mesh2D>();
        crossHair->Accept(crosshairVulkanisator);
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


        VkDescriptorPool skyDescriptorPool = m_Renderer->Rdr()->CreateDescriptorPool(poolSizes, 10);
        m_DescriptorPools.emplace_back(skyDescriptorPool);
        std::shared_ptr<Skybox> skyboxVulkanisator = std::make_shared<Skybox>(
            m_Renderer,
            m_EntityManager,
            m_ShaderManager,
            m_TextureManager,
            skyDescriptorPool
        );

        auto skyboxMesh = std::make_shared<Mesh>();
        skyboxMesh->Accept(skyboxVulkanisator);
        m_EntityManager->SetSkybox(skyboxMesh);
        m_Renderer->AddSkybox(skyboxMesh);
    }
}
