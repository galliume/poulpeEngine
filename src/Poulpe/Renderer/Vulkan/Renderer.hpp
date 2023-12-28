#pragma once

#include "Poulpe/Core/IObserver.hpp"

#include "Poulpe/GUI/Window.hpp"

#include "Poulpe/Manager/IComponentManager.hpp"
#include "Poulpe/Manager/IEntityManager.hpp"
#include "Poulpe/Manager/ILightManager.hpp"
#include "Poulpe/Manager/ITextureManager.hpp"

#include "Poulpe/Renderer/IRenderer.hpp"

#include <future>

namespace Poulpe
{
    class Renderer : public IRenderer, public std::enable_shared_from_this<Renderer>
    {

    public:

      struct DrawCommand {
        VkCommandBuffer* buffer;
        VkPipelineStageFlags stageFlags;
        VkSemaphore* semaphore;
        std::atomic_bool done{false};
      };

        Renderer(
          Window* const window,
          IEntityManager* const entityManager,
          ComponentManager* const componentManager,
          ILightManager* const lightManager
        );
        ~Renderer() = default;

        void init() override;
        void addCamera(Camera* camera) override { m_Camera = camera; }
        VkShaderModule createShaderModule(std::vector<char> & code) override {
            return m_API->createShaderModule(code);
        }

        VkDescriptorPool createDescriptorPool(std::vector<VkDescriptorPoolSize> & poolSizes, uint32_t maxSets = 100,
            VkDescriptorPoolCreateFlags flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT) override {
                return m_API->createDescriptorPool(poolSizes, maxSets, flags);
        }

        VkDescriptorSet createDescriptorSets(VkDescriptorPool const & descriptorPool,
            std::vector<VkDescriptorSetLayout>  const & descriptorSetLayouts, uint32_t count = 100) override {
                return m_API->createDescriptorSets(descriptorPool, descriptorSetLayouts, count);
            };

        VkDescriptorSetLayout createDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> & pBindings) override {
            return m_API->createDescriptorSetLayout(pBindings);
        }

        VkPipeline createGraphicsPipeline(
            VkPipelineLayout pipelineLayout,
            std::string_view name,
            std::vector<VkPipelineShaderStageCreateInfo> shadersCreateInfos,
            VkPipelineVertexInputStateCreateInfo & vertexInputInfo,
            VkCullModeFlagBits cullMode = VK_CULL_MODE_BACK_BIT,
            bool depthTestEnable = true,
            bool depthWriteEnable = true,
            bool stencilTestEnable = true,
            int polygoneMode = VK_POLYGON_MODE_FILL,
            bool hasColorAttachment = true,
            bool dynamicDepthBias = false) override;

        VkPipelineLayout createPipelineLayout(std::vector<VkDescriptorSetLayout> & descriptorSetLayouts,
            std::vector<VkPushConstantRange> & pushConstants) override {
                return m_API->createPipelineLayout(descriptorSetLayouts, pushConstants);
            }

        void draw() override;
        void destroy() override;
        void immediateSubmit(std::function<void(VkCommandBuffer cmd)> && function, int queueIndex = 0) override;
        void showGrid(bool show) override;
        inline std::vector<VkDescriptorSetLayout>* getDescriptorSetLayouts() override { return & m_DescriptorSetLayouts; }
        inline VkDevice getDevice() override { return m_API->getDevice(); }
        inline std::vector<VkImage>* getSwapChainImages() override { return & m_SwapChainImages; }
        inline VkExtent2D getSwapChainExtent() const override {
            return m_API->getSwapChainExtent();
        }
        inline glm::mat4 getPerspective() override { return m_Perspective; }
        void setDeltatime(float deltaTime) override;
        void renderScene() override;

        void clear() override;

        void drawEntities();
        void drawSkybox();
        void drawHUD();
        void drawBbox();
        //void RenderForImGui(VkCommandBuffer cmdBuffer, VkFramebuffer swapChainFramebuffer);

        void beginRendering(VkCommandBuffer commandBuffer, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE, bool continuousCmdBuffer = false);

        void endRendering(VkCommandBuffer commandBuffer);
        VkSwapchainKHR getSwapChain() { return m_SwapChain; };

        void shouldRecreateSwapChain() override;
        void recreateSwapChain() override;
        void setRayPick(float x, float y, float z, int width, int height);
        inline void setDrawBbox(bool draw) override { m_DrawBbox = draw; };
        bool getDrawBbox() { return m_DrawBbox; };
        void clearRendererScreen();
        inline void stopRendering() override { m_RenderingStopped = true; };
        
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

        inline Camera* getCamera() override { return m_Camera; }

        void drawShadowMap(std::vector<std::unique_ptr<Entity>>* entities, Light light);

        void addPipeline(std::string const & shaderName, VulkanPipeline pipeline) override;
        VulkanPipeline* getPipeline(std::string const & shaderName) override { return & m_Pipelines[shaderName]; };

        std::vector<VkImageView>* getDepthMapImageViews() override { return & m_DepthMapImageViews; };
        std::vector<VkSampler>* getDepthMapSamplers() override { return & m_DepthMapSamplers; };


