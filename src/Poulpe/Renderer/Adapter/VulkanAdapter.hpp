#pragma once

#include "IRendererAdapter.hpp"

#include "Poulpe/Core/IObserver.hpp"

#include "Poulpe/Manager/ComponentManager.hpp"
#include "Poulpe/Manager/EntityManager.hpp"
#include "Poulpe/Manager/LightManager.hpp"
#include "Poulpe/Manager/TextureManager.hpp"

#include <future>

namespace Poulpe
{
    class VulkanAdapter : public IRendererAdapter, public std::enable_shared_from_this<VulkanAdapter>
    {

    public:

      struct DrawCommand {
        VkCommandBuffer* buffer;
        VkPipelineStageFlags stageFlags;
        VkSemaphore* semaphore;
        std::atomic_bool done{false};
      };

        VulkanAdapter(
          Window* window,
          EntityManager* entityManager,
          LightManager* lightManager
        );
        ~VulkanAdapter() = default;

        void init() override;
        void addCamera(Camera* camera) override { m_Camera = camera; }
        void draw() override;
        void destroy() override;
        VulkanRenderer* rdr() override { return m_Renderer.get(); }
        void immediateSubmit(std::function<void(VkCommandBuffer cmd)> && function, int queueIndex = 0) override;
        void showGrid(bool show) override;
        inline std::vector<VkDescriptorSetLayout>* getDescriptorSetLayouts() override { return & m_DescriptorSetLayouts; }
        inline std::vector<VkImage>* getSwapChainImages() override { return & m_SwapChainImages; }
        inline glm::mat4 getPerspective() override { return m_Perspective; }
        void setDeltatime(float deltaTime) override;
        void renderScene() override;

        void clear();
        void drawEntities();
        void drawSkybox();
        void drawHUD();
        void drawBbox();
        //void RenderForImGui(VkCommandBuffer cmdBuffer, VkFramebuffer swapChainFramebuffer);

        void beginRendering(VkCommandBuffer commandBuffer, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE, bool continuousCmdBuffer = false);

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
            return std::make_pair(m_SwapChainSamplers[m_CurrentFrame], m_SwapChainImageViews[m_CurrentFrame]);
        };

        std::pair<VkSampler, VkImageView> getImguiDepthImage() {
            return std::make_pair(m_SwapChainDepthSamplers[m_CurrentFrame], m_SwapChainDepthImageViews[m_CurrentFrame]);
        };

        std::vector<VkImageView>* getSwapChainImageViews() { return &m_SwapChainImageViews; }
        uint32_t getCurrentFrameIndex() const { return m_CurrentFrame; };

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

        void drawShadowMap(std::vector<std::unique_ptr<Entity>>* entities, Light light);

        void addPipeline(std::string const & shaderName, VulkanPipeline pipeline) override;
        VulkanPipeline* getPipeline(std::string const& shaderName) { return & m_Pipelines[shaderName]; };

        std::vector<VkImageView>* getDepthMapImageViews() { return & m_DepthMapImageViews; };
        std::vector<VkSampler>* getDepthMapSamplers() { return & m_DepthMapSamplers; };

    private:
        const size_t m_MAX_FRAMES_IN_FLIGHT{ 2 };

        //@todo temp
        void setPerspective();
        void submit();
        void onFinishRender();

    private:
        std::unique_ptr<VulkanRenderer> m_Renderer{ nullptr };
        VkSwapchainKHR m_SwapChain{ nullptr };
        std::vector<VkImage> m_SwapChainImages{};
        std::vector<VkImageView> m_SwapChainImageViews{};

        VkCommandPool m_CommandPoolEntities{ nullptr };
        std::vector<VkCommandBuffer> m_CommandBuffersEntities{};

        VkCommandPool m_CommandPoolBbox{ nullptr };
        std::vector<VkCommandBuffer> m_CommandBuffersBbox{};

        VkCommandPool m_CommandPoolSkybox{ nullptr };
        std::vector<VkCommandBuffer> m_CommandBuffersSkybox{};

        VkCommandPool m_CommandPoolHud{ nullptr };
        std::vector<VkCommandBuffer> m_CommandBuffersHUD{};

        VkCommandPool m_CommandPoolShadowMap{ nullptr };
        std::vector<VkCommandBuffer> m_CommandBuffersShadowMap{};

        uint32_t m_CurrentFrame{ 0 };
        uint32_t m_ImageIndex;
        std::pair<std::vector<VkBuffer>, std::vector<VkDeviceMemory>> m_UniformBuffers{};
        
        Camera* m_Camera{ nullptr };
        Window* m_Window{ nullptr };
        EntityManager* m_EntityManager{ nullptr };
        LightManager* m_LightManager{ nullptr };

        //@todo move to meshManager
        std::vector<VkImageView>m_SwapChainDepthImageViews{};
        std::vector<VkImage>m_SwapChainDepthImages{};
        glm::mat4 m_Perspective;
        //glm::mat4 m_lastLookAt;
        float m_Deltatime{ 0.0f };
        std::vector<glm::vec3>m_LightsPos;
        std::vector<VkDescriptorPool>m_DescriptorPools;
        std::vector<VkDescriptorSetLayout>m_DescriptorSetLayouts;

        glm::vec3 m_RayPick;
        bool m_HasClicked{ false };

        bool m_DrawBbox{ false };
        std::vector<std::future<void>> m_CmdLists{};
        std::vector<std::future<void>> m_BufferedCmdLists{};
        //uint32_t m_BufferedIndex = 0;

        bool m_RenderingStopped{ false };

        std::vector<VkSampler> m_SwapChainSamplers{};
        std::vector<VkSampler> m_SwapChainDepthSamplers{};

        std::vector<IObserver*> m_Observers{};

        std::vector<VkImage> m_DepthMapImages;
        std::vector<VkImageView> m_DepthMapImageViews;
        std::vector<VkSampler> m_DepthMapSamplers;

        std::unordered_map<std::string, VulkanPipeline> m_Pipelines;

        std::vector<VkSemaphore> m_EntitiesSemaRenderFinished;
        std::vector<VkSemaphore> m_SkyboxSemaRenderFinished;
        std::vector<VkSemaphore> m_HUDSemaRenderFinished;
        std::vector<VkSemaphore> m_ShadowMapSemaRenderFinished;

        std::vector<VkSemaphore> m_ImageAvailable;
        std::vector<VkSemaphore> m_ShadowMapSemaImageAvailable;

        std::vector<VkFence> m_ImagesInFlight{};
        std::vector<VkFence> m_InFlightFences{};

        std::mutex m_MutexQueueSubmit;
        std::vector<DrawCommand*> m_CmdsToSubmit{};

        std::vector<std::unique_ptr<DrawCommand>> m_CmdSkyboxStatus{};
        std::vector<std::unique_ptr<DrawCommand>> m_CmdEntitiesStatus{};
        std::vector<std::unique_ptr<DrawCommand>> m_CmdHUDStatus{};

        std::atomic_bool m_Nodraw{ true };
    };
}
