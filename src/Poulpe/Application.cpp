#include "Application.hpp"

#include "PoulpeEngineConfig.h"

#include "Poulpe/Manager/ConfigManager.hpp"
#include "Poulpe/Manager/InputManager.hpp"

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
    Log::init();
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
        _render_manager->getCamera()->updateSpeed(dt);
  
        accumulator -= dt;
        total_time += dt;
      }
      _render_manager->updateScene(frame_time);

      //Locator::getCommandQueue()->execPreRequest();
      _render_manager->renderScene();
      //Locator::getCommandQueue()->execPostRequest();

      auto const elasped_time_since_begining{ duration<double>(
      steady_clock::now() - _start_run).count() };

      //@todo check if it's correct with accumulator method...
      if ((duration<double>(steady_clock::now() - last_time_debug_updated)).count() > 1.0) {
        ms_count = frame_time * 1000.;
        fps_count = 1 / frame_time;
        last_time_debug_updated = steady_clock::now();
      }

      std::stringstream title;
      title << "PoulpeEngine v" << PoulpeEngine_VERSION_MAJOR << "." << PoulpeEngine_VERSION_MINOR
        << " Vulkan version: " << _render_manager->getRenderer()->getAPI()->getAPIVersion()
        << " Frame " << frame_count << " "
        << " Elapsed time " << elasped_time_since_begining << " "
        << " " << ms_count << " ms"
        << " " << fps_count << " fps";
      glfwSetWindowTitle(_render_manager->getWindow()->get(), title.str().c_str());

      ++frame_count;
      
      current_time = new_time;
    }
    _render_manager->cleanUp();
  }

  void Application::startServer(std::string const& port)
  {
    _network_manager->startServer(port);
  }
}
