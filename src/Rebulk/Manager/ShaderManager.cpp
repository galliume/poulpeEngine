#include "ShaderManager.hpp"
#include "Rebulk/Core/Tools.hpp"

namespace Rbk
{
    ShaderManager::ShaderManager()
    {
        m_Shaders = std::make_shared<VulkanShaders>();
    }

    std::function<void()> ShaderManager::Load(nlohmann::json config)
    {
        m_Config = config;

        std::function shaderFuture = [=]() {
            for (auto& shader : m_Config["shader"].items()) {

                auto key = static_cast<std::string>(shader.key());
                auto data = shader.value();

                AddShader(key, data["vert"], data["frag"]);
            }

            m_LoadingDone = true;
        };

        return shaderFuture;
    }

    void ShaderManager::AddShader(const std::string& name, const std::string& vertPath, const std::string& fragPath)
    {

        if (!std::filesystem::exists(vertPath)) {
            RBK_FATAL("vertex shader file {} does not exits.", vertPath);
            return;
        }

        if (!std::filesystem::exists(fragPath)) {
            RBK_FATAL("fragment shader file {} does not exits.", fragPath);
            return;
        }

        auto vertShaderCode = Rbk::Tools::ReadFile(vertPath);
        auto fragShaderCode = Rbk::Tools::ReadFile(fragPath);
    
        VkShaderModule vertexShaderModule = m_Renderer->Rdr()->CreateShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = m_Renderer->Rdr()->CreateShaderModule(fragShaderCode);

        std::array<VkShaderModule, 2> module = { vertexShaderModule, fragShaderModule };

        m_Shaders->shaders[name] = module;
    }

    void ShaderManager::Clear()
    {
        m_Shaders->shaders.clear();
        m_LoadingDone = false;
    }

    ShaderManager::~ShaderManager()
    {

    }
}
