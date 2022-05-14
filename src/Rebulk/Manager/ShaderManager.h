#pragma once
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"

namespace Rbk
{
    struct VulkanShaders
    {
        std::map<std::string, std::array<VkShaderModule, 2>> shaders;
    };

    class ShaderManager
    {
    public:
        ShaderManager(std::shared_ptr<VulkanRenderer> renderer);
        void AddShader(std::string name, const char* vertPath, const char* fragPath);
        inline std::shared_ptr<VulkanShaders> GetShaders() { return m_Shaders; };

    private:
        std::shared_ptr<VulkanShaders> m_Shaders = nullptr;
        std::shared_ptr<VulkanRenderer> m_Renderer = nullptr;
    };
}
