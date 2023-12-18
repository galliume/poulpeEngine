#pragma once
#include "IShaderManager.hpp"
#include "Poulpe/Renderer/Adapter/IRendererAdapter.hpp"

namespace Poulpe
{
    class ShaderManager : IShaderManager
    {
    public:
        explicit ShaderManager();
        virtual ~ShaderManager() = default;

        void addShader(std::string const & name, std::string const & vertPath, std::string const & fragPath) override;
        std::function<void()> load(nlohmann::json config) override;
        inline VulkanShaders* getShaders() const override { return m_Shaders.get(); };

        void addRenderer(IRendererAdapter* renderer) { m_Renderer = renderer; };
        void clear();
        bool isLoadingDone() { return m_LoadingDone.load(); }

    private:
        void createGraphicPipeline(std::string const & shaderName);
        VkDescriptorSetLayout createDescriptorSetLayout();
        VkDescriptorSetLayout createDescriptorSetLayoutForSkybox();
        VkDescriptorSetLayout createDescriptorSetLayoutForHUD();
        VkDescriptorSetLayout createShadowMapDescriptorSetLayout();

        std::vector<VkPipelineShaderStageCreateInfo> getShadersInfo(std::string const & shaderName);
        VkPipelineVertexInputStateCreateInfo getVertexBindingDesc(VkVertexInputBindingDescription bDesc,
            std::array<VkVertexInputAttributeDescription, 6> attDesc);

    private:

        std::unique_ptr<VulkanShaders> m_Shaders = nullptr;
        IRendererAdapter* m_Renderer = nullptr;
        nlohmann::json m_Config;
        std::atomic_bool m_LoadingDone = false;
    };
}
