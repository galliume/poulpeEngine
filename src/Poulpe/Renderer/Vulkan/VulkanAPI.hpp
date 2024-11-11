#pragma once

#include "Poulpe/Component/Vertex2D.hpp"

#include "Poulpe/Core/PlpTypedef.hpp"

#include "Poulpe/Renderer/Vulkan/DeviceMemoryPool.hpp"

#include <stb_image.h>

namespace Poulpe {

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT const * pCreateInfo,
        VkAllocationCallbacks const * pAllocator, VkDebugUtilsMessengerEXT* pCallback);

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

    class VulkanAPI
    {
    public:
        VulkanAPI(Window* window);
        ~VulkanAPI();

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

        void updateDescriptorSets(std::vector<Buffer> & uniformBuffers, VkDescriptorSet & descriptorSet,
            std::vector<VkDescriptorImageInfo> & imageInfo, VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

        void updateDescriptorSets(
          std::vector<Buffer>& uniformBuffers,
          std::vector<Buffer>& storageBuffers,
          VkDescriptorSet& descriptorSet,
          std::vector<VkDescriptorImageInfo>& imageInfo);

        void updateDescriptorSet(Buffer & uniformBuffer, VkDescriptorSet & descriptorSet,
            std::vector<VkDescriptorImageInfo> & imageInfo, VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

        void updateDescriptorSet(Buffer & uniformBuffer, Buffer & storageBuffer, VkDescriptorSet & descriptorSet,
            std::vector<VkDescriptorImageInfo> & imageInfo);

        void updateStorageDescriptorSets(std::vector<Buffer> & uniformBuffers, VkDescriptorSet & descriptorSet, 
            VkDescriptorType type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

        VkPipelineLayout createPipelineLayout(std::vector<VkDescriptorSetLayout> const & descriptorSetLayouts,
            std::vector<VkPushConstantRange> const & pushConstants);

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
            bool dynamicDepthBias = false);

        VkSwapchainKHR createSwapChain(std::vector<VkImage> & swapChainImages,
            VkSwapchainKHR const & oldSwapChain = VK_NULL_HANDLE);

        std::vector<VkFramebuffer> createFramebuffers(VkRenderPass renderPass, std::vector<VkImageView> swapChainImageViews,
            std::vector<VkImageView> depth_imageview, std::vector<VkImageView> colorImageView);

        VkCommandPool createCommandPool();

        std::vector<VkCommandBuffer> allocateCommandBuffers(VkCommandPool& commandPool, uint32_t size = 1,
            bool isSecondary = false);

        VkBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage);

        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
            VkBuffer & buffer, VkDeviceMemory & bufferMemory);

        Buffer createVertexBuffer(VkCommandPool commandPool, std::vector<Vertex> vertices);

        Buffer createVertex2DBuffer(VkCommandPool const & commandPool, std::vector<Vertex2D> const & vertices);

        Buffer createIndexBuffer(VkCommandPool const & commandPool, std::vector<uint32_t> const & indices);

        //std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> createSyncObjects(std::vector<VkImage> swapChainImages);
        
        VkImageMemoryBarrier setupImageMemoryBarrier(VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
            VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = VK_REMAINING_MIP_LEVELS,
            VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);

        void copyBuffer(VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, int queueIndex = 0);

        bool souldResizeSwapChain();

        Buffer createUniformBuffers(
            uint32_t const uniformBuffersCount,
            VkCommandPool& commandPool);

        Buffer createCubeUniformBuffers(uint32_t uniformBuffersCount);

        void updateUniformBuffer(Buffer & buffer, std::vector<UniformBufferObject>* uniformBufferObjects);

        void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,VkFormat format,
            VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image);

        void createSkyboxImage(uint32_t width, uint32_t height, VkSampleCountFlagBits numSamples, VkFormat format,
            VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image);

        VkImageView createImageView(VkImage image, VkFormat format, uint32_t mipLevels,
            VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT);
        
        VkImageView createSkyboxImageView(VkImage image, VkFormat format, uint32_t mipLevels,
            VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT);

        void createTextureImage(VkCommandBuffer& cmd_buffer, stbi_uc* pixels, uint32_t texWidth, uint32_t texHeight,
            uint32_t mipLevels, VkImage& textureImage, VkFormat format);

        void createSkyboxTextureImage(VkCommandBuffer& cmd_buffer, std::vector<stbi_uc*>& skyboxPixels, uint32_t texWidth,
            uint32_t texHeight, uint32_t mipLevels, VkImage& textureImage, VkFormat format);

        void copyBufferToImage(VkCommandBuffer& cmd_buffer, VkBuffer& buffer, VkImage& image, uint32_t width,
            uint32_t height, uint32_t mipLevels = 0);

        void copyBufferToImageSkybox(VkCommandBuffer & cmd_buffer, VkBuffer & buffer, VkImage & image, uint32_t width,
            uint32_t height, std::vector<stbi_uc*>skyboxPixels, uint32_t mipLevels, uint32_t layerSize);

        VkSampler createTextureSampler(uint32_t mipLevels);

        VkSampler createSkyboxTextureSampler(uint32_t mipLevels);

        VkImageView createDepthResources(VkCommandBuffer& cmd_buffer);

        VkFormat findSupportedFormat(std::vector<VkFormat> const & candidates, VkImageTiling tiling,
            VkFormatFeatureFlags features);

        VkFormat findDepthFormat();

        bool hasStencilComponent(VkFormat format);

        VkDeviceSize getMaxMemoryHeap() { return _max_memory_heap; }

        void initMemoryPool();

        Buffer createStorageBuffers(
            ObjectBuffer const& storage_buffer,
            VkCommandPool& command_pool);
     
        Buffer createIndirectCommandsBuffer(std::vector<VkDrawIndexedIndirectCommand> const& drawCommands);

        void updateStorageBuffer(Buffer & buffer, ObjectBuffer objectBuffer);

        void setResolution(unsigned int width, unsigned int height);

        /**
        * Vulkan drawing functions, in main loop
        **/
        void resetCommandPool(VkCommandPool& commandPool);

        void beginCommandBuffer(
            VkCommandBuffer& cmd_buffer,
            VkCommandBufferUsageFlagBits flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
            VkCommandBufferInheritanceInfo inheritanceInfo = {});
        void endCommandBuffer(VkCommandBuffer& cmd_buffer);

        void beginRenderPass(VkRenderPass renderPass, VkCommandBuffer& cmd_buffer, VkFramebuffer framebuffer);

        void endRenderPass(VkCommandBuffer& cmd_buffer);

        void beginRendering(VkCommandBuffer& cmd_buffer,
            VkImageView & colorImageView,
            VkImageView & depth_imageview,
            VkAttachmentLoadOp const loadOp,
            VkAttachmentStoreOp const storeOp);

        void endRendering(VkCommandBuffer& cmd_buffer);

        void setViewPort(VkCommandBuffer& cmd_buffer);

        void setScissor(VkCommandBuffer& cmd_buffer);

        void bindPipeline(VkCommandBuffer& cmd_buffer, VkPipeline pipeline);

        void draw(VkCommandBuffer& cmd_buffer, VkDescriptorSet& descriptorSet, VulkanPipeline& pipeline, Data * data,
            bool drawIndexed = true, uint32_t index = 0);

        VkResult queueSubmit(VkCommandBuffer& cmd_buffer, int queueIndex = 0);
        void submit(
          VkQueue& queue,
          std::vector<VkSubmitInfo> const& submit_infos,
          VkPresentInfoKHR const& present_info,
          VkFence& fence);

        void addPipelineBarriers(VkCommandBuffer& cmd_buffer, std::vector<VkImageMemoryBarrier> renderBarriers,
            VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags);

        void generateMipmaps(VkCommandBuffer& cmd_buffer, VkFormat imageFormat, VkImage image, uint32_t texWidth,
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
        inline const std::vector<const char*> getValidationLayers() const { return _validation_layers; }

        inline const std::vector<VkExtensionProperties> getExtensions() const { return _extensions; }

        inline const std::vector<VkLayerProperties> getLayersAvailable() const { return _layers_available; }

        inline bool isInstanceCreated() const { return _instance_created; }

        inline bool isValidationLayersEnabled() const { return _enable_validation_layers; }

        inline uint32_t getExtensionCount() const { return _extension_count; }

        inline uint32_t getQueueFamily() const { return _queue_family_indices.graphicsFamily.value(); }

        inline VkInstance getInstance() const { return _instance; }

        inline VkPhysicalDevice getPhysicalDevice() const { return _physical_device; }

        inline VkDevice getDevice() const { return _device; }

        inline std::vector<VkQueue> getGraphicsQueues() const { return _graphics_queues; }

        inline VkPhysicalDeviceProperties getDeviceProperties() const { return _device_props; }

        inline VkPhysicalDeviceFeatures getDeviceFeatures() const { return _device_features; }

        inline bool isFramebufferResized() { return _FramebufferResized; }

        inline VkExtent2D getSwapChainExtent() const { return _swapchain_extent; }

        inline VkSurfaceKHR getSurface() const { return _surface; }

        inline VkSurfaceFormatKHR getSurfaceFormat() const { return _surface_format; }

        inline void resetCurrentFrameIndex() { _current_frame = 0; }

        inline int32_t getCurrentFrame() const { return _current_frame; }

        inline VkFormat getSwapChainImageFormat() const { return _swapchain_iImage_format; }

        inline VkSampleCountFlagBits getMsaaSamples() const { return _sample_count; }

        void initDetails();

        VkSampleCountFlagBits getMaxUsableSampleCount();

        const SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

        uint32_t getImageCount() const;

        std::string getAPIVersion();

        DeviceMemoryPool* getDeviceMemoryPool() { return _device_memory_pool.get(); }

        void startMarker(VkCommandBuffer buffer, std::string const & name, float r, float g, float b, float a = 1.0);

        void endMarker(VkCommandBuffer buffer);

        uint32_t getQueueCount() { return _queue_count; }

        std::vector<VkQueue> getPresentQueue() { return _present_queues; }

        void waitIdle();

        void createDepthMapImage(VkImage & image);
        VkImageView createDepthMapImageView(VkImage image);
        VkSampler createDepthMapSampler();
        void createDepthMapFrameBuffer(VkRenderPass & renderPass, VkImageView & imageview, VkFramebuffer & frameBuffer);

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
          VkCommandBuffer& cmd_buffer,
          VkImage& image,
          VkImageLayout old_layout,
          VkImageLayout new_layout,
          VkImageAspectFlags aspect_flags);

    public:
        bool _FramebufferResized = false;

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
        int32_t _current_frame{ 0 };
        uint32_t _extension_count;
        std::string _api_version;
        const uint32_t _queue_count{ 1 };

        Window* _window{ nullptr };

        const std::vector<const char*> _validation_layers{ "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> _device_extensions{
            VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
            VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME,
            VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
        };

        bool _instance_created{ false };
        bool _enable_validation_layers{ false };

        VkInstance _instance = VK_NULL_HANDLE;
        VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties _device_props;
        VkPhysicalDeviceFeatures _device_features;
        VkPhysicalDeviceProperties2 _device_props2;
        VkPhysicalDeviceMaintenance3Properties _device_maintenance3_props;

        VkDevice _device{ VK_NULL_HANDLE };
        std::vector<VkQueue> _graphics_queues{};
        std::vector<VkQueue> _present_queues{};
        VkSurfaceKHR _surface{ VK_NULL_HANDLE };
        VkFormat _swapchain_iImage_format{};
        VkExtent2D _swapchain_extent{};
        VkDebugUtilsMessengerEXT _debug_msg_callback{ VK_NULL_HANDLE };
        QueueFamilyIndices _queue_family_indices{};
        VkSurfaceFormatKHR _surface_format{};
        VkPresentModeKHR _present_mode{};
        SwapChainSupportDetails _swapchain_support{};

        std::vector<VkLayerProperties> _layers_available{};
        std::vector<VkExtensionProperties> _extensions{};
        std::vector<const char*> _required_extensions{};
        std::vector<VkFence> _in_flight_fences{};
        std::vector<VkFence> _images_in_flight{};

        VkSampleCountFlagBits _sample_count{ VK_SAMPLE_COUNT_8_BIT };

        std::mutex _mutex_queue_submit{};

        VkDeviceSize _max_memory_heap{};
        std::unique_ptr<DeviceMemoryPool> _device_memory_pool{ nullptr };

        VkFence _fence_acquire_image{};
        VkFence _fence_submit{};
        VkFence _fence_buffer{};

        //@todo move to config file
        // unsigned int _width{ 800 };
        // unsigned int _height{ 600 };
        unsigned int _width{ 2560 };
        unsigned int _height{ 1440 };

        //VkMemoryRequirements _MemRequirements;
    };
}
