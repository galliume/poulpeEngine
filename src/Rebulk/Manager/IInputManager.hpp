#pragma once
#include <json.hpp>

#include "Rebulk/GUI/Window.hpp"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.hpp"

namespace Rbk
{
    class IInputManager
    {
    public:
        IInputManager() = default;
        ~IInputManager() = default;

        virtual void Init(nlohmann::json inputConfig) = 0;
    };
}
