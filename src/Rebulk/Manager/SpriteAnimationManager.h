#pragma once
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"

namespace Rbk
{
    class SpriteAnimationManager
    {
    public:
        explicit SpriteAnimationManager();
        void Add(const std::string& name, std::vector<std::string> sprites);
        std::vector<std::string> GetSpritesByName(const std::string& name);

    private:
        std::map<std::string, std::vector<std::string>> m_SpriteAnimations;
    };
}