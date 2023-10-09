#include "Poulpe/GUI/Window.hpp"

#include "stb_image.h"

namespace Poulpe
{
    bool Window::m_FramebufferResized = false;

    void Window::Init(std::string_view windowTitle)
    {
        const uint32_t WIDTH = 2048;
        const uint32_t HEIGHT = 1080;

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        //glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
        //glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
#ifdef PLP_DEBUG_BUILD
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
#endif

        m_Window = glfwCreateWindow(WIDTH, HEIGHT, windowTitle.data(), nullptr, nullptr);

        //GLFWimage images[1]; 
        //images[0].pixels = stbi_load("./assets/mpoulpe.png", &images[0].width, &images[0].height, 0, 4);
        //glfwSetWindowIcon(m_Window, 1, images); 
        //stbi_image_free(images[0].pixels);

        glfwSetWindowSizeLimits(m_Window, 800, 600, 2048, 1080);
          
        const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        const int maxWidth = mode->width;
        const int maxHeight = mode->height; 

        glfwSetWindowMonitor(m_Window, NULL, (maxWidth/2)-(WIDTH/2), (maxHeight/2) - (HEIGHT/2), WIDTH, HEIGHT, GLFW_DONT_CARE);
        glfwSetWindowUserPointer(m_Window, this);

        glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* glfwWindow, int width, int height) {
            Poulpe::Window::m_FramebufferResized = true;
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

    void Window::Hide()
    {
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    }
}
