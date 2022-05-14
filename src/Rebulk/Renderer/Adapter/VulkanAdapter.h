#pragma once
#include "IRendererAdapter.h"
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"
#include "Rebulk/Renderer/Mesh.h"
#include "Rebulk/GUI/Window.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

namespace Rbk
{
    struct VImGuiInfo
    {
        ImGui_ImplVulkan_InitInfo info = {};
        VkCommandBuffer cmdBuffer = nullptr;
        VkPipeline pipeline = nullptr;
        VkRenderPass rdrPass = nullptr;
    };

    class VulkanAdapter : public IRendererAdapter
    {

    public:

        VulkanAdapter(Window* window);
        ~VulkanAdapter();

        virtual void Init() override;
        virtual void AddCamera(Camera* camera) override;
        virtual void AddTextureManager(TextureManager* textureManager) override;
        virtual void AddMeshManager(MeshManager* meshManager) override;
        virtual void AddShaderManager(ShaderManager* shaderManager) override;

        virtual void PrepareWorld() override;
        virtual void PrepareDraw() override;
        virtual void Draw() override;
        virtual void Destroy() override;

        void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
        void SetWireFrameMode(bool wireFrameModeOn) { m_WireFrameModeOn = wireFrameModeOn; };
        void SouldResizeSwapChain();

        inline uint32_t GetSwapImageIndex() { return m_ImageIndex; };
        inline VulkanRenderer* Rdr() { return m_Renderer; };
        inline std::shared_ptr<VkRenderPass> RdrPass() { return m_RenderPass; };
        inline void MakeSpin(bool spin) { m_MakeSpin = spin; };

        //@todo add GuiManager
        VkRenderPass CreateImGuiRenderPass();
        VImGuiInfo GetVImGuiInfo();

    private:
        void UpdateWorldPositions();

    private:
        VulkanRenderer* m_Renderer = nullptr;
        std::shared_ptr<VkRenderPass> m_RenderPass = nullptr;
        VkSwapchainKHR m_SwapChain = nullptr;
        std::vector<VkImage> m_SwapChainImages = {};
        std::vector<VkFramebuffer> m_SwapChainFramebuffers = {};
        std::vector<VkImageView> m_SwapChainImageViews = {};
        std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> m_Semaphores = {};
        VkCommandPool m_CommandPool = nullptr;
        std::vector<VkCommandBuffer> m_CommandBuffers = {};
        uint32_t m_ImageIndex = 0;
        std::pair<std::vector<VkBuffer>, std::vector<VkDeviceMemory>> m_UniformBuffers = {};
        VkDescriptorSet m_DescriptorSet = nullptr;
        VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
        VkPipelineLayout m_PipelineLayout = nullptr;
        VkDescriptorPool m_DescriptorPool = nullptr;
        std::vector<VulkanPipeline>m_Pipelines;
        bool m_IsPrepared = false;
        bool m_WireFrameModeOn = false;
        bool m_MakeSpin = false;
        Camera* m_Camera = nullptr;
        Window* m_Window = nullptr;
        TextureManager* m_TextureManager = nullptr;
        MeshManager* m_MeshManager = nullptr;
        ShaderManager* m_ShaderManager = nullptr;
        //@todo move to meshManager
        std::shared_ptr<Mesh> m_Crosshair = nullptr;
    };
}
