#include "rebulkpch.h"
#include "Application.h"

namespace Rbk
{
    class Mesh;

    int Application::s_UnlockedFPS = 0;

    Application* Application::s_Instance = nullptr;

    VImGuiInfo vImGuiInfo;

    Application::Application()
    {
        if (s_Instance == nullptr) {
            s_Instance = this;
        }
    }

    Application::~Application()
    {

    }

    void Application::Init()
    {
        Rbk::Log::Init();

        m_Window = std::make_shared<Window>();
        m_Window->Init();

        int width, height;
        glfwGetWindowSize(m_Window->Get(), &width, &height);
        m_Camera = std::make_shared<Rbk::Camera>();
        m_Camera->Init();
        
        m_InputManager = std::make_shared<Rbk::InputManager>(m_Window, m_Camera);
        m_InputManager->Init();
        m_LayerManager = std::make_shared<Rbk::LayerManager>();
        m_RendererAdapter = std::make_shared<Rbk::VulkanAdapter>(m_Window);
        m_TextureManager = std::make_shared<Rbk::TextureManager>(m_RendererAdapter->Rdr());
        m_EntityManager = std::make_shared<Rbk::EntityManager>(m_RendererAdapter->Rdr());
        m_ShaderManager = std::make_shared<Rbk::ShaderManager>(m_RendererAdapter->Rdr());
        m_RenderManager = std::make_shared<Rbk::RenderManager>(
            m_Window, m_RendererAdapter, m_TextureManager, m_EntityManager, m_ShaderManager
        );
        m_RenderManager->Init();
        m_RenderManager->AddCamera(m_Camera);
    }

