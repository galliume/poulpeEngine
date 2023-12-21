#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Poulpe
{
    class Window
    {
    public:
        Window() = default;
        //m_Window: delete is done in Application.cpp with glfwFunction

        inline GLFWwindow* get() const { return m_Window; }

        inline void setVSync(bool active) { m_ActiveVSync = active; }

        void hide();
        void init(std::string_view windowTitle);
        bool isMinimized();
        void quit();
        void wait();

        static bool m_FramebufferResized;

    private:
        bool m_ActiveVSync = true;
        GLFWwindow* m_Window = nullptr;
    };
}
