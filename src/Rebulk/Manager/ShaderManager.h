#pragma once
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"

namespace Rbk
{
    class ShaderManager
    {
    public:
        ShaderManager(VulkanRenderer* renderer);
        void AddShader(const char* name, const char* vertPath, const char* fragPath);
        inline VulkanShaders GetShaders() { return m_Shaders; };

    private:
        VulkanShaders m_Shaders;
        VulkanRenderer* m_Renderer = nullptr;
    };
}