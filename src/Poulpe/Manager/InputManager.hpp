#pragma once

#include "IInputManager.hpp"

#include "Poulpe/Component/Camera.hpp"
#include "Poulpe/GUI/Window.hpp"

namespace Poulpe
{
    class VulkanAdapter; //@todo replace with interface

    class InputManager : IInputManager
    {
    public:
        explicit InputManager(Window* window);
        virtual void init(nlohmann::json inputConfig) override;

        void key(int key, int scancode, int action, int mods);
        void mouse(double xPos, double yPo);
        void mouseButton(int button, int action, int mods);

        void setCamera(Camera* camera) { m_Camera = camera; };
        void addRenderer(Poulpe::VulkanAdapter* adapter) { m_Adapter = adapter; };

        static bool m_CanMoveCamera;
        static bool m_FirtMouseMove;

    private:
        Window* m_Window;
        Camera* m_Camera;
        VulkanAdapter* m_Adapter;
        float m_LastX;
        float m_LastY;

        nlohmann::json m_InputConfig;
        std::unordered_map<std::string, int> m_KeyboardKeys{};
    };
}
