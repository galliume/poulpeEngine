#include "Application.hpp"

#include "PoulpeEngineConfig.h"

#include "Poulpe/Manager/InputManager.hpp"

namespace Poulpe
{
    std::atomic<int> Application::s_MaxFPS{ 0 };
    Application* Application::s_Instance{ nullptr };

    Application::Application()
    {
        if (s_Instance == nullptr) {
            s_Instance = this;
        }
    }

    void Application::init()
    {
        Log::init();
        m_StartRun = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());

        auto* window = new Window();
        window->init("PoulpeEngine");

        int width{ 0 };
        int height{ 0 };

        glfwGetWindowSize(window->get(), & width, & height);

        auto* inputManager = new InputManager(window);
        auto* cmdQueue = new CommandQueue();
        auto* threadPool = new ThreadPool();

        Poulpe::Locator::setInputManager(inputManager);
        Poulpe::Locator::setCommandQueue(cmdQueue);
        Poulpe::Locator::setThreadPool(threadPool);

        m_RenderManager = std::make_unique<RenderManager>(window);
        m_RenderManager->init();
    }

    void Application::run()
    {
        auto endRun = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        
        PLP_TRACE("Loaded scene in {}", (endRun - m_StartRun).count());//@todo readable in seconds...

        auto lastTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());

        while (!glfwWindowShouldClose(m_RenderManager->getWindow()->get())) {

            auto frameTarget = (1.0f / (s_MaxFPS.load() / 1000.0f)) / 1000.0f;
            auto currentTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
            auto deltaTime = static_cast<float>(currentTime.time_since_epoch().count() - lastTime.time_since_epoch().count())
                / 1000.0f;

            if (deltaTime < frameTarget && s_MaxFPS.load() != 0) continue;

            //PLP_TRACE("{} ms", deltaTime);
           
            m_RenderManager->getCamera()->updateDeltaTime(deltaTime);

            glfwPollEvents();

            Locator::getCommandQueue()->execPreRequest();
            m_RenderManager->renderScene(deltaTime);
            m_RenderManager->draw();
            Locator::getCommandQueue()->execPostRequest();

            lastTime = currentTime;

            std::stringstream title;
            title << "PoulpeEngine ";
            title << "v" << PoulpeEngine_VERSION_MAJOR << "." << PoulpeEngine_VERSION_MINOR;
            title << " " << deltaTime << " ms";
            title << " " << 1 / deltaTime << " fps";

            glfwSetWindowTitle(m_RenderManager->getWindow()->get(), title.str().c_str());
        }

        m_RenderManager->cleanUp();
    }
}
