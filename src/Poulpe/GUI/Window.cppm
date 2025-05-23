module;

#include <GLFW/glfw3.h>
#include <string_view>
#include <stb_image.h>

export module Poulpe.GUI.Window;

namespace Poulpe
{
  export class Window
  {
  public:
    Window() = default;
    //_window: delete is done in Application.cpp with glfwFunction

    GLFWwindow* get() const;

    void setVSync(bool active);

    void hide();
    void init(std::string_view windowTitle);
    bool isMinimized();
    void quit();
    void wait();

    static bool _FramebufferResized;

  private:
    bool _ActiveVSync = true;
    GLFWwindow* _window = nullptr;
    GLFWmonitor* _monitor = nullptr;
  };
}
