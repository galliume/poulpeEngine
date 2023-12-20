#pragma once

#include "Poulpe/Core/Buffer.hpp"

#include "Poulpe/Renderer/IRenderer.hpp"

#include "Poulpe/Renderer/Vulkan/DeviceMemoryPool.hpp"

#include <stb_image.h>

namespace Poulpe {

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT const * pCreateInfo,
        VkAllocationCallbacks const * pAllocator, VkDebugUtilsMessengerEXT* pCallback);

    struct VulkanPipeline
    {
        VkPipelineLayout pipelineLayout;
        VkDescriptorPool descPool;
        VkDescriptorSetLayout descSetLayout;
        VkDescriptorSet descSet;
        VkPipelineCache pipelineCache;
        VkPipeline pipeline;
        std::vector<VkPipelineShaderStageCreateInfo> shaders;
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class VulkanRenderer : public IRenderer
    {
    public:
        VulkanRenderer(Window* window);
        ~VulkanRenderer();

        /**
        * Vulkan init functions, before main loop.
        **/
        VkRenderPass* createRenderPass(VkSampleCountFlagBits const & msaaSamples);

        VkShaderModule createShaderModule(std::vector<char> const & code);

        VkDescriptorPool createDescriptorPool(std::vector<VkDescriptorPoolSize> const & poolSizes, uint32_t maxSets = 100,
            VkDescriptorPoolCreateFlags flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);

        VkDescriptorSetLayout createDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> const & pBindings);

        VkDescriptorSet createDescriptorSets(VkDescriptorPool const & descriptorPool,
            std::vector<VkDescriptorSetLayout> const  & descriptorSetLayouts, uint32_t count = 100);

        void updateDescriptorSets(std::vector<Mesh::Buffer> & uniformBuffers, VkDescriptorSet & descriptorSet,
            std::vector<VkDescriptorImageInfo> & imageInfo, VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

        void updateDescriptorSets(
          std::vector<Mesh::Buffer>& uniformBuffers,
          std::vector<Mesh::Buffer>& storageBuffers,
          VkDescriptorSet& descriptorSet,
          std::vector<VkDescriptorImageInfo>& imageInfo);

        void updateDescriptorSet(Mesh::Buffer & uniformBuffer, VkDescriptorSet & descriptorSet,
            std::vector<VkDescriptorImageInfo> & imageInfo, VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

        void updateDescriptorSet(Mesh::Buffer & uniformBuffer, Mesh::Buffer & storageBuffer, VkDescriptorSet & descriptorSet,
            std::vector<VkDescriptorImageInfo> & imageInfo);

        void updateStorageDescriptorSets(std::vector<Mesh::Buffer> & uniformBuffers, VkDescriptorSet & descriptorSet, 
            VkDescriptorType type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

        VkPipelineLayout createPipelineLayout(std::vector<VkDescriptorSetLayout> const & descriptorSetLayouts,
            std::vector<VkPushConstantRange> const & pushConstants);

        VkPipeline createGraphicsPipeline(VkPipelineLayout pipelineLayout, std::string_view name,
            std::vector<VkPipelineShaderStageCreateInfo>shadersCreateInfos, VkPipelineVertexInputStateCreateInfo vertexInputInfo,
            VkCullModeFlagBits cullMode = VK_CULL_MODE_BACK_BIT, bool depthTestEnable = true,
            bool depthWriteEnable = true, bool stencilTestEnable = true, int polygoneMode = VK_POLYGON_MODE_FILL, bool hasColorAttachment = true, bool dynamicDepthBias = false);

        VkSwapchainKHR createSwapChain(std::vector<VkImage> & swapChainImages,
            VkSwapchainKHR const & oldSwapChain = VK_NULL_HANDLE);

        std::vector<VkFramebuffer> createFramebuffers(VkRenderPass renderPass, std::vector<VkImageView> swapChainImageViews,
            std::vector<VkImageView> depthImageView, std::vector<VkImageView> colorImageView);

        VkCommandPool createCommandPool();

        std::vector<VkCommandBuffer> allocateCommandBuffers(VkCommandPool commandPool, uint32_t size = 1,
            bool isSecondary = false);

        VkBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage);

        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
            VkBuffer & buffer, VkDeviceMemory & bufferMemory);

        Mesh::Buffer createVertexBuffer(VkCommandPool commandPool, std::vector<Poulpe::Vertex> vertices);

        Mesh::Buffer createVertex2DBuffer(VkCommandPool const & commandPool, std::vector<Poulpe::Vertex2D> const & vertices);

        Mesh::Buffer createIndexBuffer(VkCommandPool const & commandPool, std::vector<uint32_t> const & indices);

        //std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> createSyncObjects(std::vector<VkImage> swapChainImages);
        
        VkImageMemoryBarrier setupImageMemoryBarrier(VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
            VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = VK_REMAINING_MIP_LEVELS,
            VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);

        void copyBuffer(VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, int queueIndex = 0);

        bool souldResizeSwapChain();

        Mesh::Buffer createUniformBuffers(uint32_t uniformBuffersCount);

        Mesh::Buffer createCubeUniformBuffers(uint32_t uniformBuffersCount);

        void updateUniformBuffer(Mesh::Buffer & buffer, std::vector<UniformBufferObject>* uniformBufferObjects);

        void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,VkFormat format,
            VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image);

        void createSkyboxImage(uint32_t width, uint32_t height, VkSampleCountFlagBits numSamples, VkFormat format,
            VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image);

        VkImageView createImageView(VkImage image, VkFormat format, uint32_t mipLevels,
            VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
        
        VkImageView createSkyboxImageView(VkImage image, VkFormat format, uint32_t mipLevels,
            VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);

        void createTextureImage(VkCommandBuffer& commandBuffer, stbi_uc* pixels, uint32_t texWidth, uint32_t texHeight,
            uint32_t mipLevels, VkImage& textureImage, VkFormat format);

        void createSkyboxTextureImage(VkCommandBuffer& commandBuffer, std::vector<stbi_uc*>& skyboxPixels, uint32_t texWidth,
            uint32_t texHeight, uint32_t mipLevels, VkImage& textureImage, VkFormat format);

        void copyBufferToImage(VkCommandBuffer& commandBuffer, VkBuffer& buffer, VkImage& image, uint32_t width,
            uint32_t height, uint32_t mipLevels = 0);

        void copyBufferToImageSkybox(VkCommandBuffer & commandBuffer, VkBuffer & buffer, VkImage & image, uint32_t width,
            uint32_t height, std::vector<stbi_uc*>skyboxPixels, uint32_t mipLevels, uint32_t layerSize);

        VkSampler createTextureSampler(uint32_t mipLevels);

        VkSampler createSkyboxTextureSampler(uint32_t mipLevels);

        VkImageView createDepthResources(VkCommandBuffer commandBuffer);

        VkFormat findSupportedFormat(std::vector<VkFormat> const & candidates, VkImageTiling tiling,
            VkFormatFeatureFlags features);

        VkFormat findDepthFormat();

        bool hasStencilComponent(VkFormat format);

        VkDeviceSize getMaxMemoryHeap() { return m_MaxMemoryHeap; }

        void initMemoryPool();

        Mesh::Buffer createStorageBuffers(size_t storageBuffer);

        void updateStorageBuffer(Mesh::Buffer & buffer, ObjectBuffer objectBuffer);

        void setResolution(unsigned int width, unsigned int height);

        /**
        * Vulkan drawing functions, in main loop
        **/
        void resetCommandPool(VkCommandPool commandPool);

        void beginCommandBuffer(VkCommandBuffer commandBuffer,
            VkCommandBufferUsageFlagBits flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
            VkCommandBufferInheritanceInfo inheritanceInfo = {});
        void endCommandBuffer(VkCommandBuffer commandBuffer);

        void beginRenderPass(VkRenderPass renderPass, VkCommandBuffer commandBuffer, VkFramebuffer framebuffer);

        void endRenderPass(VkCommandBuffer commandBuffer);

        void beginRendering(VkCommandBuffer commandBuffer,
            VkImageView & colorImageView,
            VkImageView & depthImageView,
            VkAttachmentLoadOp const loadOp,
            VkAttachmentStoreOp const storeOp);

        void endRendering(VkCommandBuffer commandBuffer);

        void setViewPort(VkCommandBuffer commandBuffer);

        void setScissor(VkCommandBuffer commandBuffer);

        void bindPipeline(VkCommandBuffer commandBuffer, VkPipeline pipeline);

        void draw(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, VulkanPipeline& pipeline, Mesh::Data * data,
            uint32_t uboCount, bool drawIndexed = true, uint32_t index = 0);

        VkResult queueSubmit(VkCommandBuffer commandBuffer, int queueIndex = 0);

        void addPipelineBarriers(VkCommandBuffer commandBuffer, std::vector<VkImageMemoryBarrier> renderBarriers,
            VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags);

        void generateMipmaps(VkCommandBuffer commandBuffer, VkFormat imageFormat, VkImage image, uint32_t texWidth,
            uint32_t texHeight, uint32_t mipLevels, uint32_t layerCount = 1);

        /**
        * Vulkan clean and destroy
        **/
        //void destroyPipeline(VkPipeline pipeline);

        //void destroyPipelineData(VkPipelineLayout pipelineLayout, VkDescriptorPool descriptorPool,
        //    VkDescriptorSetLayout descriptorSetLayout);

        //void destroySwapchain(VkDevice device, VkSwapchainKHR swapChain, std::vector<VkFramebuffer> swapChainFramebuffers,
        //    std::vector<VkImageView> swapChainImageViews);

        //void destroySemaphores(std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores);

        //void destroyFences();

        //void destroyBuffer(VkBuffer buffer);

        //void destroyRenderPass(VkRenderPass* renderPass, VkCommandPool commandPool,
        //    std::vector<VkCommandBuffer> commandBuffers);

        void destroy();

        /*
        * Helper functions.
        */
        inline const std::vector<const char*> getValidationLayers() const { return m_ValidationLayers; }

        inline const std::vector<VkExtensionProperties> getExtensions() const { return m_Extensions; }

        inline const std::vector<VkLayerProperties> getLayersAvailable() const { return m_LayersAvailable; }

        inline bool isInstanceCreated() const { return m_InstanceCreated; }

        inline bool isValidationLayersEnabled() const { return m_EnableValidationLayers; }

        inline uint32_t getExtensionCount() const { return m_ExtensionCount; }

        inline uint32_t getQueueFamily() const { return m_QueueFamilyIndices.graphicsFamily.value(); }

        inline VkInstance getInstance() const { return m_Instance; };

        inline VkPhysicalDevice getPhysicalDevice() const { return m_PhysicalDevice; }

        inline VkDevice getDevice() const { return m_Device; }

        inline std::vector<VkQueue> getGraphicsQueues() const { return m_GraphicsQueues; }

        inline VkPhysicalDeviceProperties getDeviceProperties() const { return m_DeviceProps; }

        inline VkPhysicalDeviceFeatures getDeviceFeatures() const { return m_DeviceFeatures; }

        inline bool isFramebufferResized() { return m_FramebufferResized; }

        inline VkExtent2D getSwapChainExtent() const { return m_SwapChainExtent; }

        inline VkSurfaceKHR getSurface() const { return m_Surface; }

        inline VkSurfaceFormatKHR getSurfaceFormat() const { return m_SurfaceFormat; }

        inline void resetCurrentFrameIndex() { m_CurrentFrame = 0; }

        inline int32_t getCurrentFrame() const { return m_CurrentFrame; }

        inline VkFormat getSwapChainImageFormat() const { return m_SwapChainImageFormat; }

        inline VkSampleCountFlagBits getMsaaSamples() const { return m_MsaaSamples; }

        void initDetails();

        VkSampleCountFlagBits getMaxUsableSampleCount();

        const SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

        uint32_t getImageCount() const;

        std::string getAPIVersion();

       DeviceMemoryPool* getDeviceMemoryPool() { return m_DeviceMemoryPool.get(); }

        void startMarker(VkCommandBuffer buffer, std::string const & name, float r, float g, float b, float a = 1.0);

        void endMarker(VkCommandBuffer buffer);

        uint32_t getQueueCount() { return m_queueCount; }

        std::vector<VkQueue> getPresentQueue() { return m_PresentQueues; };

        void waitIdle();

        void createDepthMapImage(VkImage & image);
        VkImageView createDepthMapImageView(VkImage image);
        VkSampler createDepthMapSampler();
        void createDepthMapFrameBuffer(VkRenderPass & renderPass, VkImageView & imageView, VkFramebuffer & frameBuffer);

        static const std::string getVendor(int vendorID)
        {
            std::unordered_map<int, std::string> vendors;
            vendors[0x1002] = "AMD";
            vendors[0x1010] = "ImgTec";
            vendors[0x10DE] = "NVIDIA";
            vendors[0x13B5] = "ARM";
            vendors[0x5143] = "Qualcomm";
            vendors[0x8086] = "INTEL";

            return vendors[vendorID];
        }

        void transitionImageLayout(
          VkCommandBuffer commandBuffer,
          VkImage image,
          VkImageLayout oldLayout,
          VkImageLayout newLayout,
          VkImageAspectFlags aspectFlags);

    public:
        bool m_FramebufferResized = false;

    private:
        bool isDeviceSuitable(VkPhysicalDevice device);
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        bool checkValidationLayerSupport();
        void createInstance();
        void enumerateExtensions();
        void loadRequiredExtensions();
        void setupDebugMessenger();
        void createLogicalDevice();
        void pickPhysicalDevice();
        void createSurface();

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const & availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR> const & availablePresentModes);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR const & capabilities);

    private:
        int32_t m_CurrentFrame{ 0 };
        uint32_t m_ExtensionCount;
        std::string m_apiVersion;
        const uint32_t m_queueCount{ 2 };

        Window* m_Window{ nullptr };

        const std::vector<const char*> m_ValidationLayers{ "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> m_DeviceExtensions{
            VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
            VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
            VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME
        };

        bool m_InstanceCreated{ false };
        bool m_EnableValidationLayers{ false };

        VkInstance m_Instance = VK_NULL_HANDLE;
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties m_DeviceProps;
        VkPhysicalDeviceFeatures m_DeviceFeatures;
        VkPhysicalDeviceProperties2 m_DeviceProperties2;
        VkPhysicalDeviceMaintenance3Properties m_DeviceMaintenance3Properties;

        VkDevice m_Device{ VK_NULL_HANDLE };
        std::vector<VkQueue> m_GraphicsQueues{};
        std::vector<VkQueue> m_PresentQueues{};
        VkSurfaceKHR m_Surface{ VK_NULL_HANDLE };
        VkFormat m_SwapChainImageFormat{};
        VkExtent2D m_SwapChainExtent{};
        VkDebugUtilsMessengerEXT m_DebugMessengerCallback{ VK_NULL_HANDLE };
        QueueFamilyIndices m_QueueFamilyIndices{};
        VkSurfaceFormatKHR m_SurfaceFormat{};
        VkPresentModeKHR m_PresentMode{};
        SwapChainSupportDetails m_SwapChainSupport{};

        std::vector<VkLayerProperties> m_LayersAvailable{};
        std::vector<VkExtensionProperties> m_Extensions{};
        std::vector<const char*> m_RequiredExtensions{};
        std::vector<VkFence> m_InFlightFences{};
        std::vector<VkFence> m_ImagesInFlight{};
        VkFence m_Fence{};

        VkSampleCountFlagBits m_MsaaSamples{ VK_SAMPLE_COUNT_8_BIT };

        std::mutex m_MutexQueueSubmit{};
        std::mutex m_MutexDraw{};
        std::mutex m_MutexCmdBuffer{};
        std::mutex m_MutexAcquireNextImage{};
        std::mutex m_MutexGraphicsPipeline{};

        VkDeviceSize m_MaxMemoryHeap{};
        std::unique_ptr<DeviceMemoryPool> m_DeviceMemoryPool{ nullptr };

        VkFence m_FenceAcquireImage{};
        VkFence m_FenceSubmit{};
        VkFence m_FenceBuffer{};

        //@todo move to config file
        unsigned int m_Width{ 800 };
        unsigned int m_Height{ 600 };
        //unsigned int m_Width{ 1200 };
        //unsigned int m_Height{ 720 };

        //VkMemoryRequirements m_MemRequirements;
    };
}
