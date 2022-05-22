#pragma once
#include <GLFW/glfw3.h>

namespace Rbk
{
    class Window
    {
    public:
        void Init();
        inline GLFWwindow* Get() { return m_Window; };
        inline void SetVSync(bool active) { m_ActiveVSync = active; };
        static bool m_FramebufferResized;

    private:
        GLFWwindow* m_Window;
        bool m_ActiveVSync = true;
    };
}