    void Application::Run()
    {
        double startRun = glfwGetTime();

        std::thread loadShaders([this]() {
            m_ShaderManager->AddShader("main", "assets/shaders/spv/vert.spv", "assets/shaders/spv/frag.spv");
            m_ShaderManager->AddShader("skybox", "assets/shaders/spv/skybox_vert.spv", "assets/shaders/spv/skybox_frag.spv");
            m_ShaderManager->AddShader("2d", "assets/shaders/spv/2d_vert.spv", "assets/shaders/spv/2d_frag.spv");
            m_ShaderManager->AddShader("ambient_light", "assets/shaders/spv/ambient_shader_vert.spv", "assets/shaders/spv/ambient_shader_frag.spv");
        });

        std::thread loadTextures([this]() {
            m_TextureManager->AddTexture("minecraft_grass", "assets/mesh/minecraft/Grass_Block_TEX.png");
            m_TextureManager->AddTexture("minecraft_water", "assets/mesh/minecraft/water.jpg");
            m_TextureManager->AddTexture("campfire_tex", "assets/mesh/campfire/Campfire_MAT_BaseColor_01.jpg");
            m_TextureManager->AddTexture("tree_tex", "assets/mesh/tree/tree.jpg");
            m_TextureManager->AddTexture("tree_top_tex", "assets/mesh/tree/tree.png");
            m_TextureManager->AddTexture("crosshair", "assets/texture/crosshair/simple_crosshair.png");
            m_TextureManager->AddTexture("moon", "assets/mesh/moon/diffuse.jpg");
            m_TextureManager->AddTexture("trunk_tree_cartoon", "assets/mesh/tree/cartoon/Trunk_4_Cartoon.jpg");
            m_TextureManager->AddTexture("grass", "assets/mesh/grass/grass.png");
            m_TextureManager->AddTexture("rooftiles", "assets/mesh/house/textures/rooftiles/T_darkwood_basecolor.png");
            m_TextureManager->AddTexture("dark_wood", "assets/mesh/house/textures/wood/T_darkwood_basecolor.png");
            m_TextureManager->AddTexture("bright_wood", "assets/mesh/house/textures/wood/T_brightwood_basecolor.png");
            m_TextureManager->AddTexture("rocks", "assets/mesh/house/textures/rocks/rock_bc.jpg");
            m_TextureManager->AddTexture("dog_base_color", "assets/mesh/doghouse/doghouse0908_PBR_BaseColor.png");
            m_TextureManager->AddTexture("dog", "assets/mesh/dog/Texture_albedo.jpg");
        });

        std::thread loadAnimals([this]() {
            std::shared_ptr<Mesh> entity = std::make_shared<Mesh>();

            entity->Init(
                "doggo",
                "assets/mesh/dog/dog.obj",
                { "dog" },
                "main",
                glm::vec3(0.8f, 0.4f, -1.2f),
                glm::vec3(0.12f, 0.12f, 0.12f));

            m_EntityManager->AddEntity(entity);
        });

        std::thread loadWorldMeshFloor([this]() {

            for (int x = -7; x < 7; x++) {
                for (int y = -5; y < 15; y++) {
                    std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
                    entity->Init(
                        "ground_cube",
                        "assets/mesh/minecraft/Grass_Block.obj",
                        { "minecraft_grass" },
                        "main",
                        glm::vec3(-0.20f * static_cast<float>(x), 0.0f, -0.20f * static_cast<float>(y)),
                        glm::vec3(0.1f, 0.1f, 0.1f));

                    m_EntityManager->AddEntity(entity);
                }
            }

            for (int x = -22; x < -7; x++) {
                for (int y = -5; y < 15; y++) {
                    std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
                    entity->Init(
                        "ground_cube_2",
                        "assets/mesh/minecraft/Grass_Block.obj",
                        { "minecraft_grass" },
                        "main",
                        glm::vec3(-0.20f * static_cast<float>(x), 0.0f, -0.20f * static_cast<float>(y)),
                        glm::vec3(0.1f, 0.1f, 0.1f));

                    m_EntityManager->AddEntity(entity);
                }
            }

            for (int x = 7; x < 22; x++) {
                for (int y = -5; y < 15; y++) {
                    std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
                    entity->Init(
                        "water_cube_1",
                        "assets/mesh/minecraft/Grass_Block.obj",
                        { "minecraft_water" },
                        "main",
                        glm::vec3(-0.20f * static_cast<float>(x), 0.0f, -0.20f * static_cast<float>(y)),
                        glm::vec3(0.1f, 0.1f, 0.1f));

                    m_EntityManager->AddEntity(entity);
                }
            }

            for (int x = -22; x < 22; x++) {
                for (int y = 15; y < 22; y++) {
                    std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
                    entity->Init(
                        "water_cube_2",
                        "assets/mesh/minecraft/Grass_Block.obj",
                        { "minecraft_water" },
                        "main",
                        glm::vec3(-0.20f * static_cast<float>(x), 0.0f, -0.20f * static_cast<float>(y)),
                        glm::vec3(0.1f, 0.1f, 0.1f));

                    m_EntityManager->AddEntity(entity);
                }
            }

            for (int x = -22; x < 22; x++) {
                for (int y = 22; y < 29; y++) {
                    std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
                    entity->Init(
                        "water_cube_3",
                        "assets/mesh/minecraft/Grass_Block.obj",
                        { "minecraft_water" },
                        "main",
                        glm::vec3(-0.20f * static_cast<float>(x), 0.0f, -0.20f * static_cast<float>(y)),
                        glm::vec3(0.1f, 0.1f, 0.1f));

                    m_EntityManager->AddEntity(entity);
                }
            }
        });

        std::thread loadWorlMesh([this]() {
            std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
            entity->Init(
                "campfire",
                "assets/mesh/campfire/Campfire.obj",
                { "campfire_tex" },
                "main",
                glm::vec3(0.8f, 0.2f, -0.4f),
                glm::vec3(0.002f, 0.002f, 0.002f));

            m_EntityManager->AddEntity(entity);

            std::shared_ptr<Rbk::Mesh> entity1 = std::make_shared<Rbk::Mesh>();
            entity1->Init(
                "dead_tree",
                "assets/mesh/tree/dead_tree.obj",
                { "trunk_tree_cartoon" },
                "main",
                glm::vec3(-1.0f, 0.19f, -1.4f),
                glm::vec3(0.01f, 0.01f, 0.01f));

            m_EntityManager->AddEntity(entity1);
      
            std::shared_ptr<Rbk::Mesh> entity2 = std::make_shared<Rbk::Mesh>();
            entity2->Init(
                "dead_tree",
                "assets/mesh/tree/dead_tree.obj",
                { "trunk_tree_cartoon" },
                "main",
                glm::vec3(1.0f, 0.19f, 1.0f),
                glm::vec3(0.01f, 0.01f, 0.01f),
                glm::vec3(0.0f, 1.0f, 0.0f),
                -20.0f);

            m_EntityManager->AddEntity(entity2);

            std::shared_ptr<Rbk::Mesh> entity3 = std::make_shared<Rbk::Mesh>();
            entity3->Init(
                "dead_tree",
                "assets/mesh/tree/dead_tree.obj",
                { "trunk_tree_cartoon" },
                "main",
                glm::vec3(1.0f, 0.18f, -2.0f),
                glm::vec3(0.01f, 0.01f, 0.01f));

            m_EntityManager->AddEntity(entity3);
          
            std::shared_ptr<Rbk::Mesh> entity4 = std::make_shared<Rbk::Mesh>();
            entity4->Init(
                "dead_tree",
                "assets/mesh/tree/dead_tree.obj",
                { "trunk_tree_cartoon" },
                "main",
                glm::vec3(3.0f, 0.18f, -2.2f),
                glm::vec3(0.01f, 0.01f, 0.01f));

            m_EntityManager->AddEntity(entity);

            std::shared_ptr<Rbk::Mesh> entity5 = std::make_shared<Rbk::Mesh>();
            entity5->Init(
                "dead_tree",
                "assets/mesh/tree/dead_tree.obj",
                { "trunk_tree_cartoon" },
                "main",
                glm::vec3(3.0f, 0.18f, -0.6f),
                glm::vec3(0.01f, 0.01f, 0.01f));

            m_EntityManager->AddEntity(entity5);
       
            std::shared_ptr<Rbk::Mesh> entity6 = std::make_shared<Rbk::Mesh>();
            entity6->Init(
                "tree",
                "assets/mesh/tree/tree.obj",
                { "tree_top_tex", "trunk_tree_cartoon" },
                "main",
                glm::vec3(3.0f, 0.18f, -0.6f),
                glm::vec3(0.0008f, 0.0008f, 0.0008f));

            m_EntityManager->AddEntity(entity6);
         
            std::shared_ptr<Rbk::Mesh> entity7 = std::make_shared<Rbk::Mesh>();
            entity7->Init(
                "tree",
                "assets/mesh/tree/tree.obj",
                { "tree_top_tex", "trunk_tree_cartoon" },
                "main",
                glm::vec3(-0.25f, 0.19f, -1.2f),
                glm::vec3(0.0008f, 0.0008f, 0.0008f));

            m_EntityManager->AddEntity(entity7);

            std::shared_ptr<Rbk::Mesh> entity8 = std::make_shared<Rbk::Mesh>();
            entity8->Init(
                "tree",
                "assets/mesh/tree/tree.obj",
                { "tree_top_tex", "trunk_tree_cartoon" },
                "main",
                glm::vec3(0.7f, 0.19f, -1.6f),
                glm::vec3(0.0008f, 0.0008f, 0.0008f));

            m_EntityManager->AddEntity(entity8);
          
            std::shared_ptr<Rbk::Mesh> entity9 = std::make_shared<Rbk::Mesh>();
            entity9->Init(
                "tree",
                "assets/mesh/tree/tree.obj",
                { "tree_top_tex", "trunk_tree_cartoon" },
                "main",
                glm::vec3(1.2f, 0.19f, -0.9f),
                glm::vec3(0.0008f, 0.0008f, 0.0008f));

            m_EntityManager->AddEntity(entity9);

            std::shared_ptr<Rbk::Mesh> entity10 = std::make_shared<Rbk::Mesh>();
            entity10->Init(
                "tree",
                "assets/mesh/tree/tree.obj",
                { "tree_top_tex", "trunk_tree_cartoon" },
                "main",
                glm::vec3(2.2f, 0.19f, -1.9f),
                glm::vec3(0.0008f, 0.0008f, 0.0008f));

            m_EntityManager->AddEntity(entity10);

            std::shared_ptr<Rbk::Mesh> entity11 = std::make_shared<Rbk::Mesh>();
            entity11->Init(
                "tree",
                "assets/mesh/tree/tree.obj",
                { "tree_top_tex", "trunk_tree_cartoon" },
                "main",
                glm::vec3(3.2f, 0.19f, -0.2f),
                glm::vec3(0.0008f, 0.0008f, 0.0008f));

            m_EntityManager->AddEntity(entity11);
         
            std::shared_ptr<Rbk::Mesh> entity12 = std::make_shared<Rbk::Mesh>();
            entity12->Init(
                "tree",
                "assets/mesh/tree/tree.obj",
                { "tree_top_tex", "trunk_tree_cartoon" },
                "main",
                glm::vec3(2.8f, 0.19f, 0.5f),
                glm::vec3(0.0008f, 0.0008f, 0.0008f));

            m_EntityManager->AddEntity(entity12);
          
            std::shared_ptr<Rbk::Mesh> entity13 = std::make_shared<Rbk::Mesh>();
            entity13->Init(
                "tree",
                "assets/mesh/tree/tree.obj",
                { "tree_top_tex", "trunk_tree_cartoon" },
                "main",
                glm::vec3(2.0f, 0.19f, 0.5f),
                glm::vec3(0.0008f, 0.0008f, 0.0008f));

            m_EntityManager->AddEntity(entity13);

            std::shared_ptr<Rbk::Mesh> entity14 = std::make_shared<Rbk::Mesh>();
            entity14->Init(
                "moon",
                "assets/mesh/moon/moon.obj",
                { "moon" },
                "ambient_light",
                glm::vec3(0.5f, 4.5f, -3.00f),
                glm::vec3(0.2f, 0.2f, 0.2f),
                glm::vec3(1.0f),
                0.0f,
                false);

            m_EntityManager->AddEntity(entity14);

            std::shared_ptr<Rbk::Mesh> entity15 = std::make_shared<Rbk::Mesh>();
            entity15->Init(
                "house",
                "assets/mesh/house/midpoly_town_house_01.obj",
                { "dark_wood", "rocks", "rooftiles", "bright_wood" },
                "main",
                glm::vec3(-0.5f, 0.19f, -0.20f),
                glm::vec3(0.1f, 0.1f, 0.1f),
                glm::vec3(0.0f, 1.0f, 0.0f),
                -80.0f,
                false);

            m_EntityManager->AddEntity(entity15);
          
            std::shared_ptr<Rbk::Mesh> entity16 = std::make_shared<Rbk::Mesh>();
            entity16->Init(
                "dog_house",
                "assets/mesh/doghouse/doghouse0908.obj",
                { "dog_base_color" },
                "main",
                glm::vec3(0.7f, 0.20f, -2.0f),
                glm::vec3(0.3f, 0.3f, 0.3f),
                glm::vec3(0.0f, 1.0f, 0.0f),
                -30.0f);

            m_EntityManager->AddEntity(entity16);
        });

        std::thread loadSkybox([this]() {
            std::vector<std::string>skyboxImages;
            skyboxImages.emplace_back("assets/texture/skybox/bluesky/right.jpg");
            skyboxImages.emplace_back("assets/texture/skybox/bluesky/left.jpg");
            skyboxImages.emplace_back("assets/texture/skybox/bluesky/top.jpg");
            skyboxImages.emplace_back("assets/texture/skybox/bluesky/bottom.jpg");
            skyboxImages.emplace_back("assets/texture/skybox/bluesky/front.jpg");
            skyboxImages.emplace_back("assets/texture/skybox/bluesky/back.jpg");
            m_TextureManager->AddSkyBox(skyboxImages);
        });
     
#ifdef RBK_DEBUG
        std::shared_ptr<Rbk::VulkanLayer>vulkanLayer = std::make_shared<Rbk::VulkanLayer>();
        m_LayerManager->Add(vulkanLayer.get());

        VImGuiInfo imguiInfo = m_RendererAdapter->GetVImGuiInfo();
        Rbk::Im::Init(m_Window->Get(), imguiInfo.info, imguiInfo.rdrPass);

        m_RendererAdapter->ImmediateSubmit([&](VkCommandBuffer cmd) {
            ImGui_ImplVulkan_CreateFontsTexture(cmd);
        });

        ImGui_ImplVulkan_DestroyFontUploadObjects();

        vulkanLayer->AddWindow(m_Window);
        vulkanLayer->AddTextureManager(m_TextureManager);
        vulkanLayer->AddEntityManager(m_EntityManager);
        vulkanLayer->AddShaderManager(m_ShaderManager);
#endif

        loadAnimals.join();
        loadShaders.join();
        loadTextures.join();
        loadSkybox.join();
        loadWorlMesh.join();
        loadWorldMeshFloor.join();

        double endRun = glfwGetTime();

        Rbk::Log::GetLogger()->debug("Loaded scene in {}", endRun - startRun);

        m_RendererAdapter->Prepare();

        double lastTime = endRun;
        double timeStepSum = 0.0;
        uint32_t frameCount = 0;
        double maxFPS = 60.0;
        double maxPeriod = 1.0 / maxFPS;

        while (!glfwWindowShouldClose(m_Window->Get())) {

            if (Application::s_UnlockedFPS == 0) {
                maxFPS = 60.0;
                maxPeriod = 1.0 / maxFPS;
            } else if (Application::s_UnlockedFPS == 1) {
                maxFPS = 120.0;
                maxPeriod = 1.0 / maxFPS;
            }

            double currentTime = glfwGetTime();
            double timeStep = currentTime - lastTime;

            if (timeStep >= maxPeriod || Application::s_UnlockedFPS == 2) {
                
                timeStepSum += timeStep;
                frameCount++;

                if (1.0 <= timeStepSum) {
                    Rbk::Log::GetLogger()->debug("{} fps/sec", frameCount);
                    timeStepSum = 0.0;
                    frameCount = 0;
                }

                m_Camera->UpdateSpeed(timeStep);

                glfwPollEvents();
                m_RenderManager->SetDeltatime(timeStep);
                m_RenderManager->Draw();

#ifdef RBK_DEBUG
                //@todo move to LayerManager
                Rbk::Im::NewFrame();

                vulkanLayer->AddRenderAdapter(m_RendererAdapter);
                vulkanLayer->Render(timeStep, m_RendererAdapter->Rdr()->GetDeviceProperties());

                Rbk::Im::Render();

                m_RendererAdapter->Rdr()->BeginCommandBuffer(imguiInfo.cmdBuffer);
                ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imguiInfo.cmdBuffer);
                m_RendererAdapter->Rdr()->EndCommandBuffer(imguiInfo.cmdBuffer);
                //end @todo
#endif
                lastTime = currentTime;
            }
        }

#ifdef RBK_DEBUG
        m_RendererAdapter->Rdr()->WaitIdle();
        Rbk::Im::Destroy();
#endif
        m_RenderManager->Adp()->Destroy();

        glfwDestroyWindow(m_Window.get()->Get());
        glfwTerminate();
    }
}
