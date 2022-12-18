#include "rebulkpch.h"
#include "Application.h"
#include <nvToolsExt.h>

namespace Rbk
{
    std::atomic<int> Application::s_UnlockedFPS{ 3 };
    Application* Application::s_Instance = nullptr;

    Application::Application()
    {
        if (s_Instance == nullptr) {
            s_Instance = this;
        }
    }

    void Application::Init()
    {
        m_StartRun = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());

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
        auto endRun = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        auto lastTime = endRun;
        auto timeStepSum = std::chrono::duration<double>(0.0);
        uint32_t frameCount = 0;
        double maxFPS = 60.0;
        auto maxPeriod = std::chrono::duration<double>(1.0 / maxFPS);
  
        std::chrono::milliseconds timeStep{0};

        Rbk::Log::GetLogger()->debug("Loaded scene in {}", (endRun - m_StartRun).count());//@todo readable in seconds...

        std::mutex lockDraw;

        InitImGui();
        auto imgui =
            [=, &timeStep, &lockDraw]() {
            glfwPollEvents();

            //@todo move to LayerManager
            Rbk::Im::NewFrame();

            m_VulkanLayer->Render(
                timeStep.count(),
                m_RenderManager->GetRendererAdapter()->Rdr()->GetDeviceProperties()
            );
            
            Rbk::Im::Render();

            m_RenderManager->GetRendererAdapter()->Rdr()->BeginCommandBuffer(Rbk::Im::GetImGuiInfo().cmdBuffer);
            {
                std::lock_guard<std::mutex> guard(lockDraw);
                ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Rbk::Im::GetImGuiInfo().cmdBuffer);
            }
            m_RenderManager->GetRendererAdapter()->Rdr()->EndCommandBuffer(Rbk::Im::GetImGuiInfo().cmdBuffer);

            if (m_VulkanLayer->NeedRefresh()) {
                m_VulkanLayer->AddRenderManager(m_RenderManager);
            }
        };

        while (!glfwWindowShouldClose(m_Window->Get())) {

            nvtxRangePush(L"Yatangaki");
            nvtxMark((L"Subido la marea ! AHIIII"));

            if (Application::s_UnlockedFPS.load() == 0) {
                maxFPS = 30.0;
                maxPeriod = std::chrono::duration<double>(1.0 / maxFPS);
            } else if (Application::s_UnlockedFPS.load() == 1) {
                maxFPS = 60.0;
                maxPeriod = std::chrono::duration<double>(1.0 / maxFPS);
            } else if (Application::s_UnlockedFPS.load() == 2) {
                maxFPS = 120.0;
                maxPeriod = std::chrono::duration<double>(1.0 / maxFPS);
            }

            auto currentTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
            timeStep = currentTime - lastTime;

            if (timeStep >= maxPeriod || Application::s_UnlockedFPS.load() == 3) {
                
                timeStepSum += timeStep;
                frameCount++;

                if (1.0 <= timeStepSum.count()) {
                    Rbk::Log::GetLogger()->debug("{} fps", frameCount);
                    timeStepSum = std::chrono::duration<double>(0.0);
                    frameCount = 0;
                }

                m_RenderManager->GetCamera()->UpdateSpeed(timeStep.count());

                glfwPollEvents();
                m_RenderManager->SetDeltatime(timeStep.count());
                imgui();
                m_RenderManager->Draw();
                
                lastTime = currentTime;
            }
        }

        m_RenderManager->GetRendererAdapter()->Rdr()->WaitIdle();
        Rbk::Im::Destroy();

        m_RenderManager->CleanUp();

        glfwDestroyWindow(m_Window.get()->Get());
        glfwTerminate();

        nvtxRangePop();
    }

    void Application::InitImGui()
    {
        ImGuiInfo imguiInfo = m_RenderManager->GetRendererAdapter()->GetImGuiInfo();
        Rbk::Im::Init(m_Window->Get(), imguiInfo);

        m_RenderManager->GetRendererAdapter()->ImmediateSubmit([&](VkCommandBuffer cmd) {
            ImGui_ImplVulkan_CreateFontsTexture(cmd);
            });

        ImGui_ImplVulkan_DestroyFontUploadObjects();

        //todo move to layer manager and update application main loop accordingly
        m_VulkanLayer = std::make_shared<Rbk::VulkanLayer>();
        m_VulkanLayer->AddRenderManager(m_RenderManager);
        //m_LayerManager->Add(vulkanLayer.get());
    }
}
