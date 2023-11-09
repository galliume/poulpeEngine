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
        explicit InputManager(std::shared_ptr<Window> window);
        virtual void init(nlohmann::json inputConfig) override;

        void key(int key, int scancode, int action, int mods);
        void mouse(double xPos, double yPo);
        void mouseButton(int button, int action, int mods);

        void setCamera(std::shared_ptr<Camera> camera) { m_Camera = camera; };
        void addRenderer(std::shared_ptr<Poulpe::VulkanAdapter> adapter) { m_Adapter = adapter; };

        static bool m_CanMoveCamera;
        static bool m_FirtMouseMove;

    private:
        std::shared_ptr<Window> m_Window;
        std::shared_ptr<Camera> m_Camera;
        std::shared_ptr<VulkanAdapter> m_Adapter;
        float m_LastX;
        float m_LastY;

        nlohmann::json m_InputConfig;
        std::unordered_map<std::string, int> m_KeyboardKeys{};
    };
}
