#pragma once
#include <nlohmann/json.hpp>

#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"
#include "Poulpe/GUI/Window.hpp"

namespace Poulpe
{
    class IInputManager
    {
    public:
        IInputManager() = default;
        ~IInputManager() = default;

        virtual void Init(nlohmann::json inputConfig) = 0;
    };
}
