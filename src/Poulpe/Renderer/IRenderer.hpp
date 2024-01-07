#pragma once

#include "Poulpe/Component/Camera.hpp"

#include "Poulpe/Core/IObserver.hpp"

#include "Poulpe/Renderer/Vulkan/VulkanAPI.hpp"

#include <stb_image.h>

namespace Poulpe
{
  class Entity;

  class IRenderer
  {
  public:
    virtual ~IRenderer() = default;

    virtual inline void addCamera(Camera* const camera) = 0;
    virtual void addEntities(std::vector<Entity*> entities) = 0;
    virtual void addPipeline(std::string const & shaderName, VulkanPipeline pipeline) = 0;
    virtual void attachObserver(IObserver* const observer) = 0;
    virtual void destroy() = 0;
    virtual void draw() = 0;
    virtual inline Camera* getCamera() = 0;
    virtual VkCommandPool createCommandPool() = 0;
    virtual VkDescriptorPool createDescriptorPool(
      std::vector<VkDescriptorPoolSize> & poolSizes, 
      uint32_t maxSets = 100,
      VkDescriptorPoolCreateFlags flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT) = 0;
    virtual VkDescriptorSet createDescriptorSets(
      VkDescriptorPool const & descriptorPool,
      std::vector<VkDescriptorSetLayout> const & descriptorSetLayouts,
      uint32_t count = 100) = 0;
    virtual VkDescriptorSetLayout createDescriptorSetLayout(
      std::vector<VkDescriptorSetLayoutBinding> & pBindings) = 0;
    virtual VkPipeline createGraphicsPipeline(
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
      bool dynamicDepthBias = false) = 0;
    virtual VkPipelineLayout createPipelineLayout(std::vector<VkDescriptorSetLayout> & descriptorSetLayouts,
      std::vector<VkPushConstantRange> & pushConstants) = 0;
    virtual VkShaderModule createShaderModule(std::vector<char> & code) = 0;
    virtual std::vector<VkImageView>* getDepthMapImageViews() = 0;
    virtual std::vector<VkSampler>* getDepthMapSamplers() = 0;
    virtual inline std::vector<VkDescriptorSetLayout>* getDescriptorSetLayouts() = 0;
    virtual inline VkDevice getDevice() = 0;
    virtual VulkanPipeline* getPipeline(std::string const & shaderName) = 0;
    virtual inline glm::mat4 getPerspective() = 0;
    virtual inline std::vector<VkImage>* getSwapChainImages() = 0;
    virtual inline VkExtent2D getSwapChainExtent() const = 0;
    virtual void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function, int queueIndex = 0) = 0;
    virtual void init() = 0;
    virtual void renderScene() = 0;
    virtual void setDeltatime(float deltaTime) = 0;
    virtual void setDrawBbox(bool draw) = 0;
    virtual void showGrid(bool show) = 0;
    virtual void updateDescriptorSets(
      std::vector<Buffer> & uniformBuffers,
      VkDescriptorSet & descriptorSet,
      std::vector<VkDescriptorImageInfo> & imageInfo,
      VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) = 0;
    virtual void updateDescriptorSets(
      std::vector<Buffer>& uniformBuffers,
      std::vector<Buffer>& storageBuffers,
      VkDescriptorSet& descriptorSet,
      std::vector<VkDescriptorImageInfo>& imageInfo) = 0;
    virtual inline void updateUniformBuffer(Buffer & buffer, std::vector<UniformBufferObject>* uniformBufferObjects) = 0;
    virtual Buffer createVertexBuffer(VkCommandPool commandPool, std::vector<Vertex> vertices) = 0;
    virtual Buffer createVertex2DBuffer(VkCommandPool & commandPool, std::vector<Vertex2D> & vertices) = 0;
    virtual Buffer createIndexBuffer(VkCommandPool & commandPool, std::vector<uint32_t> const & indices) = 0;
    virtual Buffer createUniformBuffers(uint32_t uniformBuffersCount) = 0;
    virtual std::vector<VkCommandBuffer> allocateCommandBuffers(VkCommandPool commandPool, uint32_t size = 1,
        bool isSecondary = false) = 0;
    virtual void beginCommandBuffer(VkCommandBuffer commandBuffer,
      VkCommandBufferUsageFlagBits flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
      VkCommandBufferInheritanceInfo inheritanceInfo = {}) = 0;
    virtual void endCommandBuffer(VkCommandBuffer commandBuffer) = 0;
    virtual void createSkyboxTextureImage(VkCommandBuffer& commandBuffer, std::vector<stbi_uc*>& skyboxPixels, uint32_t texWidth,
      uint32_t texHeight, uint32_t mipLevels, VkImage& textureImage, VkFormat format) = 0;
    virtual VkImageView createSkyboxImageView(VkImage image, VkFormat format, uint32_t mipLevels,
       VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) = 0;
    virtual VkSampler createSkyboxTextureSampler(uint32_t mipLevels) = 0;
    virtual void createTextureImage(VkCommandBuffer& commandBuffer, stbi_uc* pixels, uint32_t texWidth, uint32_t texHeight,
      uint32_t mipLevels, VkImage& textureImage, VkFormat format) = 0;
    virtual VkImageView createImageView(VkImage image, VkFormat format, uint32_t mipLevels,
      VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) = 0;
    virtual VkSampler createTextureSampler(uint32_t mipLevels) = 0;
    virtual DeviceMemoryPool* getDeviceMemoryPool() = 0;
    virtual void clear() = 0;
    virtual inline void stopRendering() = 0;
    virtual void waitIdle() = 0;
    virtual void shouldRecreateSwapChain() = 0;
    virtual void recreateSwapChain() = 0;
    virtual inline VkPhysicalDeviceProperties getDeviceProperties() const = 0;
    virtual Buffer createStorageBuffers(size_t storageBuffer) = 0;
    virtual void updateStorageBuffer(Buffer & buffer, ObjectBuffer objectBuffer) = 0;
  };
}
