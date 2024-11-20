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
    auto const end_run = std::chrono::steady_clock::now();
    std::chrono::duration<float> const loaded_time = end_run - _start_run;
    PLP_TRACE("Started in {} seconds", loaded_time.count());

    auto last_time = std::chrono::steady_clock::now();

    while (!glfwWindowShouldClose(_render_manager->getWindow()->get())) {

      auto const fps_limit = static_cast<unsigned int>(Poulpe::Locator::getConfigManager()->appConfig()["fpsLimit"]);
      auto const frame_target = (1.0f / (static_cast<float>(fps_limit) * 0.001f));
      auto const current_time = std::chrono::steady_clock::now();
      std::chrono::duration<float, std::milli> deltatime = (current_time - last_time);

      if (deltatime.count() < frame_target && fps_limit != 0) continue;
      last_time = current_time;
      
      _render_manager->getCamera()->updateSpeed(deltatime);
      glfwPollEvents();

      //Locator::getCommandQueue()->execPreRequest();
      _render_manager->renderScene(deltatime);
      //Locator::getCommandQueue()->execPostRequest();

      std::stringstream title;
      title << "PoulpeEngine v" << PoulpeEngine_VERSION_MAJOR << "." << PoulpeEngine_VERSION_MINOR
        << " Vulkan version: " << _render_manager->getRenderer()->getAPI()->getAPIVersion()
        << " " << deltatime.count() << " ms"
        << " " << std::ceil(1.f / (deltatime.count() * 0.001f)) << " fps";
      glfwSetWindowTitle(_render_manager->getWindow()->get(), title.str().c_str());
    }
    _render_manager->cleanUp();
  }

  void Application::startServer(std::string const& port)
  {
    _network_manager->startServer(port);
  }
}
