#pragma once

#include "IRendererAdapter.hpp"

#include "Poulpe/Core/IObserver.hpp"

#include "Poulpe/GUI/ImGui/Im.hpp"

#include "Poulpe/Manager/ComponentManager.hpp"
#include "Poulpe/Manager/EntityManager.hpp"
#include "Poulpe/Manager/LightManager.hpp"

#include <future>

namespace Poulpe
{
    class VulkanAdapter : public IRendererAdapter, public std::enable_shared_from_this<VulkanAdapter>
    {

    public:

        explicit VulkanAdapter(
          Window* window,
          EntityManager* entityManager, 
          ComponentManager* componentManager,
          LightManager* lightManager
        );

        virtual ~VulkanAdapter() = default;

        void init() override;
        void addCamera(Camera* camera) override { m_Camera = camera; }
        void draw() override;
        void destroy() override;
        VulkanRenderer* rdr() override { return m_Renderer.get(); }
        void immediateSubmit(std::function<void(VkCommandBuffer cmd)> && function, int queueIndex = 0) override;
        void showGrid(bool show) override;
        inline std::vector<VkDescriptorSetLayout>* getDescriptorSetLayouts() override { return & m_DescriptorSetLayouts; }
        inline std::vector<VkImage>* getSwapChainImages() override { return & m_SwapChainImages; }
        inline VkRenderPass* rdrPass() override { return m_RenderPass.get(); }
        inline glm::mat4 getPerspective() override { return m_Perspective; }
        void setDeltatime(float deltaTime) override;
        void renderScene() override;
        ImGuiInfo getImGuiInfo() ;
        void clear();
        void drawEntities();
        void drawSkybox();
        void drawHUD();
        void drawBbox();
        //void RenderForImGui(VkCommandBuffer cmdBuffer, VkFramebuffer swapChainFramebuffer);
        void addCmdToSubmit(VkCommandBuffer cmd);

        void beginRendering(VkCommandBuffer commandBuffer, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE);

        void endRendering(VkCommandBuffer commandBuffer);
        VkSwapchainKHR getSwapChain() { return m_SwapChain; };

        void shouldRecreateSwapChain();
        void recreateSwapChain();
        void setRayPick(float x, float y, float z, int width, int height);
        void setDrawBbox(bool draw) { m_DrawBbox = draw; };
        bool getDrawBbox() { return m_DrawBbox; };
        void clearRendererScreen();
        void stopRendering() { m_RenderingStopped = true; };
        
        std::pair<VkSampler, VkImageView> getImguiTexture() {
            return std::make_pair(m_SwapChainSamplers[m_ImageIndex], m_SwapChainImageViews[m_ImageIndex]);
        };

        std::pair<VkSampler, VkImageView> getImguiDepthImage() {
            return std::make_pair(m_SwapChainDepthSamplers[m_ImageIndex], m_DepthImageViews[m_ImageIndex]);
        };

        std::vector<VkImageView>* getSwapChainImageViews() { return &m_SwapChainImageViews; }
        uint32_t getCurrentFrameIndex() const { return m_ImageIndex; };

        //@todo add GuiManager
        VkRenderPass createImGuiRenderPass(VkFormat format);
        
        void attachObserver(IObserver* observer);

        //IMGUI config
        static std::atomic<float> s_AmbiantLight;
        static std::atomic<float> s_FogDensity;
        static std::atomic<float> s_FogColor[3];
        static std::atomic<int> s_Crosshair;
        static std::atomic<int> s_PolygoneMode;

        Camera* getCamera() { return m_Camera; }

        void drawShadowMap();

    private:
        //@todo temp
        void setPerspective();
        void submit(std::vector<VkCommandBuffer> commandBuffers, int queueIndex = 0);
        void present(int queueIndex = 0);
        void onFinishRender();
        void acquireNextImage();
        void prepareShadowMap();

