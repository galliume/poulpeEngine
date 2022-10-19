#pragma once
#include <json.hpp>

#include "Rebulk/Component/Camera.h"
#include "Rebulk/GUI/Window.h"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.h"

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
