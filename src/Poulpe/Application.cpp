#include "Application.hpp"

#include "PoulpeEngineConfig.h"

#include "Poulpe/Manager/ConfigManager.hpp"
#include "Poulpe/Manager/InputManager.hpp"

#include <GLFW/glfw3.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <thread>

namespace Poulpe
{
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

        auto* configManager = new ConfigManager();
        auto* inputManager = new InputManager(window);
        auto* cmdQueue = new CommandQueue();
        auto* threadPool = new ThreadPool();

        Poulpe::Locator::setConfigManager(configManager);
        Poulpe::Locator::setInputManager(inputManager);
        Poulpe::Locator::setThreadPool(threadPool);
        Poulpe::Locator::setCommandQueue(cmdQueue);

        m_RenderManager = std::make_unique<RenderManager>(window);
        m_RenderManager->init();

        m_APIManager = std::make_unique<APIManager>(m_RenderManager.get());
        m_NetworkManager = std::make_unique<NetworkManager>(m_APIManager.get());
    }

    void Application::run()
    {
        auto const endRun = std::chrono::steady_clock::now();
        std::chrono::duration<float> const loadedTime = endRun - m_StartRun;
        PLP_TRACE("Started in {} seconds", loadedTime.count());

        auto lastTime = std::chrono::steady_clock::now();

        while (!glfwWindowShouldClose(m_RenderManager->getWindow()->get())) {

            auto const fpsLimit = static_cast<unsigned int>(Poulpe::Locator::getConfigManager()->appConfig()["fpsLimit"]);
            auto const frameTarget = (1.0f / (static_cast<float>(fpsLimit) * 0.001f));
            auto const currentTime = std::chrono::steady_clock::now();
            std::chrono::duration<float, std::milli> const deltaTime = (currentTime - lastTime);

            if (deltaTime.count() < frameTarget && fpsLimit != 0) continue;

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
            title << " " << std::ceil(1.f / (deltaTime.count() * 0.001f)) << " fps";

            glfwSetWindowTitle(m_RenderManager->getWindow()->get(), title.str().c_str());
            lastTime = currentTime;
        }
        m_RenderManager->cleanUp();
    }

    void Application::startServer(std::string const& port)
    {
      m_NetworkManager->startServer(port);
    }
}
