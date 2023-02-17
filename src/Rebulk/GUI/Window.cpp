#include "Rebulk/GUI/Window.hpp"

namespace Rbk
{
    bool Window::m_FramebufferResized = false;

    void Window::Init()
    {
        const uint32_t WIDTH = 800;
        const uint32_t HEIGHT = 600;

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        
        m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Rebulkan Engine", nullptr, nullptr);

        glfwSetWindowSizeLimits(m_Window, 800, 600, 2560, 1440);
        glfwMakeContextCurrent(m_Window);
        glfwSwapInterval(false);

        glfwSetWindowUserPointer(m_Window, this);

        glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* glfwWindow, int width, int height) {
            Rbk::Window::m_FramebufferResized = true;
        });

        //glfwSetInputMode(m_Window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
        //glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    bool Window::IsMinimized()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_Window, &width, &height);

        return (width == 0 || height == 0);
    }

    void Window::Wait()
    {
        glfwWaitEvents();
    }

    void Window::Quit()
    {
        glfwSetWindowShouldClose(m_Window, true);
    }
}
