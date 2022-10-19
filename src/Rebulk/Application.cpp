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
        Rbk::Log::Init();

        m_Window = std::make_shared<Window>();
        m_Window->Init();

        int width, height;
        glfwGetWindowSize(m_Window->Get(), &width, &height);

        m_RenderManager = std::make_shared<Rbk::RenderManager>(
            m_Window,
            std::make_shared<Rbk::VulkanAdapter>(m_Window),
            std::make_shared<Rbk::ConfigManager>(),
            std::make_shared<Rbk::InputManager>(m_Window),
            std::make_shared<Rbk::AudioManager>(),
            std::make_shared<Rbk::TextureManager>(),
            std::make_shared<Rbk::EntityManager>(),
            std::make_shared<Rbk::ShaderManager>(),
            std::make_shared<Rbk::SpriteAnimationManager>(),
            std::make_shared<Rbk::LayerManager>(),
            std::make_shared<Rbk::Camera>()
        );

        m_RenderManager->Init();
    }

    void Application::Run()
    {

        double startRun = glfwGetTime();
        double endRun = glfwGetTime();
        double lastTime = endRun;
        double timeStepSum = 0.0;
        uint32_t frameCount = 0;
        double maxFPS = 60.0;
        double maxPeriod = 1.0 / maxFPS;
        
        Rbk::Log::GetLogger()->debug("Loaded scene in {}", endRun - startRun);

        ImGuiInfo imguiInfo = m_RenderManager->GetRendererAdapter()->GetImGuiInfo();
        Rbk::Im::Init(m_Window->Get(), imguiInfo.info, imguiInfo.rdrPass);

        m_RenderManager->GetRendererAdapter()->ImmediateSubmit([&](VkCommandBuffer cmd) {
            ImGui_ImplVulkan_CreateFontsTexture(cmd);
        });

        ImGui_ImplVulkan_DestroyFontUploadObjects();

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

                m_RenderManager->GetCamera()->UpdateSpeed(timeStep);

                glfwPollEvents();
                m_RenderManager->SetDeltatime(timeStep);
                m_RenderManager->Draw();


                //@todo move to LayerManager
                Rbk::Im::NewFrame();

                m_RenderManager->GetVulkanLayer()->Render(
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

        m_RenderManager->Adp()->Destroy();

        glfwDestroyWindow(m_Window.get()->Get());
        glfwTerminate();
    }
}
