module;

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <tcl.h>
#include <tk.h>

#include <functional>
#include <iostream>
#include <latch>
#include <string>
#include <thread>

module Editor.Managers.EditorManager;

import Editor.Managers.LevelManager;

import Engine.Application;
import Engine.Core.Logger;
import Engine.Component.Components;
import Engine.Managers.AudioManager;
import Engine.Managers.ComponentManager;
import Engine.Managers.DestroyManager;
import Engine.Managers.EntityManager;
import Engine.Managers.FontManager;
import Engine.Managers.LightManager;
import Engine.Managers.ShaderManager;
import Engine.Managers.TextureManager;
import Engine.Managers.RenderManager;
import Engine.Renderer.Mesh;
import Engine.Renderer.RendererComponent;
import Engine.Renderer.RendererComponentTypes;

namespace Poulpe {

  static int plp_test_callback(
    ClientData,
    Tcl_Interp* interp,
    int,
    Tcl_Obj *const [])
  {
    Tcl_SetResult(interp, const_cast<char*>("Hello from C++!"), TCL_STATIC);
    return TCL_OK;
  }

  static int plp_get_scene(
    ClientData clientData,
    Tcl_Interp* interp,
    int,
    Tcl_Obj *const objv[])
  {
    auto * render_manager = static_cast<RenderManager*>(clientData);
    HWND glfw_hwnd = glfwGetWin32Window(render_manager->getWindow()->get());
    HWND tk_hwnd;
    Tcl_GetIntFromObj(interp, objv[1], (int*)&tk_hwnd);

    RECT tk_rect;
    GetClientRect(tk_hwnd, &tk_rect);

    RECT glfw_rect;
    GetWindowRect(glfw_hwnd, &glfw_rect);
    int glfw_width = glfw_rect.right - glfw_rect.left;
    int glfw_height = glfw_rect.bottom - glfw_rect.top;

    SetParent(glfw_hwnd, tk_hwnd);
    SetWindowPos(glfw_hwnd, HWND_TOP, 0, 0, glfw_width, glfw_height, SWP_SHOWWINDOW);

    return TCL_OK;
  }

  EditorManager::EditorManager(
    Application const * app
  ) : _app(app)
  {
    _tcl_interp = Tcl_CreateInterp();

    if (Tcl_Init(_tcl_interp) != TCL_OK) {
      Logger::error("Could not init TCL : {}", Tcl_GetStringResult(_tcl_interp));
      return;
    }
    if (Tk_Init(_tcl_interp) != TCL_OK) {
      Logger::error("Could not init TK : {}", Tcl_GetStringResult(_tcl_interp));
      return;
    }
    if (Tcl_EvalFile(_tcl_interp, "./src/Editor/Scripts/main.tcl") != TCL_OK) {
      Logger::error("Could not init tcl_interp : {}", Tcl_GetStringResult(_tcl_interp));
      return;
    }

    _init = true;

    _level_manager = std::make_unique<LevelManager>();
    _level_manager->registerCommand(_tcl_interp, _app->getRenderManager());

    Tcl_CreateObjCommand(_tcl_interp,
                  "plp_test_callback",
                  plp_test_callback,
                  const_cast<RenderManager*>(_app->getRenderManager()),
                  nullptr);

    Tcl_CreateObjCommand(_tcl_interp,
      "plp_get_scene",
      plp_get_scene,
      const_cast<RenderManager*>(_app->getRenderManager()),
      nullptr);

    std::thread app_thread([&] {
      _app->run();
    });
    app_thread.detach();

    Tk_MainLoop();
  }

  EditorManager::~EditorManager()
  {
    Tcl_DeleteInterp(_tcl_interp);
    Tcl_Finalize();
  }
}
