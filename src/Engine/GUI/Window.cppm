module;

#include <GLFW/glfw3.h>
#include <string_view>

export module Engine.GUI.Window;

namespace Poulpe
{
  export class Window
  {
  public:
    Window() = default;
    //_window: delete is done in Application.cpp with glfwFunction

    GLFWwindow* get() const;

    void setVSync(bool active);

    void show();
    void init(
      std::string_view windowTitle,
      bool const editor_mode = false);
    bool isMinimized();
    void quit();
    void wait();

    //static bool _FramebufferResized;

  private:
    bool _active_vsync { true};
    GLFWwindow* _window { nullptr };
    GLFWmonitor* _monitor { nullptr };
  };
}
