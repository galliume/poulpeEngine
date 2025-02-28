#include "Poulpe/GUI/Window.hpp"

//#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Poulpe
{
  bool Window::_FramebufferResized = false;

  void Window::hide()
  {
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
  }

  void Window::init(std::string_view window_title)
  {
    const int WIDTH = 2560;
    const int HEIGHT = 1440;
    //const int WIDTH = 1024;
    //const int HEIGHT = 720;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    //@todo check HDR support with GLFW ?
    auto _monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode * mode = glfwGetVideoMode(_monitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    //_window = glfwCreateWindow(mode->width, mode->height, window_title.data(), _monitor, nullptr);
    _window = glfwCreateWindow(WIDTH, HEIGHT, window_title.data(), NULL, nullptr);
    //glfwSetWindowSizeLimits(_window, 800, 600, 2560, 1440);

   /* const int maxWidth = mode->width;
    const int maxHeight = mode->height;*/

    //glfwSetWindowMonitor(_window, nullptr, (maxWidth/2)-(WIDTH/2), (maxHeight/2) - (HEIGHT/2), WIDTH, HEIGHT, GLFW_DONT_CARE);
    glfwSetWindowUserPointer(_window, this);

    glfwSetFramebufferSizeCallback(_window, []( GLFWwindow*, int, int) {
        Window::_FramebufferResized = true;
    });
    
    glfwSwapInterval(1);

    //glfwSetInputMode(_window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
    //glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  bool Window::isMinimized()
  {
    int width = 0, height = 0;
    glfwGetFramebufferSize(_window, &width, &height);

    return (width == 0 || height == 0);
  }

  void Window::quit()
  {
    glfwSetWindowShouldClose(_window, true);
  }

  void Window::wait()
  {
    glfwWaitEvents();
  }
}
