#pragma once

#include "IShaderManager.hpp"

#include "Poulpe/Renderer/IRenderer.hpp"

namespace Poulpe
{
    enum class DescSetLayoutType {
        Skybox, HUD, Entity, Offscreen
    };
    enum class VertexBindingType {
        Vertex2D, Vertex3D
    };

    class ShaderManager : IShaderManager
    {
    public:
        explicit ShaderManager();
        ~ShaderManager() override = default;

        inline void addRenderer(IRenderer* const renderer) override { m_Renderer = renderer; }
        void addShader(std::string const & name, std::string const & vertPath, std::string const & fragPath) override;
        void clear();
        inline VulkanShaders* getShaders() const override { return m_Shaders.get(); }
        inline bool isLoadingDone() { return m_LoadingDone.load(); }
        std::function<void()> load(nlohmann::json config) override;

    private:

        template <DescSetLayoutType T>
        VkDescriptorSetLayout createDescriptorSetLayout();
        void createGraphicPipeline(std::string const & shaderName);

        std::vector<VkPipelineShaderStageCreateInfo> getShadersInfo(std::string const & shaderName, bool offscreen = false);

        template <VertexBindingType T>
        VkPipelineVertexInputStateCreateInfo* getVertexInputState();

    private:
        nlohmann::json m_Config;
        std::atomic_bool m_LoadingDone{ false };
        IRenderer* m_Renderer{ nullptr };
        std::unique_ptr<VulkanShaders> m_Shaders{ nullptr };
    };
}
