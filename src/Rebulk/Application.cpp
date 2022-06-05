#include "rebulkpch.h"
#include "Application.h"

namespace Rbk
{
    int Application::s_UnlockedFPS = 0;

    std::shared_ptr<Rbk::Window>window = nullptr;
    std::shared_ptr<Rbk::RenderManager>renderManager = nullptr;
    std::shared_ptr<Rbk::Camera>camera= nullptr;
    std::shared_ptr<Rbk::InputManager>inputManager = nullptr;
    std::shared_ptr<Rbk::VulkanAdapter>rendererAdapter = nullptr;
    std::shared_ptr<Rbk::LayerManager>layerManager = nullptr;
    std::shared_ptr<Rbk::TextureManager>textureManager = nullptr;
    std::shared_ptr<Rbk::MeshManager>meshManager = nullptr;
    std::shared_ptr<Rbk::ShaderManager>shaderManager = nullptr;
    std::shared_ptr<Rbk::VulkanLayer>vulkanLayer = nullptr;
    std::shared_ptr<Rbk::Im>vImGui = nullptr;

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

        if (window == nullptr) {
            window = std::make_shared<Window>();
            window.get()->Init();
        }

        if (camera == nullptr) {
            int width, height;
            glfwGetWindowSize(window.get()->Get(), &width, &height);
            camera = std::make_shared<Rbk::Camera>();
            camera->Init();
        }

        if (inputManager == nullptr) {
            inputManager = std::make_shared<Rbk::InputManager>(window, camera);
            inputManager->Init();
        }

        if (layerManager == nullptr) {
            layerManager = std::make_shared<Rbk::LayerManager>();
        }

        if (rendererAdapter == nullptr) {
            rendererAdapter = std::make_shared<Rbk::VulkanAdapter>(window);
        }

        if (textureManager == nullptr) {
            textureManager = std::make_shared<Rbk::TextureManager>(rendererAdapter.get()->Rdr());
        }

        if (meshManager == nullptr) {
            meshManager = std::make_shared<Rbk::MeshManager>(rendererAdapter.get()->Rdr());
        }

        if (shaderManager == nullptr) {
            shaderManager = std::make_shared<Rbk::ShaderManager>(rendererAdapter.get()->Rdr());
        }

