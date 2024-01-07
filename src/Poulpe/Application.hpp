#pragma once

#include "Core/CommandQueue.hpp"

#include "GUI/Window.hpp"

#include "Manager/RenderManager.hpp"

#include "Renderer/IRenderer.hpp"

namespace Poulpe
{
    class Application
    {

    public:
        Application();

        inline static Application* get() { return s_Instance; }
        void init();
        void run();

        static unsigned int s_MaxFPS;

    private:
        static Application* s_Instance;

        std::unique_ptr<RenderManager> m_RenderManager;
        std::chrono::steady_clock::time_point m_StartRun;
    };
}
