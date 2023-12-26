#pragma once

#include "IShaderManager.hpp"

#include "Poulpe/Renderer/IRenderer.hpp"

namespace Poulpe
{
    class ShaderManager : IShaderManager
    {
    public:
        explicit ShaderManager();
        virtual ~ShaderManager() = default;

        inline void addRenderer(IRenderer* const renderer) override { m_Renderer = renderer; };
        void addShader(std::string const & name, std::string const & vertPath, std::string const & fragPath) override;
        void clear();
        inline VulkanShaders* getShaders() const override { return m_Shaders.get(); };
        inline bool isLoadingDone() { return m_LoadingDone.load(); }
        std::function<void()> load(nlohmann::json config) override;

    private:
        VkDescriptorSetLayout createDescriptorSetLayout();
        VkDescriptorSetLayout createDescriptorSetLayoutForHUD();
        VkDescriptorSetLayout createDescriptorSetLayoutForSkybox();
        void createGraphicPipeline(std::string const & shaderName);

        std::vector<VkPipelineShaderStageCreateInfo> getShadersInfo(std::string const & shaderName);

        VkPipelineVertexInputStateCreateInfo getVertexBindingDesc(
            VkVertexInputBindingDescription bDesc,
            std::array<VkVertexInputAttributeDescription, 6> attDesc);

        VkPipelineVertexInputStateCreateInfo getVertexBindingDesc2D(
            VkVertexInputBindingDescription bDesc,
            std::array<VkVertexInputAttributeDescription, 3> attDesc);

    private:
        nlohmann::json m_Config;
        std::atomic_bool m_LoadingDone{ false };
        IRenderer* m_Renderer{ nullptr };
        std::unique_ptr<VulkanShaders> m_Shaders{ nullptr };
    };
}
