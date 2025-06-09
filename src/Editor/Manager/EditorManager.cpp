module;

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <tcl.h>
#include <tk.h>

#include <algorithm>
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

  int plp_test_callback(
    ClientData clientData,
    Tcl_Interp* interp,
    int argc,
    const char* argv[])
  {
    Tcl_SetResult(interp, const_cast<char*>("Hello from C++!"), TCL_STATIC);
    return TCL_OK;
  }

  int plp_get_scene(
    ClientData clientData,
    Tcl_Interp* interp,
    int argc,
    const char* argv[])
  {
    Logger::debug("argv[1]: {}", argv[1]);
    auto * render_manager = static_cast<RenderManager*>(clientData);
    HWND glfw_hwnd = glfwGetWin32Window(render_manager->getWindow()->get());

    uintptr_t hwnd_value = std::strtoull(argv[1], nullptr, 10);
    HWND tk_hwnd = reinterpret_cast<HWND>(hwnd_value);

    bool result{true};

    //HWND tk_hwnd = GetParent(frame);

    // if (!IsWindow(tk_hwnd)) {
    //   Logger::error("Invalid HWND passed from Tcl");
    //   return TCL_ERROR;
    // }

    RECT tk_rect;
    WINDOWINFO info; 
    result = GetWindowInfo(tk_hwnd, &info);
    if (!result) {
      DWORD error = GetLastError();
      Logger::error("GetClientRect tk_hwnd error: {}", std::to_string(error));
    }
    int tk_width = 1280;
    int tk_height = 720;

    RECT glfw_rect;
    result = GetWindowRect(glfw_hwnd, &glfw_rect);
    if (!result) {
      DWORD error = GetLastError();
      Logger::error("GetWindowRect glfw_rect error: {}", std::to_string(error));
    }

    int glfw_width = glfw_rect.right - glfw_rect.left;
    int glfw_height = glfw_rect.bottom - glfw_rect.top;

    int64_t x = 0L;
    int64_t y = 0L;

    result = SetParent(glfw_hwnd, tk_hwnd);
    if (!result) {
      DWORD error = GetLastError();
      Logger::error("SetParent error: {}", std::to_string(error));
    }

    result = SetWindowPos(glfw_hwnd, HWND_TOP, x, y, glfw_width, glfw_height, SWP_SHOWWINDOW);
    if (!result) {
      DWORD error = GetLastError();
      Logger::error("SetWindowPos error: {}", std::to_string(error));
    }

    return TCL_OK;
  }

  EditorManager::EditorManager(
    Application const * app
  ) : _app(app)
  {
    _tcl_interp = Tcl_CreateInterp();

    if (Tcl_Init(_tcl_interp) != TCL_OK) {
      Logger::error("Could not init tcl_interp : {}", Tcl_GetStringResult(_tcl_interp));
      return;
    }
    if (Tk_Init(_tcl_interp) != TCL_OK) {
      Logger::error("Could not init tcl_interp : {}", Tcl_GetStringResult(_tcl_interp));
      return;
    }

    if (Tcl_EvalFile(_tcl_interp, "./src/Editor/Scripts/main.tcl") != TCL_OK) {
      Logger::error("Could not init tcl_interp : {}", Tcl_GetStringResult(_tcl_interp));
      return;
    }

    _init = true;

    _level_manager = std::make_unique<LevelManager>();
    _level_manager->registerCommand(_tcl_interp, _app->getRenderManager());

    Tcl_CreateCommand(_tcl_interp,
                  "plp_test_callback",
                  plp_test_callback,
                  const_cast<RenderManager*>(_app->getRenderManager()),
                  nullptr);

    Tcl_CreateCommand(_tcl_interp,
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