        VkCommandPool createCommandPool() override {
            return m_API->createCommandPool();
        }
        void updateDescriptorSets(
            std::vector<Buffer> & uniformBuffers,
            VkDescriptorSet & descriptorSet,
            std::vector<VkDescriptorImageInfo> & imageInfo,
            VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) override {
                m_API->updateDescriptorSets(uniformBuffers, descriptorSet, imageInfo, type);
            }
        void updateDescriptorSets(
          std::vector<Buffer>& uniformBuffers,
          std::vector<Buffer>& storageBuffers,
          VkDescriptorSet& descriptorSet,
          std::vector<VkDescriptorImageInfo>& imageInfo) override {
            m_API->updateDescriptorSets(uniformBuffers, storageBuffers, descriptorSet, imageInfo);
          }
        
        Buffer createVertexBuffer(VkCommandPool commandPool, std::vector<Vertex> vertices) override {
            return m_API->createVertexBuffer(commandPool, vertices);
        }

        Buffer createVertex2DBuffer(VkCommandPool & commandPool, std::vector<Vertex2D> & vertices) override {
            return m_API->createVertex2DBuffer(commandPool, vertices);
        }
        
        Buffer createIndexBuffer(VkCommandPool & commandPool, std::vector<uint32_t> const & indices) override {
            return m_API->createIndexBuffer(commandPool, indices);
        }

        void updateUniformBuffer(Buffer & buffer, std::vector<UniformBufferObject>* uniformBufferObjects) override {
            m_API->updateUniformBuffer(buffer, uniformBufferObjects);
        }

        Buffer createUniformBuffers(uint32_t uniformBuffersCount) override {
            return m_API->createCubeUniformBuffers(uniformBuffersCount);
        }

        std::vector<VkCommandBuffer> allocateCommandBuffers(VkCommandPool commandPool, uint32_t size = 1,
            bool isSecondary = false) override {
                return m_API->allocateCommandBuffers(commandPool, size, isSecondary);
            }

        void beginCommandBuffer(VkCommandBuffer commandBuffer,
            VkCommandBufferUsageFlagBits flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
            VkCommandBufferInheritanceInfo inheritanceInfo = {}) override {
                return m_API->beginCommandBuffer(commandBuffer, flags, inheritanceInfo);
            };

        void endCommandBuffer(VkCommandBuffer commandBuffer) override {
            return m_API->endCommandBuffer(commandBuffer);
        }

        void createSkyboxTextureImage(VkCommandBuffer& commandBuffer, std::vector<stbi_uc*>& skyboxPixels, uint32_t texWidth,
            uint32_t texHeight, uint32_t mipLevels, VkImage& textureImage, VkFormat format) override {
                m_API->createSkyboxTextureImage(commandBuffer, skyboxPixels, texWidth,
                texHeight, mipLevels, textureImage, format);
            }

        VkImageView createSkyboxImageView(VkImage image, VkFormat format, uint32_t mipLevels,
            VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) override {
                return m_API->createSkyboxImageView(image, format, mipLevels, aspectFlags);
        }

        VkSampler createSkyboxTextureSampler(uint32_t mipLevels) override {
            return m_API->createSkyboxTextureSampler(mipLevels);
        }

        void createTextureImage(VkCommandBuffer& commandBuffer, stbi_uc* pixels, uint32_t texWidth, uint32_t texHeight,
            uint32_t mipLevels, VkImage& textureImage, VkFormat format) override {
                return m_API->createTextureImage(commandBuffer, pixels, texWidth, texHeight, mipLevels, textureImage, format);
            };
    
        VkImageView createImageView(VkImage image, VkFormat format, uint32_t mipLevels,
            VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) override {
                return m_API->createImageView(image, format, mipLevels, aspectFlags);
            }

        VkSampler createTextureSampler(uint32_t mipLevels) override {
            return m_API->createTextureSampler(mipLevels);
        }

        DeviceMemoryPool* getDeviceMemoryPool() override { return m_API->getDeviceMemoryPool(); }
      
        void waitIdle() override {
            m_API->waitIdle();
        }

        VkPhysicalDeviceProperties getDeviceProperties() const override { return m_API->getDeviceProperties(); }
        Buffer createStorageBuffers(size_t storageBuffer) override {
            return m_API->createStorageBuffers(storageBuffer);
        };
        
        void updateStorageBuffer(Buffer & buffer, ObjectBuffer objectBuffer) override {
            m_API->updateStorageBuffer(buffer, objectBuffer);
        }

    public:
        //IMGUI config
        static std::atomic<float> s_FogDensity;
        static std::atomic<float> s_AmbiantLight;
        static std::atomic<float> s_FogColor[3];
        static std::atomic<int> s_Crosshair;
        static std::atomic<int> s_PolygoneMode;

    private:
        const size_t m_MAX_FRAMES_IN_FLIGHT{ 2 };

        //@todo temp
        void setPerspective();
        void submit();
        void onFinishRender();

    private:
        std::unique_ptr<VulkanAPI> m_API{ nullptr };
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
        IEntityManager* m_EntityManager{ nullptr };
        ComponentManager* m_ComponentManager{ nullptr };
        ILightManager* m_LightManager{ nullptr };

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
