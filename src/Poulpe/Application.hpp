#pragma once

#include "Core/CommandQueue.hpp"

#include "GUI/Window.hpp"

#include "Manager/APIManager.hpp"
#include "Manager/NetworkManager.hpp"
#include "Manager/RenderManager.hpp"

#include "Renderer/Vulkan/Renderer.hpp"

namespace Poulpe
{
    class Application
    {

    public:
        Application();

        inline static Application* get() { return s_Instance; }
        void init();
        void run();
        void startServer(std::string const& port);

    private:
        static Application* s_Instance;

        std::unique_ptr<APIManager> _APIManager;
        std::unique_ptr<NetworkManager> _NetworkManager;
        std::unique_ptr<RenderManager> _RenderManager;
        std::chrono::steady_clock::time_point _StartRun;
    };
}
