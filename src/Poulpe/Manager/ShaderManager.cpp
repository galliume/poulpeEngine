#include "ShaderManager.hpp"
#include "Poulpe/Core/Tools.hpp"

namespace Poulpe
{
    ShaderManager::ShaderManager()
    {
        m_Shaders = std::make_shared<VulkanShaders>();
    }

    std::function<void()> ShaderManager::Load(nlohmann::json config, std::condition_variable& cv)
    {
        m_Config = config;

        std::function shaderFuture = [=, this, &cv]() {
            for (auto& shader : m_Config["shader"].items()) {

                auto key = static_cast<std::string>(shader.key());
                auto data = shader.value();

                AddShader(key, data["vert"], data["frag"]);
            }

            m_LoadingDone = true;
            cv.notify_one();
        };

        return shaderFuture;
    }

    void ShaderManager::AddShader(const std::string& name, const std::string& vertPath, const std::string& fragPath)
    {

        if (!std::filesystem::exists(vertPath)) {
            PLP_FATAL("vertex shader file {} does not exits.", vertPath);
            return;
        }

        if (!std::filesystem::exists(fragPath)) {
            PLP_FATAL("fragment shader file {} does not exits.", fragPath);
            return;
        }

        auto vertShaderCode = Poulpe::Tools::ReadFile(vertPath);
        auto fragShaderCode = Poulpe::Tools::ReadFile(fragPath);
    
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