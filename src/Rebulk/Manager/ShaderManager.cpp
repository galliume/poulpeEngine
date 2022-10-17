#include "rebulkpch.h"
#include "ShaderManager.h"
#include "Rebulk/Core/Tools.h"

namespace Rbk
{
    ShaderManager::ShaderManager(std::shared_ptr<VulkanRenderer> renderer) : m_Renderer(renderer)
    {
        m_Shaders = std::make_shared<VulkanShaders>();
    }

    std::future<void> ShaderManager::Load(nlohmann::json config)
    {
        std::future shaderFuture = std::async(std::launch::async, [this, config]() {
            for (auto& shader : config["shader"].items()) {

                auto key = static_cast<std::string>(shader.key());
                auto data = shader.value();

                AddShader(key, data["vert"], data["frag"]);
            }
        });

        return std::move(shaderFuture);
    }

    void ShaderManager::AddShader(const std::string& name, const std::string& vertPath, const std::string& fragPath)
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
