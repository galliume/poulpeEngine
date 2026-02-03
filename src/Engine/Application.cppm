module;
#include "PoulpeEngineConfig.h"

export module Engine.Application;

import std;

import Engine.Core.GLFW;
import Engine.Core.MiniAudio;
import Engine.Core.TclTk;

//import Engine.Core.CommandQueueLocator;
import Engine.Managers.APIManager;
import Engine.Managers.ConfigManagerLocator;
import Engine.Managers.InputManagerLocator;
import Engine.Managers.NetworkManager;
import Engine.Managers.RenderManager;

import Engine.GUI.Window;

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

    Tcl_Interp* _tcl_interp;
  };
}
