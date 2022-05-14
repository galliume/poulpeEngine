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
        std::cout << "Application deleted" << std::endl;
    }

    void Application::Init()
    {
        Rbk::Log::Init();

        if (window == nullptr) {
            window = std::make_shared<Rbk::Window>(Rbk::Window());
            window.get()->Init();
        }

        if (camera == nullptr) {
            int width, height;
            glfwGetWindowSize(window->Get(), &width, &height);
            camera = std::make_shared<Rbk::Camera>(Rbk::Camera());
            camera->Init();
        }

        if (inputManager == nullptr) {
            inputManager = std::make_shared<Rbk::InputManager>(window.get(), camera.get());
            inputManager->Init();
        }

        if (layerManager == nullptr) {
            layerManager = std::make_shared<Rbk::LayerManager>();
        }

        if (rendererAdapter == nullptr) {
            rendererAdapter = std::make_shared<Rbk::VulkanAdapter>(window.get());
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
                window->Get(), rendererAdapter.get(), textureManager.get(), meshManager.get(), shaderManager.get()
            );
            renderManager->Init();
            renderManager->AddCamera(camera.get());
        }
    }

    void Application::Run()
    {		
        shaderManager->AddShader("main", "assets/shaders/spv/vert.spv", "assets/shaders/spv/frag.spv");
        shaderManager->AddShader("skybox", "assets/shaders/spv/skybox_vert.spv", "assets/shaders/spv/skybox_frag.spv");
        shaderManager->AddShader("2d", "assets/shaders/spv/2d_vert.spv", "assets/shaders/spv/2d_frag.spv");

        textureManager->AddTexture("minecraft_grass", "assets/mesh/minecraft/Grass_Block_TEX.png");
        textureManager->AddTexture("campfire_tex", "assets/mesh/campfire/Campfire_MAT_BaseColor_01.jpg");
        textureManager->AddTexture("tree_tex", "assets/mesh/tree/tree.jpg");
        textureManager->AddTexture("skybox_tex", "assets/texture/skybox/green/LightGreen_front5.png");
        textureManager->AddTexture("crosshair", "assets/texture/crosshair/simple_crosshair.png");

        glm::vec3 scaleMinecraft = glm::vec3(0.1f, 0.1f, 0.1f);

        for (int x = -5; x < 5; x++) {
            for (int y = 0; y < 10; y++) {
                glm::vec3 posCube = glm::vec3(-0.25f * x, -1.5f, -0.25f * y);
                meshManager->AddWorldMesh("cube", "assets/mesh/minecraft/Grass_Block.obj", "minecraft_grass", posCube, scaleMinecraft);
            }
        }

        glm::vec3 pos1 = glm::vec3(0.25f, -1.3f, -0.75f);
        glm::vec3 scaleCamp = glm::vec3(0.002f, 0.002f, 0.002f);
        meshManager->AddWorldMesh("campfire", "assets/mesh/campfire/Campfire.obj", "campfire_tex", pos1, scaleCamp);

        glm::vec3 pos2 = glm::vec3(-1.0f, -1.3f, -1.4f);
        glm::vec3 scaleTree = glm::vec3(0.0008f, 0.0008f, 0.0008f);
        meshManager->AddWorldMesh("tree", "assets/mesh/tree/tree.obj", "tree_tex", pos2, scaleTree);

        pos2 = glm::vec3(-0.7f, -1.3f, -1.9f);
        meshManager->AddWorldMesh("tree", "assets/mesh/tree/tree.obj", "tree_tex", pos2, scaleTree);

        pos2 = glm::vec3(-0.25f, -1.3f, -1.2f);
        meshManager->AddWorldMesh("tree", "assets/mesh/tree/tree.obj", "tree_tex", pos2, scaleTree);

        pos2 = glm::vec3(0.7f, -1.3f, -1.6f);
        meshManager->AddWorldMesh("tree", "assets/mesh/tree/tree.obj", "tree_tex", pos2, scaleTree);

        pos2 = glm::vec3(1.2f, -1.3f, -0.9f);
        meshManager->AddWorldMesh("tree", "assets/mesh/tree/tree.obj", "tree_tex", pos2, scaleTree);

        glm::vec3 scaleCubeTest = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 pos5 = glm::vec3(0.25f, -1.3f, -0.75f);
        //meshManager->AddWorldMesh("cubeTest", "assets/mesh/cube/cube.obj", "skybox_tex", pos5, scaleCubeTest);

        std::vector<const char*>skyboxImages;
        skyboxImages.emplace_back("assets/texture/skybox/green/LightGreen_right1.png");
        skyboxImages.emplace_back("assets/texture/skybox/green/LightGreen_left2.png");
        skyboxImages.emplace_back("assets/texture/skybox/green/LightGreen_top3.png");
        skyboxImages.emplace_back("assets/texture/skybox/green/LightGreen_bottom4.png");
        skyboxImages.emplace_back("assets/texture/skybox/green/LightGreen_front5.png");
        skyboxImages.emplace_back("assets/texture/skybox/green/LightGreen_back6.png");
        textureManager->AddSkyBox(skyboxImages);

        glm::vec3 pos3 = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 scaleSkybox = glm::vec3(1.0f, 1.0f, 1.0f);
        meshManager->AddSkyboxMesh("skybox", pos3, scaleSkybox);
     
#ifdef RBK_DEBUG
        vulkanLayer = std::make_shared<Rbk::VulkanLayer>();
        layerManager->Add(vulkanLayer.get());

        VImGuiInfo imguiInfo = rendererAdapter->GetVImGuiInfo();
        Rbk::Im::Init(window->Get(), imguiInfo.info, imguiInfo.rdrPass);

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

        while (!glfwWindowShouldClose(window->Get())) {

            double currentTime = glfwGetTime();
            double timeStep = currentTime - lastTime;
            timeStepSum += timeStep;
            frameCount++;

            if (1.0f <= timeStepSum) {
                Rbk::Log::GetLogger()->debug("{} fps/sec", frameCount);
                timeStepSum = 0.0f;
                frameCount = 0;
            }

            camera->UpdateSpeed(timeStep);

            glfwPollEvents();

            renderManager->PrepareDraw();
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

#ifdef RBK_DEBUG
        Rbk::Im::Destroy();
#endif
        renderManager->Adp()->Destroy();

        glfwDestroyWindow(window->Get());
        glfwTerminate();
    }
}
