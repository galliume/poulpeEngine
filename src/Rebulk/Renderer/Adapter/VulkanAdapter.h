#pragma once
#include "IRendererAdapter.h"

namespace Rbk
{
    class VulkanAdapter : public IRendererAdapter, public std::enable_shared_from_this<VulkanAdapter>
    {

    public:

        explicit VulkanAdapter(std::shared_ptr<Window> window);
        ~VulkanAdapter();

        virtual void Init() override;
        virtual void AddCamera(std::shared_ptr<Camera> camera) override { m_Camera = camera; }
        virtual void Draw() override;
        virtual void Destroy() override;
        virtual void DrawSplashScreen() override;
        virtual void WaitIdle() override;
        virtual std::shared_ptr<VulkanRenderer> Rdr() override { return m_Renderer; }
        virtual ImGuiInfo GetImGuiInfo() override;
        virtual void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function, int queueIndex = 0) override;
        virtual void ShowGrid(bool show) override;
        virtual void AddEntities(std::vector<std::shared_ptr<Entity>>* entities) override;
        virtual void AddBbox(std::vector<std::shared_ptr<Entity>>* bbox) override { m_BoundingBox = bbox; }
        virtual void AddSkybox(std::shared_ptr<Mesh> skyboxMesh) override { m_SkyboxMesh = skyboxMesh; }
        virtual void AddHUD(std::vector<std::shared_ptr<Mesh>> hud) override { m_HUD = hud; }
        virtual void AddSplash(std::vector<std::shared_ptr<Mesh>> splash) override { m_Splash = splash; }
        virtual inline std::vector<VkDescriptorSetLayout>* GetDescriptorSetLayouts() override { return &m_DescriptorSetLayouts; }
        virtual inline std::vector<VkImage>* GetSwapChainImages() override { return &m_SwapChainImages; }
        virtual inline std::shared_ptr<VkRenderPass> RdrPass() override { return m_RenderPass; }
        virtual inline glm::mat4 GetPerspective() override { return m_Perspective; }
        virtual void SetDeltatime(float deltaTime) override;
        void Clear();
        void DrawEntities(std::vector<std::shared_ptr<Entity>>& entities);
        void DrawSkybox();
        void DrawHUD();
        void DrawBbox();

        void ShouldRecreateSwapChain();
        void RecreateSwapChain();
        inline uint32_t GetSwapImageIndex() { return m_ImageIndex; }
        void SetRayPick(float x, float y, float z, int width, int height);
        void FlushSplashScreen();

        //@todo add GuiManager
        VkRenderPass CreateImGuiRenderPass();
        
        //IMGUI config
        static std::atomic<float> s_AmbiantLight;
        static std::atomic<float> s_FogDensity;
        static std::atomic<float> s_FogColor[3];
        static std::atomic<int> s_Crosshair;
        static std::atomic<int> s_PolygoneMode;

        std::shared_ptr<Camera> GetCamera() { return m_Camera; }
        std::vector<glm::vec3> GetLights() { return m_LightsPos; }

    private:
        //@todo temp
        void SetPerspective();
        void BeginRendering(VkCommandBuffer commandBuffer, const VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD, const VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE);
        void EndRendering(VkCommandBuffer commandBuffer);
        void Submit(std::vector<VkCommandBuffer> commandBuffers, int queueIndex = 0);

    private:
        std::shared_ptr<VulkanRenderer> m_Renderer = nullptr;
        std::shared_ptr<VkRenderPass> m_RenderPass = nullptr;
        VkSwapchainKHR m_SwapChain = nullptr;
        std::vector<VkImage> m_SwapChainImages = {};
        std::vector<VkFramebuffer> m_SwapChainFramebuffers = {};
        std::vector<VkImageView> m_SwapChainImageViews = {};

        std::vector<std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>> m_Semaphores = {};

        VkCommandPool m_CommandPoolSplash = nullptr;
        std::vector<VkCommandBuffer> m_CommandBuffersSplash = {};

        VkCommandPool m_CommandPoolEntities = nullptr;
        std::vector<VkCommandBuffer> m_CommandBuffersEntities = {};

        VkCommandPool m_CommandPoolBbox = nullptr;
        std::vector<VkCommandBuffer> m_CommandBuffersBbox = {};

        VkCommandPool m_CommandPoolSkybox = nullptr;
        std::vector<VkCommandBuffer> m_CommandBuffersSkybox = {};

        VkCommandPool m_CommandPoolHud = nullptr;
        std::vector<VkCommandBuffer> m_CommandBuffersHud = {};

        uint32_t m_ImageIndex = 0;
        std::pair<std::vector<VkBuffer>, std::vector<VkDeviceMemory>> m_UniformBuffers = {};
        std::vector<VulkanPipeline>m_Pipelines;
        std::shared_ptr<Camera> m_Camera = nullptr;
        std::shared_ptr<Window> m_Window = nullptr;

        //@todo move to meshManager
        std::vector<std::shared_ptr<Mesh>> m_Splash = {};
        std::vector<std::shared_ptr<Mesh>> m_HUD = {};
        std::vector<VkImageView>m_DepthImageViews = {};
        std::vector<VkImage>m_DepthImages = {};
        glm::mat4 m_Perspective;
        glm::mat4 m_lastLookAt;
        float m_Deltatime = 0.0f;
        std::vector<glm::vec3>m_LightsPos;
        std::vector<VkDescriptorPool>m_DescriptorPools;
        std::vector<VkDescriptorSetLayout>m_DescriptorSetLayouts;

        glm::vec3 m_RayPick;
        bool m_HasClicked = false;

        std::vector<std::vector<std::shared_ptr<Entity>>> m_Entities;
        std::shared_ptr<Mesh> m_SkyboxMesh = nullptr;
        std::vector<std::shared_ptr<Entity>>* m_BoundingBox;
        std::vector<VkCommandBuffer> m_CmdToSubmit;;
    };
}
