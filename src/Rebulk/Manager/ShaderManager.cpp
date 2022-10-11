#include "rebulkpch.h"
#include "ShaderManager.h"
#include "Rebulk/Core/Tools.h"

namespace Rbk
{
    ShaderManager::ShaderManager(std::shared_ptr<VulkanRenderer> renderer) : m_Renderer(renderer)
    {
        m_Shaders = std::make_shared<VulkanShaders>();
    }

    void ShaderManager::Load()
    {
        std::future shaderFuture = std::async(std::launch::async, [this]() {
            AddShader("main", "assets/shaders/spv/vert.spv", "assets/shaders/spv/frag.spv");
            AddShader("grid", "assets/shaders/spv/grid_vert.spv", "assets/shaders/spv/grid_frag.spv");
            AddShader("skybox", "assets/shaders/spv/skybox_vert.spv", "assets/shaders/spv/skybox_frag.spv");
            AddShader("ambient_light", "assets/shaders/spv/ambient_shader_vert.spv", "assets/shaders/spv/ambient_shader_frag.spv");
        });
    }

    void ShaderManager::AddShader(const std::string& name, const char* vertPath, const char* fragPath)
    {

        if (!std::filesystem::exists(vertPath)) {
            Rbk::Log::GetLogger()->critical("vertex shader file {} does not exits.", vertPath);
            return;
        }

        if (!std::filesystem::exists(fragPath)) {
            Rbk::Log::GetLogger()->critical("fragment shader file {} does not exits.", fragPath);
            return;
        }

        auto vertShaderCode = Rbk::Tools::ReadFile(vertPath);
        auto fragShaderCode = Rbk::Tools::ReadFile(fragPath);
    
        VkShaderModule vertexShaderModule = m_Renderer.get()->CreateShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = m_Renderer.get()->CreateShaderModule(fragShaderCode);

        std::array<VkShaderModule, 2> module = { vertexShaderModule, fragShaderModule };

        m_Shaders->shaders[name] = module;
    }
}