    private:
        std::unique_ptr<VulkanRenderer> m_Renderer{ nullptr };
        std::unique_ptr<VkRenderPass> m_RenderPass{ nullptr };
        VkSwapchainKHR m_SwapChain{ nullptr };
        std::vector<VkImage> m_SwapChainImages{};
        std::vector<VkFramebuffer> m_SwapChainFramebuffers{};
        std::vector<VkImageView> m_SwapChainImageViews{};

        //@todo wtf
        std::vector<std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>> m_Semaphores{};

        VkCommandPool m_CommandPoolEntities{ nullptr };
        std::vector<VkCommandBuffer> m_CommandBuffersEntities{};

        VkCommandPool m_CommandPoolBbox{ nullptr };
        std::vector<VkCommandBuffer> m_CommandBuffersBbox{};

        VkCommandPool m_CommandPoolSkybox{ nullptr };
        std::vector<VkCommandBuffer> m_CommandBuffersSkybox{};

        VkCommandPool m_CommandPoolHud{ nullptr };
        std::vector<VkCommandBuffer> m_CommandBuffersHud{};

        uint32_t m_ImageIndex{ 0 };
        std::pair<std::vector<VkBuffer>, std::vector<VkDeviceMemory>> m_UniformBuffers{};
        std::vector<VulkanPipeline>m_Pipelines;
        
        Camera* m_Camera{ nullptr };
        Window* m_Window{ nullptr };
        EntityManager* m_EntityManager{ nullptr };
        [[maybe_unused]] ComponentManager* m_ComponentManager{ nullptr };
        [[maybe_unused]] LightManager* m_LightManager{ nullptr };

        //@todo move to meshManager
        std::vector<VkImageView>m_DepthImageViews{};
        std::vector<VkImage>m_DepthImages{};
        glm::mat4 m_Perspective;
        //glm::mat4 m_lastLookAt;
        float m_Deltatime{ 0.0f };
        std::vector<glm::vec3>m_LightsPos;
        std::vector<VkDescriptorPool>m_DescriptorPools;
        std::vector<VkDescriptorSetLayout>m_DescriptorSetLayouts;

        glm::vec3 m_RayPick;
        bool m_HasClicked{ false };

        //std::vector<std::shared_ptr<Entity>>* m_BoundingBox;
        std::vector<VkCommandBuffer> m_CmdToSubmit;
        std::vector<VkCommandBuffer> m_moreCmdToSubmit;

        bool m_DrawBbox{ false };
        std::vector<std::future<void>> m_CmdLists{};
        std::vector<std::future<void>> m_BufferedCmdLists{};
        //uint32_t m_BufferedIndex = 0;

        //thread signaling
        std::mutex m_MutexRenderScene;
        std::mutex m_MutexSubmit;
        std::mutex m_MutexCmdSubmit;
        std::mutex m_MutexCmdSubmitEntities;
        std::mutex m_MutexCmdSubmitSkbybox;
        std::mutex m_MutexCmdSubmitHUD;
        std::mutex m_MutexCmdSubmitBBox;
        std::condition_variable m_RenderCond;
        unsigned int m_renderStatus{ 1 };

        bool m_RenderingStopped{ false };

        std::vector<VkSampler> m_SwapChainSamplers{};
        std::vector<VkSampler> m_SwapChainDepthSamplers{};

        std::vector<IObserver*> m_Observers{};
        //VkCommandBuffer m_CopyCmd;
        //VkCommandPool m_CopyCommandPool;

        [[maybe_unused]] VkFramebuffer m_DepthMapFrameBuffer;
        [[maybe_unused]] VkImage m_DepthMapImage;
        [[maybe_unused]] VkDeviceMemory m_DepthMapDeviceMemory;
        [[maybe_unused]] VkImageView m_DepthMapView;
        [[maybe_unused]] VkRenderPass m_DepthMapRenderPass;
        [[maybe_unused]] VkSampler m_DepthMapSampler;
        [[maybe_unused]] VkDescriptorImageInfo m_DepthMapDescriptor;
    };
}
