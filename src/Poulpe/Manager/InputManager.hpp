#pragma once

#include "IInputManager.hpp"

#include "Poulpe/Component/Camera.hpp"

#include "Poulpe/GUI/Window.hpp"

namespace Poulpe
{
    class InputManager : IInputManager
    {
    public:
        explicit InputManager(Window* window);

        virtual void init(nlohmann::json inputConfig) override;

        void key(int key, int scancode, int action, int mods);
        void mouseButton(int button, int action, int mods);
        void saveLastMousePos(double xPos, double yPos);
        void inline setCamera(Camera* camera) { m_Camera = camera; };
        void updateMousePos(double xPos, double yPo);

        static bool m_CanMoveCamera;
        static bool m_FirtMouseMove;

    private:
        double m_LastX;
        double m_LastY;

        Camera* m_Camera;
        Window* m_Window;

        nlohmann::json m_InputConfig;
        std::unordered_map<std::string, int> m_KeyboardKeys{};
    };
}
