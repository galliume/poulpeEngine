#include "Application.hpp"

namespace Poulpe
{
    std::atomic<int> Application::s_UnlockedFPS{ 1 };
    Application* Application::s_Instance{ nullptr };

    Application::Application()
    {
        if (s_Instance == nullptr) {
            s_Instance = this;
        }
    }

    void Application::init()
    {
        Poulpe::Log::init();
        m_StartRun = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());

        m_Window = std::make_shared<Window>();
        m_Window->init("PoulpeEngine");

        int width{ 0 };
        int height{ 0 };

        glfwGetWindowSize(m_Window->get(), & width, & height);

        auto input = std::make_shared<Poulpe::InputManager>(m_Window.get());
        auto cmdQueue = std::make_shared<Poulpe::CommandQueue>();

        Poulpe::Locator::setThreadPool(std::make_unique<ThreadPool>());
        Poulpe::Locator::setInputManager(input);
        Poulpe::Locator::setCommandQueue(cmdQueue);

        m_RenderManager = std::make_shared<Poulpe::RenderManager>(m_Window.get());
        m_RenderManager->init();

        //todo move to layer manager and update application main loop accordingly
        //#ifdef PLP_DEBUG_BUILD
            m_VulkanLayer = std::make_shared<Poulpe::VulkanLayer>();
            m_VulkanLayer->addRenderManager(m_RenderManager.get());
            m_VulkanLayer->init(m_Window.get());
        //#endif
    }

    void Application::run()
    {
        auto endRun = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        auto lastTime = endRun;
        auto timeStepSum = std::chrono::duration<double>(0.0);
        uint32_t frameCount = 0;
        double maxFPS = 60.0;
        auto maxPeriod = std::chrono::duration<double>(1.0 / maxFPS);
  
        std::chrono::milliseconds timeStep{0};

        PLP_WARN("Loaded scene in {}", (endRun - m_StartRun).count());//@todo readable in seconds...

        std::mutex mutex;

         while (!glfwWindowShouldClose(m_Window->get())) {

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
                    PLP_WARN("{} fps", frameCount);
                    timeStepSum = std::chrono::duration<double>(0.0);
                    frameCount = 0;
                }
                m_RenderManager->getCamera()->updateDeltaTime(timeStep.count());

                glfwPollEvents();

                m_RenderManager->getRendererAdapter()->shouldRecreateSwapChain();
                Poulpe::Locator::getCommandQueue()->execPreRequest();
                m_VulkanLayer->render(timeStep.count());
                m_RenderManager->renderScene();
                m_RenderManager->draw();
                Poulpe::Locator::getCommandQueue()->execPostRequest();

                lastTime = currentTime;
            }
        }

        Poulpe::Im::destroy();
        m_RenderManager->cleanUp();

        glfwDestroyWindow(m_Window.get()->get());
        glfwTerminate();
    }
}