        if (renderManager == nullptr) {
            renderManager = std::make_shared<Rbk::RenderManager>(
                window, rendererAdapter, textureManager, meshManager, shaderManager
            );
            renderManager->Init();
            renderManager->AddCamera(camera);
        }
    }

    void Application::Run()
    {		
        shaderManager->AddShader("main", "assets/shaders/spv/vert.spv", "assets/shaders/spv/frag.spv");
        shaderManager->AddShader("skybox", "assets/shaders/spv/skybox_vert.spv", "assets/shaders/spv/skybox_frag.spv");
        shaderManager->AddShader("2d", "assets/shaders/spv/2d_vert.spv", "assets/shaders/spv/2d_frag.spv");
        shaderManager->AddShader("ambient_light", "assets/shaders/spv/ambient_shader_vert.spv", "assets/shaders/spv/ambient_shader_frag.spv");

        textureManager->AddTexture("minecraft_grass", "assets/mesh/minecraft/Grass_Block_TEX.png");
        textureManager->AddTexture("minecraft_water", "assets/mesh/minecraft/water.jpg");
        textureManager->AddTexture("campfire_tex", "assets/mesh/campfire/Campfire_MAT_BaseColor_01.jpg");
        textureManager->AddTexture("tree_tex", "assets/mesh/tree/tree.jpg");
        textureManager->AddTexture("tree_top_tex", "assets/mesh/tree/tree.png");
        textureManager->AddTexture("crosshair", "assets/texture/crosshair/simple_crosshair.png");
        textureManager->AddTexture("moon", "assets/mesh/moon/diffuse.jpg");
        textureManager->AddTexture("trunk_tree_cartoon", "assets/mesh/tree/cartoon/Trunk_4_Cartoon.jpg");
        textureManager->AddTexture("grass", "assets/mesh/grass/grass.png");
        textureManager->AddTexture("rooftiles", "assets/mesh/house/textures/rooftiles/T_darkwood_basecolor.png");
        textureManager->AddTexture("dark_wood", "assets/mesh/house/textures/wood/T_darkwood_basecolor.png");
        textureManager->AddTexture("bright_wood", "assets/mesh/house/textures/wood/T_brightwood_basecolor.png");
        textureManager->AddTexture("rocks", "assets/mesh/house/textures/rocks/rock_bc.jpg");
        textureManager->AddTexture("dog_base_color", "assets/mesh/doghouse/doghouse0908_PBR_BaseColor.png");

        glm::vec3 scaleMinecraft = glm::vec3(0.1f, 0.1f, 0.1f);

        for (int x = -7; x < 7; x++) {
            for (int y = -5; y < 15; y++) {
                glm::vec3 posCube = glm::vec3(-0.20f * x, 0.0f, -0.20f * y);
                meshManager->AddMesh("ground_cube", "assets/mesh/minecraft/Grass_Block.obj", { "minecraft_grass" }, "main", posCube, scaleMinecraft);
            }
        }

        for (int x = -22; x < -7; x++) {
            for (int y = -5; y < 15; y++) {
                glm::vec3 posCube = glm::vec3(-0.20f * x, 0.0f, -0.20f * y);
                meshManager->AddMesh("water_cube", "assets/mesh/minecraft/Grass_Block.obj", { "minecraft_water" }, "main", posCube, scaleMinecraft);
            }
        }

        for (int x = 7; x < 22; x++) {
            for (int y = -5; y < 15; y++) {
                glm::vec3 posCube = glm::vec3(-0.20f * x, 0.0f, -0.20f * y);
                meshManager->AddMesh("water_cube_2", "assets/mesh/minecraft/Grass_Block.obj", { "minecraft_water" }, "main", posCube, scaleMinecraft);
            }
        }

        for (int x = -22; x < 22; x++) {
            for (int y = 15; y < 22; y++) {
                glm::vec3 posCube = glm::vec3(-0.20f * x, 0.0f, -0.20f * y);
                meshManager->AddMesh("water_cube_3", "assets/mesh/minecraft/Grass_Block.obj", { "minecraft_water" }, "main", posCube, scaleMinecraft);
            }
        }

        for (int x = -22; x < 22; x++) {
            for (int y = 22; y < 29; y++) {
                glm::vec3 posCube = glm::vec3(-0.20f * x, 0.0f, -0.20f * y);
                meshManager->AddMesh("water_cube_4", "assets/mesh/minecraft/Grass_Block.obj", { "minecraft_water" }, "main", posCube, scaleMinecraft);
            }
        }

        glm::vec3 pos1 = glm::vec3(0.8f, 0.2f, -0.4f);
        glm::vec3 scaleCamp = glm::vec3(0.002f, 0.002f, 0.002f);
        meshManager->AddMesh("campfire", "assets/mesh/campfire/Campfire.obj", { "campfire_tex" }, "main", pos1, scaleCamp);

        glm::vec3 pos2 = glm::vec3(-1.0f, 0.19f, -1.4f);
        glm::vec3 scaleDeadTree = glm::vec3(0.01f, 0.01f, 0.01f);
        meshManager->AddMesh("dead_tree", "assets/mesh/tree/dead_tree.obj", { "trunk_tree_cartoon" }, "main", pos2, scaleDeadTree, glm::vec3(0.0f, 1.0f, 0.0f), 20.0f);

        pos2 = glm::vec3(1.0f, 0.19f, 1.0f);
        meshManager->AddMesh("dead_tree", "assets/mesh/tree/dead_tree.obj", { "trunk_tree_cartoon" }, "main", pos2, scaleDeadTree, glm::vec3(0.0f, 1.0f, 0.0f), -20.0f);

        pos2 = glm::vec3(1.0f, 0.18f, -2.0f);
        meshManager->AddMesh("dead_tree", "assets/mesh/tree/dead_tree.obj", { "trunk_tree_cartoon" }, "main", pos2, scaleDeadTree);

        glm::vec3 scaleTree = glm::vec3(0.0008f, 0.0008f, 0.0008f);
        pos2 = glm::vec3(-0.7f, 0.19f, -1.9f);
        meshManager->AddMesh("tree", "assets/mesh/tree/tree.obj", { "tree_top_tex", "trunk_tree_cartoon" }, "main", pos2, scaleTree, glm::vec3(0.0f, 1.0f, 0.0f), 20.0f);

        pos2 = glm::vec3(-0.25f, 0.19f, -1.2f);
        meshManager->AddMesh("tree", "assets/mesh/tree/tree.obj", { "tree_top_tex", "trunk_tree_cartoon" }, "main", pos2, scaleTree, glm::vec3(0.0f, 1.0f, 0.0f), 10.0f);

        pos2 = glm::vec3(0.7f, 0.19f, -1.6f);
        meshManager->AddMesh("tree", "assets/mesh/tree/tree.obj", { "tree_top_tex", "trunk_tree_cartoon" }, "main", pos2, scaleTree, glm::vec3(0.0f, 1.0f, 0.0f), 80.0f);

        pos2 = glm::vec3(1.2f, 0.19f, -0.9f);
        meshManager->AddMesh("tree", "assets/mesh/tree/tree.obj", { "tree_top_tex", "trunk_tree_cartoon" }, "main", pos2, scaleTree);

        glm::vec3 moonCubeTest = glm::vec3(0.2f, 0.2f, 0.2f);
        glm::vec3 pos6 = glm::vec3(0.5f, 4.5f, -3.00f);
        meshManager->AddMesh("moon", "assets/mesh/moon/moon.obj", { "moon" }, "ambient_light", pos6, moonCubeTest, glm::vec3(1.0f), 0.0f, false);

        glm::vec3 houseScale = glm::vec3(0.1f, 0.1f, 0.1f);
        glm::vec3 posHouse = glm::vec3(-0.5f, 0.19f, -0.20f);
        glm::vec3 rotHouse = glm::vec3(0.0f, 1.0f, 0.0f);
        meshManager->AddMesh("house", "assets/mesh/house/midpoly_town_house_01.obj", { "dark_wood", "rocks", "rooftiles", "bright_wood" }, "main", posHouse, houseScale, rotHouse, -80.0f, false);

        glm::vec3 dogScale = glm::vec3(0.3f, 0.3f, 0.3f);
        glm::vec3 posDog = glm::vec3(0.7f, 0.20f, -2.0f);
        glm::vec3 rotDog = glm::vec3(0.0f, 1.0f, 0.0f);
        meshManager->AddMesh("dog_house", "assets/mesh/doghouse/doghouse0908.obj", { "dog_base_color" }, "main", posDog, dogScale, rotDog, -30.0f);

        std::vector<std::string>skyboxImages;
        skyboxImages.emplace_back("assets/texture/skybox/bluesky/right.jpg");
        skyboxImages.emplace_back("assets/texture/skybox/bluesky/left.jpg");
        skyboxImages.emplace_back("assets/texture/skybox/bluesky/top.jpg");
        skyboxImages.emplace_back("assets/texture/skybox/bluesky/bottom.jpg");
        skyboxImages.emplace_back("assets/texture/skybox/bluesky/front.jpg");
        skyboxImages.emplace_back("assets/texture/skybox/bluesky/back.jpg");
        textureManager->AddSkyBox(skyboxImages);
     
#ifdef RBK_DEBUG
        vulkanLayer = std::make_shared<Rbk::VulkanLayer>();
        layerManager->Add(vulkanLayer.get());

        VImGuiInfo imguiInfo = rendererAdapter->GetVImGuiInfo();
        Rbk::Im::Init(window.get()->Get(), imguiInfo.info, imguiInfo.rdrPass);

        rendererAdapter->ImmediateSubmit([&](VkCommandBuffer cmd) {
            ImGui_ImplVulkan_CreateFontsTexture(cmd);
        });

        ImGui_ImplVulkan_DestroyFontUploadObjects();

        vulkanLayer->AddWindow(window.get());
        vulkanLayer->AddTextureManager(textureManager.get());
        vulkanLayer->AddMeshManager(meshManager.get());
        vulkanLayer->AddShaderManager(shaderManager.get());
#endif

        rendererAdapter->PrepareWorld();

        double lastTime = glfwGetTime();
        double timeStepSum = 0.0f;
        uint32_t frameCount = 0;
        double maxFPS = 60.0;
        double maxPeriod = 1.0 / maxFPS;

        while (!glfwWindowShouldClose(window.get()->Get())) {

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

                if (1.0f <= timeStepSum) {
                    Rbk::Log::GetLogger()->debug("{} fps/sec", frameCount);
                    timeStepSum = 0.0f;
                    frameCount = 0;
                }

                camera->UpdateSpeed(timeStep);

                glfwPollEvents();
                renderManager->SetDeltatime(timeStep);
                renderManager->Draw();

#ifdef RBK_DEBUG
                //@todo move to LayerManager
                Rbk::Im::NewFrame();

                vulkanLayer->AddRenderAdapter(rendererAdapter.get());
                vulkanLayer->Render(timeStep, rendererAdapter->Rdr()->GetDeviceProperties());

                Rbk::Im::Render();

                rendererAdapter->Rdr()->BeginCommandBuffer(imguiInfo.cmdBuffer);
                ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imguiInfo.cmdBuffer);
                rendererAdapter->Rdr()->EndCommandBuffer(imguiInfo.cmdBuffer);
                //end @todo
#endif
                lastTime = currentTime;
            }
        }

#ifdef RBK_DEBUG
        Rbk::Im::Destroy();
#endif
        renderManager->Adp()->Destroy();

        glfwDestroyWindow(window.get()->Get());
        glfwTerminate();
    }
}
