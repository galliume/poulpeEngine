module;
#include <tcl.h>

export module Editor.Managers.ExplorerManager;

import Engine.Managers.RenderManager;

namespace Poulpe {

  export class ExplorerManager
  {
    public:
      ExplorerManager() = default;
      ~ExplorerManager() = default;

      void registerCommand(
        Tcl_Interp * tcl_interp,
        RenderManager const * render_manager);
  };
}
