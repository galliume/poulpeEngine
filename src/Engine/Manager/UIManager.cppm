export module Engine.Managers.UIManager;

import std;

import Engine.Core.TclTk;

import Engine.GUI.Window;

namespace Poulpe
{
  export class UIManager
  {
  public:
    UIManager(Window & window);
    void init(std::string const& root_path);

  private:
    Window & _window;
    //@todo make unique
    Tcl_Interp* _tcl_interp;
  };
}
