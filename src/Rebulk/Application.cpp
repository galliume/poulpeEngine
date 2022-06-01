#include "rebulkpch.h"
#include "Application.h"

namespace Rbk
{
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

        textureManager->AddTexture("minecraft_grass", "assets/mesh/minecraft/Grass_Block_TEX.png");
        textureManager->AddTexture("minecraft_water", "assets/mesh/minecraft/water.jpg");
        textureManager->AddTexture("campfire_tex", "assets/mesh/campfire/Campfire_MAT_BaseColor_01.jpg");
        textureManager->AddTexture("tree_tex", "assets/mesh/tree/tree.jpg");
        textureManager->AddTexture("tree_top_tex", "assets/mesh/tree/tree.png");
        textureManager->AddTexture("crosshair", "assets/texture/crosshair/simple_crosshair.png");
        textureManager->AddTexture("moon", "assets/mesh/moon/diffuse.jpg");
        textureManager->AddTexture("trunk_tree_cartoon", "assets/mesh/tree/cartoon/Trunk_4_Cartoon.jpg");
        textureManager->AddTexture("grass", "assets/mesh/grass/grass.png");

        glm::vec3 scaleMinecraft = glm::vec3(0.1f, 0.1f, 0.1f);

        for (int x = -7; x < 7; x++) {
            for (int y = -5; y < 15; y++) {
                glm::vec3 posCube = glm::vec3(-0.20f * x, 0.0f, -0.20f * y);
                meshManager->AddWorldMesh("ground_cube", "assets/mesh/minecraft/Grass_Block.obj", { "minecraft_grass" }, posCube, scaleMinecraft);
            }
        }

        for (int x = -22; x < -7; x++) {
            for (int y = -5; y < 15; y++) {
                glm::vec3 posCube = glm::vec3(-0.20f * x, 0.0f, -0.20f * y);
                meshManager->AddWorldMesh("water_cube", "assets/mesh/minecraft/Grass_Block.obj", { "minecraft_water" }, posCube, scaleMinecraft);
            }
        }

        for (int x = 7; x < 22; x++) {
            for (int y = -5; y < 15; y++) {
                glm::vec3 posCube = glm::vec3(-0.20f * x, 0.0f, -0.20f * y);
                meshManager->AddWorldMesh("water_cube_2", "assets/mesh/minecraft/Grass_Block.obj", { "minecraft_water" }, posCube, scaleMinecraft);
            }
        }

        for (int x = -22; x < 22; x++) {
            for (int y = 15; y < 22; y++) {
                glm::vec3 posCube = glm::vec3(-0.20f * x, 0.0f, -0.20f * y);
                meshManager->AddWorldMesh("water_cube_3", "assets/mesh/minecraft/Grass_Block.obj", { "minecraft_water" }, posCube, scaleMinecraft);
            }
        }

        for (int x = -22; x < 22; x++) {
            for (int y = 22; y < 29; y++) {
                glm::vec3 posCube = glm::vec3(-0.20f * x, 0.0f, -0.20f * y);
                meshManager->AddWorldMesh("water_cube_4", "assets/mesh/minecraft/Grass_Block.obj", { "minecraft_water" }, posCube, scaleMinecraft);
            }
        }

        glm::vec3 pos1 = glm::vec3(0.25f, 0.2f, -0.75f);
        glm::vec3 scaleCamp = glm::vec3(0.002f, 0.002f, 0.002f);
        meshManager->AddWorldMesh("campfire", "assets/mesh/campfire/Campfire.obj", { "campfire_tex" }, pos1, scaleCamp);

        glm::vec3 pos2 = glm::vec3(-1.0f, 0.19f, -1.4f);
        glm::vec3 scaleDeadTree = glm::vec3(0.01f, 0.01f, 0.01f);
        meshManager->AddWorldMesh("dead_tree", "assets/mesh/tree/dead_tree.obj", { "trunk_tree_cartoon" }, pos2, scaleDeadTree);

