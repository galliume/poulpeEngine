module;
#include "PoulpeEngineConfig.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include <GLFW/glfw3.h>
#include <thread>

export module Engine.Application;

//import Engine.Core.CommandQueueLocator;
import Engine.Managers.ConfigManagerLocator;
import Engine.Managers.InputManagerLocator;
import Engine.GUI.Window;
import Engine.Managers.APIManager;
import Engine.Managers.NetworkManager;
import Engine.Managers.RenderManager;

namespace Poulpe
{
  export class Application
  {
  public:
    Application() = default;
    ~Application() = default;

    void init(bool const editor_mode);
    void run() const;
    void startServer(std::string const& port);

    RenderManager const * getRenderManager() const { return _render_manager.get(); }

  private:
    std::unique_ptr<APIManager> _api_manager;
    std::unique_ptr<NetworkManager> _network_manager;
    std::unique_ptr<RenderManager> _render_manager;
    std::chrono::time_point<std::chrono::steady_clock> _start_run;
  };
}
