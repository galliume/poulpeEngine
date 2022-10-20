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
        m_Camera(camera)
    {
    }

    RenderManager::~RenderManager()
    {
        std::cout << "RenderManager deleted" << std::endl;
    }

    void RenderManager::Init()
    {
        m_Renderer->WaitIdle();
        m_Camera->Init();
        m_Renderer->Init();
        m_Renderer->AddCamera(m_Camera);
        m_TextureManager->AddConfig(m_ConfigManager->TexturesConfig());
        
        nlohmann::json appConfig = m_ConfigManager->AppConfig();
        std::string defaultLevel = static_cast<std::string>(appConfig["defaultLevel"]);
        std::string defaultSkybox = static_cast<std::string>(appConfig["defaultSkybox"]);
        bool splashScreenMusic = static_cast<bool>(appConfig["splashScreenMusic"]);
        bool ambientScreenMusic = static_cast<bool>(appConfig["ambientMusic"]);
        nlohmann::json inputConfig = appConfig["input"];
        nlohmann::json textureConfig = m_ConfigManager->TexturesConfig();
        nlohmann::json shaderConfig = m_ConfigManager->ShaderConfig();
        nlohmann::json soundConfig = m_ConfigManager->SoundConfig();
        nlohmann::json levelConfig = m_ConfigManager->EntityConfig(defaultLevel);

        //@todo, those managers should not have the usage of the renderer...
        m_TextureManager->AddRenderer(m_Renderer);
        m_EntityManager->AddRenderer(m_Renderer);
        m_ShaderManager->AddRenderer(m_Renderer);
        m_InputManager->AddRenderer(m_Renderer);
        m_InputManager->SetCamera(m_Camera);
        //end @todo

        m_InputManager->Init(inputConfig);
        m_AudioManager->Load(soundConfig);

        std::vector<std::string> splashSprites{};
        for (auto& texture : textureConfig["splash"].items()) {
            splashSprites.emplace_back(texture.value());
        }

        m_SpriteAnimationManager->Add("splashAnim", splashSprites);
        m_ShaderManager->AddShader("splashscreen", "assets/shaders/spv/2d_vert.spv", "assets/shaders/spv/2d_frag.spv");

        PrepareSplashScreen();

        if (splashScreenMusic)
            m_AudioManager->StartSplash();

        std::future<void> loading = std::async(std::launch::async, [this]() {

            while (!IsLoaded()) {
                m_Renderer->DrawSplashScreen();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });

        LoadData();
        loading.wait();

        m_AudioManager->StopSplash();

        if (ambientScreenMusic)
            m_AudioManager->StartAmbient();

        Prepare();
    }

    void RenderManager::LoadData()
    {
        nlohmann::json appConfig = m_ConfigManager->AppConfig();
        std::string defaultLevel = static_cast<std::string>(appConfig["defaultLevel"]);
        std::string defaultSkybox = static_cast<std::string>(appConfig["defaultSkybox"]);
        nlohmann::json shaderConfig = m_ConfigManager->ShaderConfig();
        nlohmann::json levelConfig = m_ConfigManager->EntityConfig(defaultLevel);

        std::future<void> shaderFuture = m_ShaderManager->Load(shaderConfig);
        std::vector<std::future<void>> textureFutures = m_TextureManager->Load(defaultSkybox);
        std::vector<std::future<void>> entityFutures = m_EntityManager->Load(levelConfig);

        for (auto& future : entityFutures) {
            future.wait();
        }
        for (auto& future : textureFutures) {
            future.wait();
        }
        shaderFuture.wait();

        SetIsLoaded();
        m_Renderer->AddEntities(m_EntityManager->GetEntities());
        m_Renderer->AddBbox(m_EntityManager->GetBBox());
    }

    void RenderManager::Draw()
    {
        m_Renderer->Draw();
    }

    void RenderManager::SetDeltatime(float deltaTime)
    {
        m_Renderer->SetDeltatime(deltaTime);
    }

    void RenderManager::Refresh()
    {
        Prepare();
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

    void RenderManager::Prepare()
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

        std::vector<VkPushConstantRange> pushConstants = {};
        VkPushConstantRange vkPushconstants;
        vkPushconstants.offset = 0;
        vkPushconstants.size = sizeof(constants);
        vkPushconstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstants.emplace_back(vkPushconstants);

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

        VkDescriptorPool skyDescriptorPool = m_Renderer->Rdr()->CreateDescriptorPool(poolSizes, 1000);
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
        m_Renderer->AddSkybox(skyboxMesh);

        std::vector<std::shared_ptr<Mesh>> hud{};

        VkDescriptorPool HUDDescriptorPool = m_Renderer->Rdr()->CreateDescriptorPool(poolSizes, 1000);
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

        m_Renderer->AddHUD(hud);
    }

}