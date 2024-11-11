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
        //const int WIDTH = 800;
        //const int HEIGHT = 600;

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

        _window = glfwCreateWindow(WIDTH, HEIGHT, window_title.data(), nullptr, nullptr);

        glfwSetWindowSizeLimits(_window, 800, 600, 2560, 1440);
          
        const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        const int maxWidth = mode->width;
        const int maxHeight = mode->height; 

        glfwSetWindowMonitor(_window, nullptr, (maxWidth/2)-(WIDTH/2), (maxHeight/2) - (HEIGHT/2), WIDTH, HEIGHT, GLFW_DONT_CARE);
        glfwSetWindowUserPointer(_window, this);

        glfwSetFramebufferSizeCallback(_window, []( GLFWwindow*, int, int) {
            Window::_FramebufferResized = true;
        });

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
