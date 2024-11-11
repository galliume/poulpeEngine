#pragma once

#include <GLFW/glfw3.h>

namespace Poulpe
{
  class Window
  {
  public:
    Window() = default;
    //_window: delete is done in Application.cpp with glfwFunction

    inline GLFWwindow* get() const { return _window; }

    inline void setVSync(bool active) { _ActiveVSync = active; }

    void hide();
    void init(std::string_view windowTitle);
    bool isMinimized();
    void quit();
    void wait();

    static bool _FramebufferResized;

  private:
    bool _ActiveVSync = true;
    GLFWwindow* _window = nullptr;
  };
}
