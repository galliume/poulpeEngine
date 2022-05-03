#include "rebulkpch.h"
#include "ShaderManager.h"

namespace Rbk
{
    ShaderManager::ShaderManager(VulkanRenderer* renderer) : m_Renderer(renderer)
    {
        m_Shaders = std::make_shared<VulkanShaders>();
    }
    void ShaderManager::AddShader(std::string name, const char* vertPath, const char* fragPath)
    {

        if (!std::filesystem::exists(vertPath)) {
            Rbk::Log::GetLogger()->critical("vertex shader file {} does not exits.", vertPath);
            return;
        }

        if (!std::filesystem::exists(fragPath)) {
            Rbk::Log::GetLogger()->critical("fragment shader file {} does not exits.", fragPath);
            return;
        }

        auto vertShaderCode = ReadFile(vertPath);
        auto fragShaderCode = ReadFile(fragPath);
    
        VkShaderModule vertexShaderModule = m_Renderer->CreateShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = m_Renderer->CreateShaderModule(fragShaderCode);

        std::array<VkShaderModule, 2> module = { vertexShaderModule, fragShaderModule };

        m_Shaders->shaders[name] = module;
    }
}
