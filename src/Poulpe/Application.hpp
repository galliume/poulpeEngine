#pragma once

#include "Manager/APIManager.hpp"
#include "Manager/NetworkManager.hpp"
#include "Manager/RenderManager.hpp"

namespace Poulpe
{
  class CommandQueue;
  class Renderer;
  class Window;

  class Application
  {

  public:
    Application();

    inline static Application* get() { return _instance; }
    void init();
    void run();
    void startServer(std::string const& port);

  private:
    static Application* _instance;

    std::unique_ptr<APIManager> _api_manager;
    std::unique_ptr<NetworkManager> _network_manager;
    std::unique_ptr<RenderManager> _render_manager;
    std::chrono::time_point<std::chrono::steady_clock> _start_run;
  };
}
