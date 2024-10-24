#pragma once

#include "Poulpe/GUI/Window.hpp"

#include "Poulpe/Manager/IComponentManager.hpp"
#include "Poulpe/Manager/IEntityManager.hpp"
#include "Poulpe/Manager/ILightManager.hpp"
#include "Poulpe/Manager/ITextureManager.hpp"

#include "Poulpe/Renderer/IRenderer.hpp"

#include <future>

namespace Poulpe
{
  struct DrawCommand {
    VkCommandBuffer* buffer;
    VkPipelineStageFlags stageFlags;
    VkSemaphore* semaphore;
    std::atomic_bool done{false};
  };

  class Renderer : public IRenderer, public std::enable_shared_from_this<Renderer>
  {

  public:

    Renderer(
      Window* const window,
      IEntityManager* const entityManager,
      ComponentManager* const componentManager,
      ILightManager* const lightManager,
      ITextureManager* const textureManager
    );
    ~Renderer() override = default;

    inline void addCamera(Camera* const camera) override { m_Camera = camera; }
    void addEntities(std::vector<Entity*> entities) override;
    void addEntity(Entity* entity) override;
    void updateData(std::string const& name, UniformBufferObject const& ubo, std::vector<Vertex> const& vertices) override;
    void addPipeline(std::string const & shaderName, VulkanPipeline pipeline) override;
    void attachObserver(IObserver* const observer) override;
    void beginRendering(VkCommandBuffer commandBuffer,
      VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
      VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      bool continuousCmdBuffer = false);
    void clear() override;
    void clearRendererScreen();
    void destroy() override;
    void drawBbox();
    void drawEntities();
    void drawHUD();
    void drawShadowMap();
    void drawSkybox();
    void endRendering(VkCommandBuffer commandBuffer);
    inline Camera* getCamera() override { return m_Camera; }
    inline uint32_t getCurrentFrameIndex() const { return m_CurrentFrame; }
    inline std::vector<VkImageView>* getDepthMapImageViews() override { return & m_DepthMapImageViews; }
    inline std::vector<VkSampler>* getDepthMapSamplers() override { return & m_DepthMapSamplers; }
    inline bool getDrawBbox() { return m_DrawBbox; }
    inline std::vector<VkDescriptorSetLayout>* getDescriptorSetLayouts() override { return & m_DescriptorSetLayouts; }
    inline VkDevice getDevice() override { return m_API->getDevice(); }
    inline glm::mat4 getPerspective() override { return m_Perspective; }
    VulkanPipeline* getPipeline(std::string const & shaderName) override { return & m_Pipelines[shaderName]; }
    inline VkSwapchainKHR getSwapChain() { return m_SwapChain; }
    inline std::vector<VkImage>* getSwapChainImages() override { return & m_SwapChainImages; }
    inline  std::vector<VkImageView>* getSwapChainImageViews() { return &m_SwapChainImageViews; }
    void immediateSubmit(std::function<void(VkCommandBuffer cmd)> && function, int queueIndex = 0) override;
    void init() override;
    void recreateSwapChain() override;
    void renderScene() override;
    void setDeltatime(float deltaTime) override;
    inline void setDrawBbox(bool draw) override { m_DrawBbox = draw; }
    void setRayPick(float x, float y, float z, int width, int height);
    void shouldRecreateSwapChain() override;
    void showGrid(bool show) override;
    inline void stopRendering() override { m_RenderingStopped = true; }

    //void RenderForImGui(VkCommandBuffer cmdBuffer, VkFramebuffer swapChainFramebuffer);
    //std::pair<VkSampler, VkImageView> getImguiTexture() {
    //    return std::make_pair(m_SwapChainSamplers[m_CurrentFrame], m_SwapChainImageViews[m_CurrentFrame]);
    //};
    //std::pair<VkSampler, VkImageView> getImguiDepthImage() {
    //    return std::make_pair(m_SwapChainDepthSamplers[m_CurrentFrame], m_SwapChainDepthImageViews[m_CurrentFrame]);
    //};
    //@todo add GuiManager
    //VkRenderPass createImGuiRenderPass(VkFormat format);

