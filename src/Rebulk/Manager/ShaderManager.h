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
        
explicit ShaderManager(std::shared_ptr<VulkanRenderer> renderer);
        void AddShader(const std::string& name, const char* vertPath, const char* fragPath);
        void Load();

        inline const std::shared_ptr<VulkanShaders> GetShaders() const { return m_Shaders; };

    private:
        std::shared_ptr<VulkanShaders> m_Shaders = nullptr;
        std::shared_ptr<VulkanRenderer> m_Renderer = nullptr;
    };
}
