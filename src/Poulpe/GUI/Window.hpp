#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Poulpe
{
    class Window
    {
    public:
        Window() = default;

        void init(std::string_view windowTitle);
        inline GLFWwindow* get() { return m_Window; }
        inline void setVSync(bool active) { m_ActiveVSync = active; }
        bool isMinimized();
        void wait();
        void quit();
        void hide();

        static bool m_FramebufferResized;

    private:
        GLFWwindow* m_Window = nullptr;
        bool m_ActiveVSync = true;
    };
}
