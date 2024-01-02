#include "Application.hpp"

#include "PoulpeEngineConfig.h"

#include "Poulpe/Manager/InputManager.hpp"

namespace Poulpe
{
    float Application::s_MaxFPS{ 0.f };
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
        m_StartRun = std::chrono::steady_clock::now();

        auto* window = new Window();
        window->init("PoulpeEngine");

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
        auto endRun = std::chrono::steady_clock::now();
        std::chrono::duration<float> loadedTime = endRun - m_StartRun;
        PLP_TRACE("Started in {} seconds", loadedTime.count());

        auto lastTime = std::chrono::steady_clock::now();

        while (!glfwWindowShouldClose(m_RenderManager->getWindow()->get())) {

            auto frameTarget = (1.0f / (s_MaxFPS * 0.001f));
            auto currentTime = std::chrono::steady_clock::now();
            std::chrono::duration<float, std::milli> deltaTime = (currentTime - lastTime);

            if (deltaTime.count() < frameTarget && s_MaxFPS != 0) continue;

            m_RenderManager->getCamera()->updateSpeed(deltaTime);
            glfwPollEvents();

            Locator::getCommandQueue()->execPreRequest();
            m_RenderManager->renderScene(deltaTime);
            m_RenderManager->draw();
            Locator::getCommandQueue()->execPostRequest();

            std::stringstream title;
            title << "PoulpeEngine ";
            title << "v" << PoulpeEngine_VERSION_MAJOR << "." << PoulpeEngine_VERSION_MINOR;
            title << " " << deltaTime.count() << " ms";
            title << " " << std::ceil(1 / (deltaTime.count() * 0.001)) << " fps";

            glfwSetWindowTitle(m_RenderManager->getWindow()->get(), title.str().c_str());
            lastTime = currentTime;
        }
        m_RenderManager->cleanUp();
    }
}
