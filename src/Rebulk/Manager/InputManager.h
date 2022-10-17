#pragma once
#include <json.hpp>

#include "Rebulk/Component/Camera.h"
#include "Rebulk/GUI/Window.h"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.h"

namespace Rbk
{
    class InputManager
    {
    public:
        explicit InputManager(std::shared_ptr<Window> window, std::shared_ptr<Camera> camera, std::shared_ptr<Rbk::VulkanAdapter> adapter);
        void Init(nlohmann::json inputConfig);
        void Key(int key, int scancode, int action, int mods);
        void Mouse(double xPos, double yPo);
        void MouseButton(int button, int action, int mods);

        static bool m_CanMoveCamera;
        static bool m_FirtMouseMove;

    private:
        std::shared_ptr<Window> m_Window;
        std::shared_ptr<Camera> m_Camera;
        std::shared_ptr<VulkanAdapter> m_Adapter;
        float m_LastX;
        float m_LastY;

        nlohmann::json m_InputConfig;
    };
}
