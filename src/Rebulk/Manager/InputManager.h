#pragma once
#include <json.hpp>

#include "Rebulk/Component/Camera.h"
#include "Rebulk/GUI/Window.h"

namespace Rbk
{
    class InputManager
    {
    public:
        explicit InputManager(std::shared_ptr<Window> window, std::shared_ptr<Camera> camera);
        void Init(nlohmann::json inputConfig);
        void Key(int key, int scancode, int action, int mods);
        void Mouse(double xPos, double yPo);
        void MouseButton(int button, int action, int mods);

        static bool m_CanMoveCamera;
        static bool m_FirtMouseMove;

    private:
        std::shared_ptr<Window> m_Window;
        std::shared_ptr<Camera> m_Camera;
        float m_LastX;
        float m_LastY;

        nlohmann::json m_InputConfig;
    };
}
