#include "Application.hpp"

namespace Poulpe
{
    std::atomic<int> Application::s_UnlockedFPS{ 1 };
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

        Poulpe::Log::Init();

        Poulpe::Locator::setThreadPool(std::make_unique<ThreadPool>());

        m_Window = std::make_shared<Window>();
        m_Window->Init("PoulpeEngine");

        int width, height;
        glfwGetWindowSize(m_Window->Get(), &width, &height);

        m_CommandQueue = std::make_shared<Poulpe::CommandQueue>();

        auto adapter = std::make_shared<Poulpe::VulkanAdapter>(m_Window);
        auto config = std::make_shared<Poulpe::ConfigManager>();
        auto input = std::make_shared<Poulpe::InputManager>(m_Window);
        auto audio = std::make_shared<Poulpe::AudioManager>();
        auto texture = std::make_shared<Poulpe::TextureManager>();
        auto entity = std::make_shared<Poulpe::EntityManager>();
        auto shader = std::make_shared<Poulpe::ShaderManager>();
        auto sprite = std::make_shared<Poulpe::SpriteAnimationManager>();
        auto destroyer = std::make_shared<Poulpe::DestroyManager>();
        auto camera = std::make_shared<Poulpe::Camera>();

        m_RenderManager = std::make_shared<Poulpe::RenderManager>(
            m_Window, adapter, config,
            input, audio, texture,
            entity, shader, sprite,
            destroyer, camera, m_CommandQueue
        );
        m_RenderManager->Init();

        //todo move to layer manager and update application main loop accordingly
        #ifdef PLP_DEBUG_BUILD
            m_VulkanLayer = std::make_shared<Poulpe::VulkanLayer>();
            m_VulkanLayer->AddRenderManager(m_RenderManager.get());
            m_VulkanLayer->Init(m_Window.get(), m_CommandQueue);
        #endif
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

        PLP_DEBUG("Loaded scene in {}", (endRun - m_StartRun).count());//@todo readable in seconds...

        std::mutex mutex;

         while (!glfwWindowShouldClose(m_Window->Get())) {

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

                if (1.0 < timeStepSum.count()) {
                    PLP_DEBUG("{} fps", frameCount);
                    timeStepSum = std::chrono::duration<double>(0.0);
                    frameCount = 0;
                }
                m_RenderManager->GetCamera()->UpdateDeltaTime(timeStep.count());

                glfwPollEvents();

                m_RenderManager->GetRendererAdapter()->ShouldRecreateSwapChain();

                #ifdef PLP_DEBUG_BUILD
                    if (m_VulkanLayer->NeedRefresh()) {
                        m_VulkanLayer->SetNeedRefresh(false);
                        //@todo: tmp, fix imgui update
                        m_VulkanLayer->Destroy();
                        m_VulkanLayer->Init(m_Window.get(), m_CommandQueue);
                        m_VulkanLayer->AddRenderManager(m_RenderManager.get());
                    }

                    m_VulkanLayer->Render(timeStep.count());
                #endif

                m_RenderManager->RenderScene();
                m_RenderManager->Draw();

                lastTime = currentTime;
            }
        }

        #ifdef PLP_DEBUG_BUILD
            Poulpe::Im::Destroy();
        #endif

        m_RenderManager->CleanUp();

        glfwDestroyWindow(m_Window.get()->Get());
        glfwTerminate();
    }
}
