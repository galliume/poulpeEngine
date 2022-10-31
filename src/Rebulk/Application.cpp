#include "rebulkpch.h"
#include "Application.h"

namespace Rbk
{
    int Application::s_UnlockedFPS = 0;
    Application* Application::s_Instance = nullptr;

    Application::Application()
    {
        if (s_Instance == nullptr) {
            s_Instance = this;
        }
    }

    void Application::Init()
    {
        m_StartRun = glfwGetTime();

        Rbk::Log::Init();

        m_Window = std::make_shared<Window>();
        m_Window->Init();

        int width, height;
        glfwGetWindowSize(m_Window->Get(), &width, &height);

        auto adapter = std::make_shared<Rbk::VulkanAdapter>(m_Window);
        auto config = std::make_shared<Rbk::ConfigManager>();
        auto input = std::make_shared<Rbk::InputManager>(m_Window);
        auto audio = std::make_shared<Rbk::AudioManager>();
        auto texture = std::make_shared<Rbk::TextureManager>();
        auto entity = std::make_shared<Rbk::EntityManager>();
        auto shader = std::make_shared<Rbk::ShaderManager>();
        auto sprite = std::make_shared<Rbk::SpriteAnimationManager>();
        auto destroyer = std::make_shared<Rbk::DestroyManager>();
        auto camera = std::make_shared<Rbk::Camera>();

        m_RenderManager = std::make_shared<Rbk::RenderManager>(
            m_Window, adapter, config,
            input, audio, texture,
            entity, shader, sprite,
            destroyer, camera
        );
        m_RenderManager->Init();
    }

    void Application::Run()
    {
        double endRun = glfwGetTime();
        double lastTime = endRun;
        double timeStepSum = 0.0;
        uint32_t frameCount = 0;
        double maxFPS = 60.0;
        double maxPeriod = 1.0 / maxFPS;
  
        ImGuiInfo imguiInfo = m_RenderManager->GetRendererAdapter()->GetImGuiInfo();
        Rbk::Im::Init(m_Window->Get(), imguiInfo.info, imguiInfo.rdrPass);

        m_RenderManager->GetRendererAdapter()->ImmediateSubmit([&](VkCommandBuffer cmd) {
            ImGui_ImplVulkan_CreateFontsTexture(cmd);
        });

        ImGui_ImplVulkan_DestroyFontUploadObjects();

        //todo move to layer manager and update application main loop accordingly
        auto vulkanLayer = std::make_shared<Rbk::VulkanLayer>();
        vulkanLayer->AddRenderManager(m_RenderManager);
        //m_LayerManager->Add(vulkanLayer.get());

        Rbk::Log::GetLogger()->debug("Loaded scene in {}", endRun - m_StartRun);

        while (!glfwWindowShouldClose(m_Window->Get())) {

            if (Application::s_UnlockedFPS == 0) {
                maxFPS = 30.0;
                maxPeriod = 1.0 / maxFPS;
            } else if (Application::s_UnlockedFPS == 1) {
                maxFPS = 60.0;
                maxPeriod = 1.0 / maxFPS;
            } else if (Application::s_UnlockedFPS == 2) {
                maxFPS = 120.0;
                maxPeriod = 1.0 / maxFPS;
            }

            double currentTime = glfwGetTime();
            double timeStep = currentTime - lastTime;

            if (timeStep >= maxPeriod || Application::s_UnlockedFPS == 3) {
                
                timeStepSum += timeStep;
                frameCount++;

                if (1.0 <= timeStepSum) {
                    Rbk::Log::GetLogger()->debug("{} fps/sec", frameCount);
                    timeStepSum = 0.0;
                    frameCount = 0;
                }

                m_RenderManager->GetCamera()->UpdateSpeed(timeStep);

                glfwPollEvents();
                m_RenderManager->SetDeltatime(timeStep);
                m_RenderManager->Draw();

                //@todo move to LayerManager
                Rbk::Im::NewFrame();

                vulkanLayer->Render(
                    timeStep, 
                    m_RenderManager->GetRendererAdapter()->Rdr()->GetDeviceProperties()
                );

                Rbk::Im::Render();

                m_RenderManager->GetRendererAdapter()->Rdr()->BeginCommandBuffer(imguiInfo.cmdBuffer);
                ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imguiInfo.cmdBuffer);
                m_RenderManager->GetRendererAdapter()->Rdr()->EndCommandBuffer(imguiInfo.cmdBuffer);
                //end @todo

                lastTime = currentTime;
            }
        }

        m_RenderManager->GetRendererAdapter()->Rdr()->WaitIdle();
        Rbk::Im::Destroy();

        m_RenderManager->CleanUp();

        glfwDestroyWindow(m_Window.get()->Get());
        glfwTerminate();
    }
}