        pos2 = glm::vec3(1.0f, 0.19f, 1.0f);
        meshManager->AddWorldMesh("dead_tree", "assets/mesh/tree/dead_tree.obj", { "trunk_tree_cartoon" }, pos2, scaleDeadTree);

        pos2 = glm::vec3(1.0f, 0.18f, -2.0f);
        meshManager->AddWorldMesh("dead_tree", "assets/mesh/tree/dead_tree.obj", { "trunk_tree_cartoon" }, pos2, scaleDeadTree);

        glm::vec3 scaleTree = glm::vec3(0.0008f, 0.0008f, 0.0008f);
        pos2 = glm::vec3(-0.7f, 0.19f, -1.9f);
        meshManager->AddWorldMesh("tree", "assets/mesh/tree/tree.obj", { "tree_top_tex", "trunk_tree_cartoon" }, pos2, scaleTree);

        pos2 = glm::vec3(-0.25f, 0.19f, -1.2f);
        meshManager->AddWorldMesh("tree", "assets/mesh/tree/tree.obj", { "tree_top_tex", "trunk_tree_cartoon" }, pos2, scaleTree);

        pos2 = glm::vec3(0.7f, 0.19f, -1.6f);
        meshManager->AddWorldMesh("tree", "assets/mesh/tree/tree.obj", { "tree_top_tex", "trunk_tree_cartoon" }, pos2, scaleTree);

        pos2 = glm::vec3(1.2f, 0.19f, -0.9f);
        meshManager->AddWorldMesh("tree", "assets/mesh/tree/tree.obj", { "tree_top_tex", "trunk_tree_cartoon" }, pos2, scaleTree);

        glm::vec3 scalegrass = glm::vec3(0.05f, 0.05f, 0.05f);

        //pos2 = glm::vec3(0.2f, 0.19f, -0.9f);
        //meshmanager->addworldmesh("grass", "assets/mesh/grass/grass.obj", "grass", pos2, scalegrass);
        //pos2 = glm::vec3(1.2f, 0.19f, 0.9f);
        //meshmanager->addworldmesh("grass", "assets/mesh/grass/grass.obj", "grass", pos2, scalegrass);
        //pos2 = glm::vec3(0.8f, 0.19f, -1.9f);
        //meshmanager->addworldmesh("grass", "assets/mesh/grass/grass.obj", "grass", pos2, scalegrass);
        //pos2 = glm::vec3(-0.2f, 0.19f, 1.0f);
        //meshmanager->addworldmesh("grass", "assets/mesh/grass/grass.obj", "grass", pos2, scalegrass);

 /*       glm::vec3 scalecubetest = glm::vec3(0.10f, 0.10f, 0.10f);
        glm::vec3 pos5 = glm::vec3(0.5f, -1.3f, -0.75f);
        meshmanager->addworldmesh("backpack", "assets/mesh/backpack/backpack.obj", "backpack", pos5, scalecubetest, false);*/

        glm::vec3 moonCubeTest = glm::vec3(0.2f, 0.2f, 0.2f);
        glm::vec3 pos6 = glm::vec3(0.5f, 4.5f, -3.00f);
        meshManager->AddWorldMesh("moon", "assets/mesh/moon/moon.obj", { "moon" }, pos6, moonCubeTest, false);

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
        const double maxFPS = 60.0;
        const double maxPeriod = 1.0 / maxFPS;

        while (!glfwWindowShouldClose(window.get()->Get())) {

            double currentTime = glfwGetTime();
            double timeStep = currentTime - lastTime;

            if (timeStep >= maxPeriod) {
                
                timeStepSum += timeStep;
                frameCount++;

                if (1.0f <= timeStepSum) {
                    Rbk::Log::GetLogger()->debug("{} fps/sec", frameCount);
                    timeStepSum = 0.0f;
                    frameCount = 0;
                }

                camera->UpdateSpeed(timeStep);

                glfwPollEvents();

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
