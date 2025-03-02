#include "Application.hpp"

#include "PoulpeEngineConfig.h"

#include "Core/CommandQueue.hpp"

#include "GUI/Window.hpp"

#include "Poulpe/Manager/ConfigManager.hpp"
#include "Poulpe/Manager/InputManager.hpp"

#include "Renderer/Vulkan/Renderer.hpp"

#include <GLFW/glfw3.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <thread>

namespace Poulpe
{
  Application* Application::_instance{ nullptr };

  Application::Application()
  {
    if (_instance == nullptr) {
      _instance = this;
    }
  }

  void Application::init()
  {
    _start_run = std::chrono::steady_clock::now();

    auto* window = new Window();
    window->init("PoulpeEngine");

    Poulpe::Locator::init(window);

    _render_manager = std::make_unique<RenderManager>(window);
    _render_manager->init();

    _api_manager = std::make_unique<APIManager>(_render_manager.get());
    _network_manager = std::make_unique<NetworkManager>(_api_manager.get());
  }

  void Application::run()
  {
    using namespace std::chrono;

    double const loaded_time{ duration<double>(
      steady_clock::now() - _start_run).count()};

    PLP_TRACE("Started in {} seconds", loaded_time);

    duration<double> const title_rate{1.0};
    duration<double> title_update{ title_rate};

    auto current_time{ steady_clock::now() };

    double accumulator{ 0.0 };
    double total_time{ 0.0 };
    double const dt{ 0.01 };

    double ms_count{0.0};
    double fps_count{0.0};
    size_t frame_count{ 0 };
    auto last_time_debug_updated{ steady_clock::now() };

    FontManager::Text frame_counter {
      .name = "_plp_frame_counter",
      .text = "Frame ",
      .position = glm::vec3(10.0f, 0.0f, 0.0f),
      .color = glm::vec3(1.0f, 1.0f, 0.0f),
    };
    _render_manager->addText(frame_counter);
    FontManager::Text elapsed_time{
      .name = "_plp_elapsed_time",
      .text = "Elapsed time 0",
      .position = glm::vec3(10.0f, 20.0f, 0.0f),
      .color = glm::vec3(1.0f, 1.0f, 0.0f),
    };
    _render_manager->addText(elapsed_time);
    FontManager::Text ms_counter{
      .name = "_plp_ms_counter",
      .text = "0 ms",
      .position = glm::vec3(10.0f, 40.0f, 0.0f),
      .color = glm::vec3(0.5f, 1.0f, 0.0f),
    };
    _render_manager->addText(ms_counter);
    FontManager::Text camera_pos {
      .name = "_plp_camera_pos",
      .text = "Position x: 0.0 y: 0.0 z: 0.0",
      .position = glm::vec3(10.0f, 60.0f, 0.0f),
      .color = glm::vec3(1.0f, 1.0f, 1.0f),
    };
    _render_manager->addText(camera_pos);

    std::stringstream title;
    title << "PoulpeEngine v" << PoulpeEngine_VERSION_MAJOR << "." << PoulpeEngine_VERSION_MINOR
      << " Vulkan version: " << _render_manager->getRenderer()->getAPI()->getAPIVersion();
    glfwSetWindowTitle(_render_manager->getWindow()->get(), title.str().c_str());

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
        total_time += dt;
      }
      _render_manager->updateScene(frame_time);

      //Locator::getCommandQueue()->execPreRequest();
      _render_manager->renderScene();
      //Locator::getCommandQueue()->execPostRequest();

      auto const _elapsed_time{ duration<double>(steady_clock::now() - _start_run).count() };
      Poulpe::Locator::getConfigManager()->setElapsedTime(_elapsed_time);

      //@todo check if it's correct with accumulator method...
      if ((duration<double>(steady_clock::now() - last_time_debug_updated)).count() > 1.0) {
        ms_count = frame_time * 1000.;
        fps_count = 1 / frame_time;
        last_time_debug_updated = steady_clock::now();
      }

      _render_manager->updateText("_plp_ms_counter", std::format("{:<.2f}ms {:<.2f}fps", ms_count, fps_count));
      _render_manager->updateText("_plp_frame_counter", std::format("Frame {:<}", frame_count));
      _render_manager->updateText("_plp_elapsed_time", std::format("Elapsed time {:<.2f}", _elapsed_time));
 
      auto const& camera_pos = _render_manager->getCamera()->getPos();
      _render_manager->updateText("_plp_camera_pos", std::format("Position x: {:<.2f} y: {:<.2f} z: {:<.2f}", camera_pos.x, camera_pos.y, camera_pos.z));

      ++frame_count;

      current_time = new_time;

      if (Poulpe::Locator::getConfigManager()->reload()) {
        _render_manager->cleanUp();
        _render_manager->init();
        Poulpe::Locator::getConfigManager()->setReload(false);
      }
    }
    _render_manager->cleanUp();
  }

  void Application::startServer(std::string const& port)
  {
    _network_manager->startServer(port);
  }
}
