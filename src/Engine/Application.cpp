module;
#include "PoulpeEngineConfig.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

module Engine.Application;

import std;

import Engine.Core.Logger;

import Engine.Managers.DbManager;
import Engine.Managers.FontManager;

namespace Poulpe
{
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
      appConfig["width"].get<uint16_t>(),
      appConfig["height"].get<uint16_t>(),
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
  }

  void Application::run() const
  {
    using namespace std::chrono;

    double const loaded_time{ duration<double>(
      steady_clock::now() - _start_run).count()};

    Logger::trace("Started in {} seconds", loaded_time);

    //duration<double> const title_rate{1.0};
    //duration<double> title_update{ title_rate};

    auto current_time{ steady_clock::now() };

    double accumulator{ 0.0 };
    //double total_time{ 0.0 };
    double const dt{ 0.01 };

    double ms_count{0.0};
    double fps_count{0.0};
    std::size_t frame_count{ 0 };
    auto last_time_debug_updated{ steady_clock::now() };

    FontManager::Text frame_counter {
      .name = "_plp_frame_counter",
      .text = "Frame 000000",
      .position = glm::vec3(10.0f, 20.0f, 0.0f),
      .color = glm::vec3(1.0f, 1.0f, 0.0f),
    };
    _render_manager->addText(frame_counter);
    FontManager::Text elapsed_time{
      .name = "_plp_elapsed_time",
      .text = "Elapsed time 000000",
      .position = glm::vec3(200.0f, 20.0f, 0.0f),
      .color = glm::vec3(1.0f, 1.0f, 0.0f),
    };
    _render_manager->addText(elapsed_time);
    FontManager::Text ms_counter{
      .name = "_plp_ms_counter",
      .text = "000.000 ms 000.000 fps",
      .position = glm::vec3(10.0f, 40.0f, 0.0f),
      .color = glm::vec3(0.5f, 1.0f, 0.0f),
    };
    _render_manager->addText(ms_counter);
    FontManager::Text camera_pos {
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

      glfwSetWindowTitle(_render_manager->getWindow()->get(), title.c_str());

    while (!glfwWindowShouldClose(_render_manager->getWindow()->get())) {

      glfwPollEvents();

      auto const new_time{ steady_clock::now() };
      double frame_time{ duration<double>((new_time - current_time)).count() };

      if (frame_time > 0.25) {
        frame_time = 0.25;
      }
      accumulator += frame_time;

      //cf https://gafferongames.com/post/fix_your_timestep/
      while (accumulator >= dt) {
        _render_manager->getCamera()->updateDeltaTime(dt);
        _render_manager->getCamera()->move();

        accumulator -= dt;
        //total_time += dt;
      }
      //Locator::getCommandQueue()->execPreRequest();
      _render_manager->renderScene(frame_time);
      //Locator::getCommandQueue()->execPostRequest();

      auto const _elapsed_time{ duration<double>(steady_clock::now() - _start_run).count() };

      //@todo check if it's correct with accumulator method...
      if ((duration<double>(steady_clock::now() - last_time_debug_updated)).count() > 1.0) {
        ms_count = frame_time * 1000.;
        fps_count = 1 / frame_time;
        last_time_debug_updated = steady_clock::now();
      }

      _render_manager->updateText("_plp_ms_counter", std::format("{:<.2f}ms {:<.2f}fps", ms_count, fps_count));
      _render_manager->updateText("_plp_frame_counter", std::format("Frame {:<}", frame_count));
      _render_manager->updateText("_plp_elapsed_time", std::format("Elapsed time {:<.2f}", _elapsed_time));
      _render_manager->setElapsedTime(_elapsed_time);

      auto const& current_camera_pos = _render_manager->getCamera()->getPos();
      _render_manager->updateText(
        "_plp_camera_pos",
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