    //@todo clean API call
    std::vector<VkCommandBuffer> allocateCommandBuffers(VkCommandPool commandPool,
      uint32_t size = 1,
      bool isSecondary = false) override {
        return m_API->allocateCommandBuffers(commandPool, size, isSecondary);
    }
    void beginCommandBuffer(VkCommandBuffer commandBuffer,
      VkCommandBufferUsageFlagBits flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
      VkCommandBufferInheritanceInfo inheritanceInfo = {}) override {
        return m_API->beginCommandBuffer(commandBuffer, flags, inheritanceInfo);
    }
    VkCommandPool createCommandPool() override {
      return m_API->createCommandPool();
    }
    VkDescriptorPool createDescriptorPool(std::vector<VkDescriptorPoolSize> & poolSizes,
      uint32_t maxSets = 100,
      VkDescriptorPoolCreateFlags flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT) override {
        return m_API->createDescriptorPool(poolSizes, maxSets, flags);
    }
    VkDescriptorSetLayout createDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> & pBindings) override {
      return m_API->createDescriptorSetLayout(pBindings);
    }
    VkDescriptorSet createDescriptorSets(VkDescriptorPool const & descriptorPool,
      std::vector<VkDescriptorSetLayout> const & descriptorSetLayouts,
      uint32_t count = 100) override {
        return m_API->createDescriptorSets(descriptorPool, descriptorSetLayouts, count);
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
    Buffer createIndexBuffer(VkCommandPool & commandPool,
      std::vector<uint32_t> const & indices) override {
        return m_API->createIndexBuffer(commandPool, indices);
    }
    VkImageView createImageView(VkImage image,
      VkFormat format,
      uint32_t mipLevels,
      VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) override {
        return m_API->createImageView(image, format, mipLevels, aspectFlags);
    }
    VkPipelineLayout createPipelineLayout(std::vector<VkDescriptorSetLayout> & descriptorSetLayouts,
      std::vector<VkPushConstantRange> & pushConstants) override {
        return m_API->createPipelineLayout(descriptorSetLayouts, pushConstants);
    }
    VkShaderModule createShaderModule(std::vector<char> & code) override {
      return m_API->createShaderModule(code);
    }
    void createSkyboxTextureImage(VkCommandBuffer& commandBuffer,
      std::vector<stbi_uc*>& skyboxPixels,
      uint32_t texWidth,
      uint32_t texHeight,
      uint32_t mipLevels,
      VkImage& textureImage,
      VkFormat format) override {
        m_API->createSkyboxTextureImage(
          commandBuffer,
          skyboxPixels,
          texWidth,
          texHeight,
          mipLevels,
          textureImage,
          format);
    }
    VkImageView createSkyboxImageView(VkImage image,
      VkFormat format,
      uint32_t mipLevels,
      VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) override {
        return m_API->createSkyboxImageView(image, format, mipLevels, aspectFlags);
    }
    VkSampler createSkyboxTextureSampler(uint32_t mipLevels) override {
      return m_API->createSkyboxTextureSampler(mipLevels);
    }
    Buffer createStorageBuffers(size_t storageBuffer) override {
      return m_API->createStorageBuffers(storageBuffer);
    }
    void createTextureImage(VkCommandBuffer& commandBuffer,
      stbi_uc* pixels,
      uint32_t texWidth,
      uint32_t texHeight,
      uint32_t mipLevels,
      VkImage& textureImage,
      VkFormat format) override {
        return m_API->createTextureImage(commandBuffer,
          pixels,
          texWidth,
          texHeight,
          mipLevels,
          textureImage,
          format);
    }
    VkSampler createTextureSampler(uint32_t mipLevels) override {
      return m_API->createTextureSampler(mipLevels);
    }
    Buffer createUniformBuffers(uint32_t uniformBuffersCount) override {
      return m_API->createUniformBuffers(uniformBuffersCount);
    }
    Buffer createVertexBuffer(VkCommandPool commandPool,
      std::vector<Vertex> vertices) override {
        return m_API->createVertexBuffer(commandPool, vertices);
    }
    Buffer createVertex2DBuffer(VkCommandPool & commandPool,
      std::vector<Vertex2D> & vertices) override {
        return m_API->createVertex2DBuffer(commandPool, vertices);
    }
    void endCommandBuffer(VkCommandBuffer commandBuffer) override {
      return m_API->endCommandBuffer(commandBuffer);
    }
    DeviceMemoryPool* getDeviceMemoryPool() override { return m_API->getDeviceMemoryPool(); }
    VkPhysicalDeviceProperties getDeviceProperties() const override { return m_API->getDeviceProperties(); }
    inline VkExtent2D getSwapChainExtent() const override {
      return m_API->getSwapChainExtent();
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
    void updateUniformBuffer(Buffer & buffer,
      std::vector<UniformBufferObject>* uniformBufferObjects) override {
        m_API->updateUniformBuffer(buffer, uniformBufferObjects);
    }
    void updateStorageBuffer(Buffer & buffer,
      ObjectBuffer objectBuffer) override {
        m_API->updateStorageBuffer(buffer, objectBuffer);
    }
    void waitIdle() override { m_API->waitIdle(); }

    std::string getAPIVersion() override { return m_API->getAPIVersion(); }

  public:
    //IMGUI config
    static std::atomic<float> s_FogDensity;
    static std::atomic<float> s_AmbiantLight;
    static std::atomic<float> s_FogColor[3];
    static std::atomic<int> s_Crosshair;
    static std::atomic<int> s_PolygoneMode;

  private:
    const size_t m_MAX_FRAMES_IN_FLIGHT{ 2 };

    void onFinishRender();
    void setPerspective();
    void submit();

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
    ITextureManager* m_TextureManager{ nullptr };

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
    bool m_DepthMapDescSetUpdated{ false };

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
    std::mutex m_MutexEntitySubmit;
    std::vector<DrawCommand*> m_CmdsToSubmit{};

    std::vector<std::unique_ptr<DrawCommand>> m_CmdSkyboxStatus{};
    std::vector<std::unique_ptr<DrawCommand>> m_CmdEntitiesStatus{};
    std::vector<std::unique_ptr<DrawCommand>> m_CmdHUDStatus{};

    std::atomic_bool m_Nodraw{ true };

    std::vector<Entity*> m_Entities{};
  };
}
