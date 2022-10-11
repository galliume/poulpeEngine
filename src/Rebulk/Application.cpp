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

        m_AudioManager = std::make_shared<Rbk::AudioManager>();
    }

    void Application::Run()
    {
        double startRun = glfwGetTime();

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
        vulkanLayer->AddAudioManager(m_AudioManager);

        m_TextureManager->AddTexture("splashscreen", "assets/splashscreen/splash_1.png");
        m_ShaderManager->AddShader("2d", "assets/shaders/spv/2d_vert.spv", "assets/shaders/spv/2d_frag.spv");

        m_RendererAdapter->PrepareSplashScreen();
        m_RendererAdapter->Draw();

        m_ShaderManager->Load();
        m_TextureManager->Load();
        m_EntityManager->Load();
        m_AudioManager->LoadAmbient();

        m_RendererAdapter->Prepare();
        vulkanLayer->AddRenderAdapter(m_RendererAdapter);

        double endRun = glfwGetTime();
        double lastTime = endRun;
        double timeStepSum = 0.0;
        uint32_t frameCount = 0;
        double maxFPS = 60.0;
        double maxPeriod = 1.0 / maxFPS;
        
        Rbk::Log::GetLogger()->debug("Loaded scene in {}", endRun - startRun);

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


                //@todo move to LayerManager
                Rbk::Im::NewFrame();

                vulkanLayer->Render(timeStep, m_RendererAdapter->Rdr()->GetDeviceProperties());

                Rbk::Im::Render();

                m_RendererAdapter->Rdr()->BeginCommandBuffer(imguiInfo.cmdBuffer);
                ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imguiInfo.cmdBuffer);
                m_RendererAdapter->Rdr()->EndCommandBuffer(imguiInfo.cmdBuffer);
                //end @todo

                lastTime = currentTime;
            }
        }

        m_RendererAdapter->Rdr()->WaitIdle();
        Rbk::Im::Destroy();

        m_RenderManager->Adp()->Destroy();

        glfwDestroyWindow(m_Window.get()->Get());
        glfwTerminate();
    }
}
