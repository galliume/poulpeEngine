#pragma once
#include <nlohmann/json.hpp>

#include "Rebulk/Renderer/Adapter/VulkanAdapter.hpp"
#include "Rebulk/GUI/Window.hpp"

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
