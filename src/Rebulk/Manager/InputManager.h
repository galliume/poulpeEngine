#pragma once
#include "Rebulk/Component/Camera.h"
#include "Rebulk/GUI/Window.h"

namespace Rbk
{
    class InputManager
    {
    public:
        InputManager(std::shared_ptr<Window> window, std::shared_ptr<Camera> camera);
        void Init();
        void Key(int key, int scancode, int action, int mods);
        void Mouse(double xPos, double yPo);
        void MouseButton(int button, int action, int mods);
    private:
        std::shared_ptr<Window> m_Window;
        std::shared_ptr<Camera> m_Camera;
        float m_LastX;
        float m_LastY;
    };
}
