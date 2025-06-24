module;

#include <tcl.h>

#include <functional>
#include <memory>

export module Editor.Managers.EditorManager;

import Editor.Managers.ExplorerManager;
import Editor.Managers.LevelManager;

import Engine.Application;

namespace Poulpe {

  export class EditorManager
  {
    public:
      EditorManager(Application const * app);

        ~EditorManager();

    private:
      bool _init {false};
      Tcl_Interp* _tcl_interp;

      Application const * _app;
      std::unique_ptr<LevelManager> _level_manager;
      std::unique_ptr<ExplorerManager> _explorer_manager;
  };
}
