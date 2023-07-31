#pragma once
#include "IInputManager.hpp"

namespace Poulpe
{
    class InputManager : IInputManager
    {
    public:
        explicit InputManager(std::shared_ptr<Window> window);
        virtual void Init(nlohmann::json inputConfig) override;

        void Key(int key, int scancode, int action, int mods);
        void Mouse(double xPos, double yPo);
        void MouseButton(int button, int action, int mods);

        void SetCamera(std::shared_ptr<Camera> camera) { m_Camera = camera; };
        void AddRenderer(std::shared_ptr<Poulpe::VulkanAdapter> adapter) { m_Adapter = adapter; };

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
