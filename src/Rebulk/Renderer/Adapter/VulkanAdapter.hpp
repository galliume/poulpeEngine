#pragma once
#include "IRendererAdapter.hpp"

namespace Rbk
{
    class VulkanAdapter : public IRendererAdapter, public std::enable_shared_from_this<VulkanAdapter>
    {

    public:

        explicit VulkanAdapter(std::shared_ptr<Window> window);
        ~VulkanAdapter() = default;

        virtual void Init() override;
        virtual void AddCamera(std::shared_ptr<Camera> camera) override { m_Camera = camera; }
        virtual void Draw() override;
        virtual void Destroy() override;
        virtual void DrawSplashScreen() override;
        virtual std::shared_ptr<VulkanRenderer> Rdr() override { return m_Renderer; }
        virtual void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function, int queueIndex = 0) override;
        virtual void ShowGrid(bool show) override;
        virtual void AddEntities(std::vector<std::shared_ptr<Entity>>* entities) override;
        virtual void AddSkybox(std::shared_ptr<Mesh> skyboxMesh) override { m_SkyboxMesh = skyboxMesh; }
        virtual void AddHUD(std::vector<std::shared_ptr<Mesh>> hud) override { m_HUD = hud; }
        virtual void AddSplash(std::vector<std::shared_ptr<Mesh>> splash) override { m_Splash = splash; }
        virtual inline std::vector<VkDescriptorSetLayout>* GetDescriptorSetLayouts() override { return &m_DescriptorSetLayouts; }
        virtual inline std::vector<VkImage>* GetSwapChainImages() override { return &m_SwapChainImages; }
        virtual inline std::shared_ptr<VkRenderPass> RdrPass() override { return m_RenderPass; }
        virtual inline glm::mat4 GetPerspective() override { return m_Perspective; }
        virtual void SetDeltatime(float deltaTime) override;
        virtual void RenderScene() override;
        ImGuiInfo GetImGuiInfo() ;
        void Clear();
        void DrawEntities();
        void DrawSkybox();
        void DrawHUD();
        void DrawBbox();
        //void RenderForImGui(VkCommandBuffer cmdBuffer, VkFramebuffer swapChainFramebuffer);
        void AddCmdToSubmit(VkCommandBuffer cmd);
        void BeginRendering(VkCommandBuffer commandBuffer, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD, VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE);
        void EndRendering(VkCommandBuffer commandBuffer);
        VkSwapchainKHR GetSwapChain() { return m_SwapChain; };

        void ShouldRecreateSwapChain();
        void RecreateSwapChain();
        void SetRayPick(float x, float y, float z, int width, int height);
        void ClearSplashScreen();
        void SetDrawBbox(bool draw) { m_DrawBbox = draw; };
        bool GetDrawBbox() { return m_DrawBbox; };
        void ClearRendererScreen();
        void StopRendering() { m_RenderingStopped = true; };

        std::vector<VkImageView>* GetSwapChainImageViews() { return &m_SwapChainImageViews; }
        uint32_t GetCurrentFrameIndex() const { return m_ImageIndex; };

        //@todo add GuiManager
        VkRenderPass CreateImGuiRenderPass(VkFormat format);
        
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
        void Submit(std::vector<VkCommandBuffer> commandBuffers, int queueIndex = 0);
        void Present(int queueIndex = 0);

    private:
        std::shared_ptr<VulkanRenderer> m_Renderer = nullptr;
        std::shared_ptr<VkRenderPass> m_RenderPass = nullptr;
        VkSwapchainKHR m_SwapChain = nullptr;
        std::vector<VkImage> m_SwapChainImages = {};
        std::vector<VkFramebuffer> m_SwapChainFramebuffers = {};
        std::vector<VkImageView> m_SwapChainImageViews = {};

        //@todo wtf
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

        std::vector<std::shared_ptr<Entity>>* m_Entities;
        std::shared_ptr<Mesh> m_SkyboxMesh = nullptr;
        std::vector<std::shared_ptr<Entity>>* m_BoundingBox;
        std::vector<VkCommandBuffer> m_CmdToSubmit;
        std::vector<VkCommandBuffer> m_moreCmdToSubmit;

        bool m_DrawBbox = false;
        std::vector<std::future<void>> m_CmdLists{};
        std::vector<std::future<void>> m_BufferedCmdLists{};
        uint32_t m_BufferedIndex = 0;

        //thread signaling
        std::mutex m_MutexRenderScene;
        std::mutex m_MutexSubmit;
        std::mutex m_MutexCmdSubmit;
        std::mutex m_MutexCmdSubmitEntities;
        std::mutex m_MutexCmdSubmitSkbybox;
        std::mutex m_MutexCmdSubmitHUD;
        std::mutex m_MutexCmdSubmitBBox;
        std::condition_variable m_RenderCond;
        unsigned int m_renderStatus = 1;

        bool m_RenderingStopped = false;
    };
}
