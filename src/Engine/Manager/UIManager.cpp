module Engine.Managers.UIManager;

import std;

import Engine.Core.GLFW;
import Engine.Core.Logger;

import Engine.Managers.RenderManager;

namespace Poulpe
{
  static int plp_quit_game (
    ClientData client_data,
    Tcl_Interp*,
    int,
    Tcl_Obj *const [])
  {
    auto * const window { static_cast<Window*>(client_data) };
    window->quit();

    return TCL_OK;
  }

  UIManager::UIManager(Window& window)
    : _window(window)
  {

  }

  void UIManager::init(std::string const& root_path)
  {
    auto tk_ui = [&, root_path]() {

      _tcl_interp = Tcl_CreateInterp();

      if (Tcl_Init(_tcl_interp) != TCL_OK) {
        Logger::error("Could not init TCL : {}", Tcl_GetStringResult(_tcl_interp));
        return;
      }
      if (Tk_Init(_tcl_interp) != TCL_OK) {
        Logger::error("Could not init TK : {}", Tcl_GetStringResult(_tcl_interp));
        return;
      }

      Tcl_CreateObjCommand(
        _tcl_interp,
        "plp_quit_game",
        plp_quit_game,
        const_cast<Window*>(&_window),
        nullptr);

      int x, y;
      glfwGetWindowPos(_window.getGlfwWindow(), &x, &y);

      glfwSetWindowUserPointer(_window.getGlfwWindow(), this);

      glfwSetWindowPosCallback(_window.getGlfwWindow(),
        [](GLFWwindow* glfw_window, int xpos, int ypos) {

        auto* self { static_cast<UIManager*>(glfwGetWindowUserPointer(glfw_window)) };

        std::string const cmd { std::format("update_ui_pos {} {}", std::to_string(xpos), std::to_string(ypos)) };

        if (Tcl_Eval(self->_tcl_interp, cmd.c_str()) != TCL_OK) {
          std::cout << std::format("Could not update ui pos : {} \n", Tcl_GetStringResult(self->_tcl_interp));
        }
      });

      Tcl_SetVar2(_tcl_interp, "game_menu_x", nullptr, std::to_string(x).c_str(), TCL_GLOBAL_ONLY);
      Tcl_SetVar2(_tcl_interp, "game_menu_y", nullptr, std::to_string(y).c_str(), TCL_GLOBAL_ONLY);

      auto const& tk_scripts_path { root_path + "/src/Engine/GUI/Scripts/main.tcl" };
      
      if (Tcl_EvalFile(_tcl_interp, tk_scripts_path.c_str()) != TCL_OK) {
        Logger::error("Could not init tcl_interp : {}", Tcl_GetStringResult(_tcl_interp));
        return;
      }
      Tk_MainLoop();
    };

    std::thread tk_thread(tk_ui);
    tk_thread.detach();
  }
}
