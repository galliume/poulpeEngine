#include "rebulkpch.h"
#include "Application.h"

namespace Rbk
{
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
            glm::vec3 pos = glm::vec3(0.8f, 0.4f, -1.2f);
            glm::vec3 scale = glm::vec3(0.12f, 0.12f, 0.12f);
            m_EntityManager->AddMesh("campfire", "assets/mesh/dog/dog.obj", { "dog" }, "main", pos, scale);
        });

        std::thread loadWorldMeshFloor([this]() {
            glm::vec3 scaleMinecraft = glm::vec3(0.1f, 0.1f, 0.1f);

            for (int x = -7; x < 7; x++) {
                for (int y = -5; y < 15; y++) {
                    glm::vec3 posCube = glm::vec3(-0.20f * static_cast<float>(x), 0.0f, -0.20f * static_cast<float>(y));
                    m_EntityManager->AddMesh("ground_cube", "assets/mesh/minecraft/Grass_Block.obj", { "minecraft_grass" }, "main", posCube, scaleMinecraft);
                }
            }

            for (int x = -22; x < -7; x++) {
                for (int y = -5; y < 15; y++) {
                    glm::vec3 posCube = glm::vec3(-0.20f * static_cast<float>(x), 0.0f, -0.20f * static_cast<float>(y));
                    m_EntityManager->AddMesh("ground_cube_2", "assets/mesh/minecraft/Grass_Block.obj", { "minecraft_grass" }, "main", posCube, scaleMinecraft);
                }
            }

            for (int x = 7; x < 22; x++) {
                for (int y = -5; y < 15; y++) {
                    glm::vec3 posCube = glm::vec3(-0.20f * static_cast<float>(x), 0.0f, -0.20f * static_cast<float>(y));
                    m_EntityManager->AddMesh("water_cube_2", "assets/mesh/minecraft/Grass_Block.obj", { "minecraft_water" }, "main", posCube, scaleMinecraft);
                }
            }

            for (int x = -22; x < 22; x++) {
                for (int y = 15; y < 22; y++) {
                    glm::vec3 posCube = glm::vec3(-0.20f * static_cast<float>(x), 0.0f, -0.20f * static_cast<float>(y));
                    m_EntityManager->AddMesh("water_cube_3", "assets/mesh/minecraft/Grass_Block.obj", { "minecraft_water" }, "main", posCube, scaleMinecraft);
                }
            }

            for (int x = -22; x < 22; x++) {
                for (int y = 22; y < 29; y++) {
                    glm::vec3 posCube = glm::vec3(-0.20f * static_cast<float>(x), 0.0f, -0.20f * static_cast<float>(y));
                    m_EntityManager->AddMesh("water_cube_4", "assets/mesh/minecraft/Grass_Block.obj", { "minecraft_water" }, "main", posCube, scaleMinecraft);
                }
            }
        });

        std::thread loadWorlMesh([this]() {
            glm::vec3 pos1 = glm::vec3(0.8f, 0.2f, -0.4f);
            glm::vec3 scaleCamp = glm::vec3(0.002f, 0.002f, 0.002f);
            m_EntityManager->AddMesh("campfire", "assets/mesh/campfire/Campfire.obj", { "campfire_tex" }, "main", pos1, scaleCamp);

            glm::vec3 pos2 = glm::vec3(-1.0f, 0.19f, -1.4f);
            glm::vec3 scaleDeadTree = glm::vec3(0.01f, 0.01f, 0.01f);
            m_EntityManager->AddMesh("dead_tree", "assets/mesh/tree/dead_tree.obj", { "trunk_tree_cartoon" }, "main", pos2, scaleDeadTree, glm::vec3(0.0f, 1.0f, 0.0f), 20.0f);

            pos2 = glm::vec3(1.0f, 0.19f, 1.0f);
            m_EntityManager->AddMesh("dead_tree", "assets/mesh/tree/dead_tree.obj", { "trunk_tree_cartoon" }, "main", pos2, scaleDeadTree, glm::vec3(0.0f, 1.0f, 0.0f), -20.0f);

            pos2 = glm::vec3(1.0f, 0.18f, -2.0f);
            m_EntityManager->AddMesh("dead_tree", "assets/mesh/tree/dead_tree.obj", { "trunk_tree_cartoon" }, "main", pos2, scaleDeadTree);

            pos2 = glm::vec3(3.0f, 0.18f, -2.2f);
            m_EntityManager->AddMesh("dead_tree", "assets/mesh/tree/dead_tree.obj", { "trunk_tree_cartoon" }, "main", pos2, scaleDeadTree);
            
            pos2 = glm::vec3(2.3f, 0.18f, 0.3f);
            m_EntityManager->AddMesh("dead_tree", "assets/mesh/tree/dead_tree.obj", { "trunk_tree_cartoon" }, "main", pos2, scaleDeadTree);

            pos2 = glm::vec3(3.0f, 0.18f, -0.6f);
            m_EntityManager->AddMesh("dead_tree", "assets/mesh/tree/dead_tree.obj", { "trunk_tree_cartoon" }, "main", pos2, scaleDeadTree);


            glm::vec3 scaleTree = glm::vec3(0.0008f, 0.0008f, 0.0008f);
            pos2 = glm::vec3(-0.7f, 0.19f, -1.9f);
            m_EntityManager->AddMesh("tree", "assets/mesh/tree/tree.obj", { "tree_top_tex", "trunk_tree_cartoon" }, "main", pos2, scaleTree, glm::vec3(0.0f, 1.0f, 0.0f), 20.0f);

            pos2 = glm::vec3(-0.25f, 0.19f, -1.2f);
            m_EntityManager->AddMesh("tree", "assets/mesh/tree/tree.obj", { "tree_top_tex", "trunk_tree_cartoon" }, "main", pos2, scaleTree, glm::vec3(0.0f, 1.0f, 0.0f), 10.0f);

            pos2 = glm::vec3(0.7f, 0.19f, -1.6f);
            m_EntityManager->AddMesh("tree", "assets/mesh/tree/tree.obj", { "tree_top_tex", "trunk_tree_cartoon" }, "main", pos2, scaleTree, glm::vec3(0.0f, 1.0f, 0.0f), 80.0f);

            pos2 = glm::vec3(1.2f, 0.19f, -0.9f);
            m_EntityManager->AddMesh("tree", "assets/mesh/tree/tree.obj", { "tree_top_tex", "trunk_tree_cartoon" }, "main", pos2, scaleTree);

            pos2 = glm::vec3(2.2f, 0.19f, -1.9f);
            m_EntityManager->AddMesh("tree", "assets/mesh/tree/tree.obj", { "tree_top_tex", "trunk_tree_cartoon" }, "main", pos2, scaleTree);

            pos2 = glm::vec3(3.2f, 0.19f, -0.2f);
            m_EntityManager->AddMesh("tree", "assets/mesh/tree/tree.obj", { "tree_top_tex", "trunk_tree_cartoon" }, "main", pos2, scaleTree);

            pos2 = glm::vec3(2.8f, 0.19f, 0.5f);
            m_EntityManager->AddMesh("tree", "assets/mesh/tree/tree.obj", { "tree_top_tex", "trunk_tree_cartoon" }, "main", pos2, scaleTree);

            pos2 = glm::vec3(2.0f, 0.19f, 0.5f);
            m_EntityManager->AddMesh("tree", "assets/mesh/tree/tree.obj", { "tree_top_tex", "trunk_tree_cartoon" }, "main", pos2, scaleTree);


            glm::vec3 moonCubeTest = glm::vec3(0.2f, 0.2f, 0.2f);
            glm::vec3 pos6 = glm::vec3(0.5f, 4.5f, -3.00f);
            m_EntityManager->AddMesh("moon", "assets/mesh/moon/moon.obj", { "moon" }, "ambient_light", pos6, moonCubeTest, glm::vec3(1.0f), 0.0f, false);

            glm::vec3 houseScale = glm::vec3(0.1f, 0.1f, 0.1f);
            glm::vec3 posHouse = glm::vec3(-0.5f, 0.19f, -0.20f);
            glm::vec3 rotHouse = glm::vec3(0.0f, 1.0f, 0.0f);
            m_EntityManager->AddMesh("house", "assets/mesh/house/midpoly_town_house_01.obj", { "dark_wood", "rocks", "rooftiles", "bright_wood" }, "main", posHouse, houseScale, rotHouse, -80.0f, false);

            glm::vec3 dogScale = glm::vec3(0.3f, 0.3f, 0.3f);
            glm::vec3 posDog = glm::vec3(0.7f, 0.20f, -2.0f);
            glm::vec3 rotDog = glm::vec3(0.0f, 1.0f, 0.0f);
            m_EntityManager->AddMesh("dog_house", "assets/mesh/doghouse/doghouse0908.obj", { "dog_base_color" }, "main", posDog, dogScale, rotDog, -30.0f);
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
