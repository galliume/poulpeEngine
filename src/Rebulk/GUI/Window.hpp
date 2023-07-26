#pragma once
#include <GLFW/glfw3.h>

namespace Rbk
{
    class Window
    {
    public:
        void Init(std::string_view windowTitle);
        inline GLFWwindow* Get() { return m_Window; }
        inline void SetVSync(bool active) { m_ActiveVSync = active; }
        static bool m_FramebufferResized;
        bool IsMinimized();
        void Wait();
        void Quit();
        void Hide();

    private:
        GLFWwindow* m_Window;
        bool m_ActiveVSync = true;
    };
}
