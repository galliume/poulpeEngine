#include "rebulkpch.h"
#include "RenderManager.h"

namespace Rbk
{
    RenderManager* RenderManager::s_Instance = nullptr;

    RenderManager::RenderManager(
        std::shared_ptr<Window> window,
        std::shared_ptr<IRendererAdapter> renderer,
        std::shared_ptr<IConfigManager> configManager,
        std::shared_ptr<IInputManager> inputManager,
        std::shared_ptr<IAudioManager> audioManager,
        std::shared_ptr<ITextureManager> textureManager,
        std::shared_ptr<IEntityManager> entityManager,
        std::shared_ptr<IShaderManager> shaderManager,
        std::shared_ptr<ISpriteAnimationManager> spriteAnimationManager,
        std::shared_ptr<Rbk::ILayerManager> layerManager,
        std::shared_ptr<Rbk::Camera> camera
    ) : m_Window(window),
        m_Renderer(renderer),
        m_ConfigManager(configManager),
        m_InputManager(inputManager),
        m_AudioManager(audioManager),
        m_TextureManager(textureManager),
        m_EntityManager(entityManager),
        m_ShaderManager(shaderManager),
        m_SpriteAnimationManager(spriteAnimationManager),
        m_LayerManager(layerManager),
        m_Camera(camera)
    {
        if (s_Instance == nullptr) {
            s_Instance = this;
        }
    }

    RenderManager::~RenderManager()
    {
        std::cout << "RenderManager deleted" << std::endl;
    }

    void RenderManager::Init()
    {
        m_Camera->Init();

        m_Renderer->WaitIdle();
        m_Renderer->Init();
        m_Renderer->AddCamera(m_Camera);

        m_TextureManager->AddConfig(m_ConfigManager->TexturesConfig());
        
        //@todo, those managers should not have the usage of the renderer...
        m_TextureManager->AddRenderer(m_Renderer);
        m_EntityManager->AddRenderer(m_Renderer);
        //end @todo

        m_Renderer->AddTextureManager(m_TextureManager);
        m_Renderer->AddEntityManager(m_EntityManager);
        m_Renderer->AddShaderManager(m_ShaderManager);
        m_Renderer->AddSpriteAnimationManager(m_SpriteAnimationManager);

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

        m_InputManager->Init(inputConfig);

        std::vector<std::string> splashSprites{};
        for (auto& texture : textureConfig["splash"].items()) {
            splashSprites.emplace_back(texture.value());
        }

        m_SpriteAnimationManager->Add("splashAnim", splashSprites);
        m_ShaderManager->AddShader("splashscreen", "assets/shaders/spv/2d_vert.spv", "assets/shaders/spv/2d_frag.spv");
        m_Renderer->PrepareSplashScreen();
        m_AudioManager->Load(soundConfig);

        if (splashScreenMusic)
            m_AudioManager->StartSplash();

        std::future<void> loading = std::async(std::launch::async, [this]() {

            while (!IsLoaded()) {
                m_Renderer->DrawSplashScreen();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            });

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
        loading.wait();
        m_AudioManager->StopSplash();

        if (ambientScreenMusic)
            m_AudioManager->StartAmbient();

        m_Renderer->Prepare();

        //todo move to layer manager and update application main loop accordingly
        m_VulkanLayer = std::make_shared<Rbk::ILayer>();
        m_VulkanLayer->AddRenderManager(shared_from_this());
        //m_LayerManager->Add(vulkanLayer.get());
    }

    void RenderManager::Draw()
    {
        m_Renderer->Draw();
    }

    void RenderManager::SetDeltatime(float deltaTime)
    {
        m_Renderer->SetDeltatime(deltaTime);
    }
}