#include "Poulpe/GUI/Window.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Poulpe
{
    bool Window::m_FramebufferResized = false;

    void Window::hide()
    {
      glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    }

    void Window::init(std::string_view windowTitle)
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

        m_Window = glfwCreateWindow(WIDTH, HEIGHT, windowTitle.data(), nullptr, nullptr);

        glfwSetWindowSizeLimits(m_Window, 800, 600, 2560, 1440);
          
        const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        const int maxWidth = mode->width;
        const int maxHeight = mode->height; 

        glfwSetWindowMonitor(m_Window, nullptr, (maxWidth/2)-(WIDTH/2), (maxHeight/2) - (HEIGHT/2), WIDTH, HEIGHT, GLFW_DONT_CARE);
        glfwSetWindowUserPointer(m_Window, this);

        glfwSetFramebufferSizeCallback(m_Window, []( GLFWwindow* glfwWindow,  int width,  int height) {
            Window::m_FramebufferResized = true;
        });

        //glfwSetInputMode(m_Window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
        //glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    bool Window::isMinimized()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_Window, &width, &height);

        return (width == 0 || height == 0);
    }

    void Window::quit()
    {
        glfwSetWindowShouldClose(m_Window, true);
    }

    void Window::wait()
    {
        glfwWaitEvents();
    }
}
