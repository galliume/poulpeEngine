#pragma once
#include "IRendererAdapter.h"
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"
#include "Rebulk/Component/Mesh.h"
#include "Rebulk/Component/Mesh2D.h"
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

        explicit VulkanAdapter(std::shared_ptr<Window> window);
        virtual ~VulkanAdapter();

        virtual void Init() override;
        virtual void AddCamera(std::shared_ptr<Camera> camera) override;
        virtual void AddTextureManager(std::shared_ptr<TextureManager> textureManager) override;
        virtual void AddEntityManager(std::shared_ptr<EntityManager> entityManager) override;
        virtual void AddShaderManager(std::shared_ptr<ShaderManager> shaderManager) override;

        virtual void Prepare() override;
        virtual void Draw() override;
        virtual void Destroy() override;

        void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
        void SetWireFrameMode(bool wireFrameModeOn) { m_WireFrameModeOn = wireFrameModeOn; };
        void ShouldRecreateSwapChain();
        void RecreateSwapChain();

        inline uint32_t GetSwapImageIndex() { return m_ImageIndex; };
        inline std::shared_ptr<VulkanRenderer> Rdr() { return m_Renderer; };
        inline std::shared_ptr<VkRenderPass> RdrPass() { return m_RenderPass; };
        void SetDeltatime(float deltaTime) override;

        //@todo add GuiManager
        VkRenderPass CreateImGuiRenderPass();
        VImGuiInfo GetVImGuiInfo();

        //IMGUI config
        static float s_AmbiantLight;
        static float s_FogDensity;
        static float s_FogColor[3];

    private:
        //@todo temp
        void SetPerspective();

    private:
        std::shared_ptr<VulkanRenderer> m_Renderer = nullptr;
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
        std::vector<VulkanPipeline>m_Pipelines;
        bool m_WireFrameModeOn = false;
        std::shared_ptr<Camera> m_Camera = nullptr;
        std::shared_ptr<Window> m_Window = nullptr;
        std::shared_ptr<TextureManager> m_TextureManager = nullptr;
        std::shared_ptr<EntityManager> m_EntityManager = nullptr;
        std::shared_ptr<ShaderManager> m_ShaderManager = nullptr;
        //@todo move to meshManager
        std::shared_ptr<Mesh> m_Crosshair = nullptr;
        std::vector<VkImageView>m_DepthImageViews = {};
        std::vector<VkImageView>m_ColorImageViews = {};
        glm::mat4 m_Perspective;
        glm::mat4 m_lastLookAt;
        float m_Deltatime = 0.0f;
        std::vector<glm::vec3>m_LightsPos;
        std::vector<VkDescriptorPool>m_DescriptorPools;
        std::vector<VkDescriptorSetLayout>m_DescriptorSetLayouts;

        VkCommandPool m_EntitiesCommandPool = nullptr;
        std::vector<VkCommandBuffer> m_EntitiesCommandBuffers = {};
        VkCommandPool m_SkyboxCommandPool = nullptr;
        std::vector<VkCommandBuffer> m_SkyboxCommandBuffers = {};
        VkCommandPool m_CrosshairCommandPool = nullptr;
        std::vector<VkCommandBuffer> m_CrosshairCommandBuffers = {};

    };
}
