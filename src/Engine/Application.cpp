module;
#include "PoulpeEngineConfig.h"

module Engine.Application;

import std;

import Engine.Core.GLFW;
import Engine.Core.GLM;
import Engine.Core.Logger;
import Engine.Core.TclTk;

import Engine.Managers.DbManager;
import Engine.Managers.FontManager;

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

  void Application::init(bool const editor_mode)
  {
    _start_run = std::chrono::steady_clock::now();

    //CommandQueueManagerLocator::init();
    //@todo ugly fix
    auto const root_path { std::filesystem::absolute(__FILE__).parent_path().parent_path().parent_path().string() };

    ConfigManagerLocator::init(root_path);
    auto const& appConfig { ConfigManagerLocator::get()->appConfig()["resolution"] };

    auto* window = new Window();
    window->init("PoulpeEngine",
      root_path + "/vampyropoda.jpg",
      appConfig["width"].get<std::uint16_t>(),
      appConfig["height"].get<std::uint16_t>(),
      editor_mode);

    InputManagerLocator::init(window);

    _render_manager = std::make_unique<RenderManager>(window);
    _render_manager->init();

    _api_manager = std::make_unique<APIManager>();
    _network_manager = std::make_unique<NetworkManager>(_api_manager.get());

    //auto db_manager = std::make_unique<DbManager>();
    if (!editor_mode) {
      _render_manager->getWindow()->show();
    }

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
        const_cast<Window*>(_render_manager->getWindow()),
        nullptr);

      int x, y;
      glfwGetWindowPos(_render_manager->getWindow()->getGlfwWindow(), &x, &y);

      glfwSetWindowUserPointer(_render_manager->getWindow()->getGlfwWindow(), this);

      glfwSetWindowPosCallback(_render_manager->getWindow()->getGlfwWindow(),
        [](GLFWwindow* glfw_window, int xpos, int ypos) {

        auto* self { static_cast<Application*>(glfwGetWindowUserPointer(glfw_window)) };

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

  void Application::run() const
  {
    using namespace std::chrono;

    //duration<double> const title_rate{1.0};
    //duration<double> title_update{ title_rate};

    auto current_time{ steady_clock::now() };

    double accumulator{ 0.0 };
    //double total_time{ 0.0 };
    double const dt{ 0.01 };

    double ms_count{0.0};
    double fps_count{0.0};
    double frame_count{ 0.0 };
    auto last_time_debug_updated{ steady_clock::now() };

    FontManager::Text frame_counter {
      .id = 0,
      .name = "_plp_frame_counter",
      .text = "Frame 000000",
      .position = glm::vec3(10.0f, 20.0f, 0.0f),
      .color = glm::vec3(1.0f, 1.0f, 0.0f),
    };
    _render_manager->addText(frame_counter);
    FontManager::Text elapsed_time {
      .id = 0,
      .name = "_plp_elapsed_time",
      .text = "Elapsed time 000000",
      .position = glm::vec3(200.0f, 20.0f, 0.0f),
      .color = glm::vec3(1.0f, 1.0f, 0.0f),
    };
    _render_manager->addText(elapsed_time);
    FontManager::Text ms_counter{
      .id = 0,
      .name = "_plp_ms_counter",
      .text = "000.000 ms 000.000 fps",
      .position = glm::vec3(10.0f, 40.0f, 0.0f),
      .color = glm::vec3(0.5f, 1.0f, 0.0f),
    };
    _render_manager->addText(ms_counter);
    FontManager::Text camera_pos {
      .id = 0,
      .name = "_plp_camera_pos",
      .text = "Position x: 000.000 y: 000.000 z: 000.000",
      .position = glm::vec3(10.0f, 60.0f, 0.0f),
      .color = glm::vec3(1.0f, 1.0f, 1.0f),
    };
    _render_manager->addText(camera_pos);

    bool release_build { true };

  #ifdef PLP_DEBUG_BUILD
    release_build = false;
  #endif

    std::string const title { std::format("PoulpeEngine v{}.{} Vulkan version: {} {}",
      PoulpeEngine_VERSION_MAJOR, PoulpeEngine_VERSION_MINOR,
      _render_manager->getRenderer()->getAPI()->getAPIVersion(),
      (release_build ? "Release build" : "Debug build")) };

    glfwSetWindowTitle(_render_manager->getWindow()->getGlfwWindow(), title.c_str());

    double const loaded_time { duration<double>(steady_clock::now() - _start_run).count() };
    Logger::trace("Started in {} seconds", loaded_time);

    while (!glfwWindowShouldClose(_render_manager->getWindow()->getGlfwWindow())) {

      while (Tcl_DoOneEvent(TCL_DONT_WAIT)) {}

      glfwPollEvents();

      auto const new_time{ steady_clock::now() };
      double frame_time{ duration<double>((new_time - current_time)).count() };

      if (frame_time > 0.25) {
        frame_time = 0.25;
      }
      accumulator += frame_time;

      //cf https://gafferongames.com/post/fix_your_timestep/
      while (accumulator >= dt) {
        _render_manager->getCamera()->savePreviousState();
        _render_manager->getCamera()->updateDeltaTime(dt);
        _render_manager->getCamera()->move();

        accumulator -= dt;
        //total_time += dt;
      }

      double const alpha { accumulator / dt };
      _render_manager->getCamera()->interpolate(alpha);
      //Locator::getCommandQueue()->execPreRequest();
      _render_manager->renderScene(frame_time);
      //Locator::getCommandQueue()->execPostRequest();

      auto const _elapsed_time{ duration<double>(new_time - _start_run).count() };

      //@todo check if it's correct with accumulator method...
      if ((duration<double>(new_time - last_time_debug_updated)).count() > 1.0) {
        fps_count = frame_count;
        ms_count = 1000.0 / std::max(1.0, static_cast<double>(frame_count));
        last_time_debug_updated = new_time;
        frame_count = 0;
      }

      _render_manager->updateText(ms_counter.id, std::format("{:<.2f}ms {:<.2f}fps", ms_count, fps_count));
      _render_manager->updateText(frame_counter.id, std::format("Frame {:<}", frame_count));
      _render_manager->updateText(elapsed_time.id, std::format("Elapsed time {:<.2f}", _elapsed_time));
      _render_manager->setElapsedTime(_elapsed_time);

      auto const& current_camera_pos = _render_manager->getCamera()->getPos();
      _render_manager->updateText(
        camera_pos.id,
        std::format("Position x: {:<.2f} y: {:<.2f} z: {:<.2f}",
          current_camera_pos.x, current_camera_pos.y, current_camera_pos.z));

      ++frame_count;

      current_time = new_time;

      //if (ConfigManagerLocator::get()->reload()) {
      //   _render_manager->cleanUp();
      //   _render_manager->init();
      //   ConfigManagerLocator::get()->setReload(false);
      // }
    }
    _render_manager->cleanUp();
  }

  void Application::startServer(std::string const& port)
  {
    _network_manager->startServer(port);
  }
}
