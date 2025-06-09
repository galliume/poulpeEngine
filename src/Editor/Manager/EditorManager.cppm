module;

#include <tcl.h>

export module Editor.Managers.EditorManager;

import Engine.Managers.RenderManager;

namespace Poulpe {

  export class EditorManager
  {
    public:
      EditorManager(RenderManager const * render_manager);
      ~EditorManager();

      void run();

    private:
      bool _init {false};
      Tcl_Interp* _tcl_interp;

      RenderManager const * _render_manager;
  };
}
