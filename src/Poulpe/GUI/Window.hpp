#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

namespace Poulpe
{
    struct GLFWWindowDeleter {
      void operator()(GLFWwindow* window) const {
        glfwDestroyWindow(window);
      }
    };

    using UniqueGLFWWindowPtr = std::unique_ptr<GLFWwindow, GLFWWindowDeleter>;

    class Window
    {
    public:
        Window() = default;

        void init(std::string_view windowTitle);
        inline GLFWwindow* get() { return m_Window.get(); }
        inline void setVSync(bool active) { m_ActiveVSync = active; }
        bool isMinimized();
        void wait();
        void quit();
        void hide();

        static bool m_FramebufferResized;

    private:

        UniqueGLFWWindowPtr m_Window;

        bool m_ActiveVSync = true;
    };
}
