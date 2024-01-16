#include "Application.hpp"

#include "PoulpeEngineConfig.h"

#include "Poulpe/Manager/InputManager.hpp"

#include <GLFW/glfw3.h>

//@todo find a better way than this
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdouble-promotion"
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wcast-align"
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#pragma clang diagnostic ignored "-Wcomma"
#pragma clang diagnostic ignored "-Wformat"
#pragma clang diagnostic ignored "-Wswitch-enum"
#pragma clang diagnostic ignored "-Wfloat-equal"
#pragma clang diagnostic ignored "-Wextra-semi-stmt"
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wundef"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wtautological-type-limit-compare"
#pragma clang diagnostic ignored "-Wcast-function-type-strict"
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#pragma clang diagnostic pop

#include <thread>

namespace Poulpe
{
    unsigned int Application::s_MaxFPS{ 60 };
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

        m_APIManager = std::make_unique<APIManager>();
        m_NetworkManager = std::make_unique<NetworkManager>(m_APIManager.get());
    }

    void Application::run()
    {
        auto endRun = std::chrono::steady_clock::now();
        std::chrono::duration<float> loadedTime = endRun - m_StartRun;
        PLP_TRACE("Started in {} seconds", loadedTime.count());

        auto lastTime = std::chrono::steady_clock::now();

        while (!glfwWindowShouldClose(m_RenderManager->getWindow()->get())) {

            auto frameTarget = (1.0f / (static_cast<float>(s_MaxFPS) * 0.001f));
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
