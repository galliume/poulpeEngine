module;
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

module Engine.GUI.Window;

import std;

//bool _FramebufferResized = false;
namespace Poulpe
{
  GLFWwindow* Window::get() const { return _window; }

  void Window::setVSync(bool active) { _active_vsync = active; }

  void Window::show()
  {
    glfwShowWindow(_window);
  }

  void Window::init(
    std::string_view window_title,
    uint16_t const width,
    uint16_t const height,
    bool const editor_mode)
  {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    if (editor_mode) {
      glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    }

    //@todo check HDR support with GLFW ?
    _monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(_monitor);

    // glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    // glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    // glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    // glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    //_window = glfwCreateWindow(mode->width, mode->height, window_title.data(), _monitor, nullptr);
    _window = glfwCreateWindow(width, height, window_title.data(), nullptr, nullptr);
    //glfwSetWindowSizeLimits(_window, 800, 600, 2560, 1440);

  /* const int maxWidth = mode->width;
    const int maxHeight = mode->height;*/

    GLFWimage icon[1];
    stbi_set_flip_vertically_on_load(false);
    icon[0].pixels = stbi_load("./mpoulpe.png", &icon[0].width, &icon[0].height, nullptr, 4);

    glfwSetWindowIcon(_window, 1, icon);
    stbi_image_free(icon[0].pixels);

    //glfwSetWindowMonitor(_window, nullptr, (maxWidth/2)-(WIDTH/2), (maxHeight/2) - (HEIGHT/2), WIDTH, HEIGHT, GLFW_DONT_CARE);
    glfwSetWindowUserPointer(_window, this);

    glfwSetFramebufferSizeCallback(_window, [](GLFWwindow*, int, int) {
      //_FramebufferResized = true;
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

  //static bool _FramebufferResized;
}
