module;
#include "PoulpeEngineConfig.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include <GLFW/glfw3.h>
#include <thread>

export module Poulpe.Application;

//import Poulpe.Core.CommandQueueLocator;
import Poulpe.Managers.ConfigManagerLocator;
import Poulpe.Managers.InputManagerLocator;
import Poulpe.GUI.Window;
import Poulpe.Managers;

namespace Poulpe
{
  export class Application
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
