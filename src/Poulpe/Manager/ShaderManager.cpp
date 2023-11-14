#include "ShaderManager.hpp"

#include "Poulpe/Core/Tools.hpp"

namespace Poulpe
{
    ShaderManager::ShaderManager()
    {
        m_Shaders = std::make_unique<VulkanShaders>();
    }

    std::function<void()> ShaderManager::load(nlohmann::json config, std::condition_variable & cv)
    {
        m_Config = config;

        std::function shaderFuture = [this, & cv]() {
            for (auto & shader : m_Config["shader"].items()) {

                auto key = static_cast<std::string>(shader.key());
                auto data = shader.value();

                addShader(key, data["vert"], data["frag"]);
            }

            m_LoadingDone = true;
            cv.notify_one();
        };

        return shaderFuture;
    }

    void ShaderManager::addShader(std::string const & name, std::string const & vertPath, std::string const & fragPath)
    {

        if (!std::filesystem::exists(vertPath)) {
            PLP_FATAL("vertex shader file {} does not exits.", vertPath);
            return;
        }

        if (!std::filesystem::exists(fragPath)) {
            PLP_FATAL("fragment shader file {} does not exits.", fragPath);
            return;
        }

        auto vertShaderCode = Poulpe::Tools::readFile(vertPath);
        auto fragShaderCode = Poulpe::Tools::readFile(fragPath);
    
        VkShaderModule vertexShaderModule = m_Renderer->rdr()->createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = m_Renderer->rdr()->createShaderModule(fragShaderCode);

        std::array<VkShaderModule, 2> module = { vertexShaderModule, fragShaderModule };

        m_Shaders->shaders[name] = module;
    }

    void ShaderManager::clear()
    {
        m_Shaders->shaders.clear();
        m_LoadingDone = false;
    }
}
