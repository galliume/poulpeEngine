module;

#include <tcl.h>

export module Editor.Managers.LevelManager;

import Engine.Managers.RenderManager;

namespace Poulpe {

  export class LevelManager
  {
    public:
      LevelManager() = default;
      ~LevelManager() = default;

      void registerCommand(
        Tcl_Interp * tcl_interp,
        RenderManager const * render_manager);
  };
}
