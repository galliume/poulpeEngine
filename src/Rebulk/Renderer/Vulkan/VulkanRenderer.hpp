#pragma once

#include "Rebulk/Renderer/IRenderer.hpp"
#include "Rebulk/Renderer/Vulkan/DeviceMemoryPool.hpp"

namespace Rbk {

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback);

    struct VulkanPipeline
    {
        VkPipelineLayout pipelineLayout;
        VkDescriptorPool descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        VkPipelineCache pipelineCache;
        std::vector<VkPipeline> graphicsPipeline;
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
        VulkanRenderer(std::shared_ptr<Window> window);
        ~VulkanRenderer();

        /**
        * Vulkan init functions, before main loop.
        **/
        std::shared_ptr<VkRenderPass> CreateRenderPass(const VkSampleCountFlagBits& msaaSamples);
        VkShaderModule CreateShaderModule(const std::vector<char>& code);
        VkDescriptorPool CreateDescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets = 100);
        VkDescriptorSetLayout CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& pBindings, const VkDescriptorSetLayoutCreateFlagBits& flags);
        VkDescriptorSet CreateDescriptorSets(const VkDescriptorPool& descriptorPool, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, uint32_t count = 100);
        
        void UpdateDescriptorSets(const std::vector<Buffer>& uniformBuffers, const VkDescriptorSet& descriptorSet, const std::vector<VkDescriptorImageInfo>& imageInfo, VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        
        VkPipelineLayout CreatePipelineLayout(const std::vector<VkDescriptorSet>& descriptorSets, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, const std::vector<VkPushConstantRange>& pushConstants);
        VkPipeline CreateGraphicsPipeline(
            std::shared_ptr<VkRenderPass> renderPass,
            VkPipelineLayout pipelineLayout,
            std::string_view name,
            std::vector<VkPipelineShaderStageCreateInfo>shadersCreateInfos,
            VkPipelineVertexInputStateCreateInfo vertexInputInfo,
            VkCullModeFlagBits cullMode = VK_CULL_MODE_BACK_BIT,
            bool dynamicRendering = true,
            bool depthTestEnable = true,
            bool depthWriteEnable = true,
            bool stencilTestEnable = true,
            int polygoneMode = VK_POLYGON_MODE_FILL
        );
        VkSwapchainKHR CreateSwapChain(std::vector<VkImage>& swapChainImages, const VkSwapchainKHR& oldSwapChain = VK_NULL_HANDLE);
        std::vector<VkFramebuffer> CreateFramebuffers(std::shared_ptr<VkRenderPass> renderPass, std::vector<VkImageView> swapChainImageViews, std::vector<VkImageView> depthImageView, std::vector<VkImageView> colorImageView);
        VkCommandPool CreateCommandPool();
        std::vector<VkCommandBuffer> AllocateCommandBuffers(VkCommandPool commandPool, uint32_t size = 1, bool isSecondary = false);
        VkBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        Buffer CreateVertexBuffer(VkCommandPool commandPool, std::vector<Rbk::Vertex> vertices);
        Buffer CreateVertex2DBuffer(const VkCommandPool& commandPool, const std::vector<Rbk::Vertex2D>& vertices);
        Buffer CreateIndexBuffer(const VkCommandPool& commandPool, const std::vector<uint32_t>& indices);
        std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> CreateSyncObjects(std::vector<VkImage> swapChainImages);
        VkImageMemoryBarrier SetupImageMemoryBarrier(VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = VK_REMAINING_MIP_LEVELS, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);
        void CopyBuffer(VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, int queueIndex = 0);
        bool SouldResizeSwapChain(VkSwapchainKHR swapChain);
        Buffer CreateUniformBuffers(uint32_t uniformBuffersCount);
        Buffer CreateCubeUniformBuffers(uint32_t uniformBuffersCount);
        void UpdateUniformBuffer(Buffer& buffer, std::vector<UniformBufferObject> uniformBufferObjects, uint32_t uniformBuffersCount);
        void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image);
        void CreateSkyboxImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image);
        VkImageView CreateImageView(VkImage image, VkFormat format, uint32_t mipLevels, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
        VkImageView CreateSkyboxImageView(VkImage image, VkFormat format, uint32_t mipLevels, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
        void CreateTextureImage(VkCommandBuffer& commandBuffer, stbi_uc* pixels, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels, VkImage& textureImage, VkFormat format);
        void CreateSkyboxTextureImage(VkCommandBuffer& commandBuffer, std::vector<stbi_uc*>& skyboxPixels, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels, VkImage& textureImage, VkFormat format);
        void CopyBufferToImage(VkCommandBuffer& commandBuffer, VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height, uint32_t mipLevels,  uint32_t offset);
        void CopyBufferToImageSkybox(VkCommandBuffer& commandBuffer, VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height, std::vector<stbi_uc*>skyboxPixels, uint32_t mipLevels, uint32_t layerSize, uint32_t offset);
        VkSampler CreateTextureSampler(uint32_t mipLevels);
        VkSampler CreateSkyboxTextureSampler(uint32_t mipLevels);
        VkImageView CreateDepthResources(VkCommandBuffer commandBuffer);
        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat FindDepthFormat();
        bool HasStencilComponent(VkFormat format);
        VkDeviceSize GetMaxMemoryHeap() { return m_MaxMemoryHeap; }
        void InitMemoryPool();
        Buffer CreateStorageBuffers(uint32_t uniformBuffersCount);
        void UpdateStorageBuffer(Buffer buffer, std::vector<UniformBufferObject> bufferObjects);

        /**
        * Vulkan drawing functions, in main loop
        **/
        void ResetCommandPool(VkCommandPool commandPool);
        void BeginCommandBuffer(VkCommandBuffer commandBuffer,
            VkCommandBufferUsageFlagBits flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
            VkCommandBufferInheritanceInfo inheritanceInfo = {});
        void EndCommandBuffer(VkCommandBuffer commandBuffer);
        void BeginRenderPass(std::shared_ptr<VkRenderPass> renderPass, VkCommandBuffer commandBuffer, VkFramebuffer swapChainFramebuffer);
        void EndRenderPass(VkCommandBuffer commandBuffer);
        void BeginRendering(const VkCommandBuffer& commandBuffer, const VkImageView& colorImageView, const VkImageView& depthImageView, const VkAttachmentLoadOp loadOp, const VkAttachmentStoreOp storeOp);
        void EndRendering(VkCommandBuffer commandBuffer);
        void SetViewPort(VkCommandBuffer commandBuffer);
        void SetScissor(VkCommandBuffer commandBuffer);
        void BindPipeline(const VkCommandBuffer& commandBuffer, const VkPipeline& pipeline);
        void Draw(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, Mesh* mesh, Data data, uint32_t uboCount, uint32_t frameIndex, bool drawIndexed = true, uint32_t index = 0);
        uint32_t AcquireNextImageKHR(VkSwapchainKHR swapChain, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>& semaphores);
        void QueueSubmit(VkCommandBuffer commandBuffer, int queueIndex = 0);
        void QueueSubmit(uint32_t imageIndex, std::vector<VkCommandBuffer> commandBuffers, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>& semaphores, int queueIndex = 0);
        void QueuePresent(uint32_t imageIndex, VkSwapchainKHR swapChain, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>& semaphores, int queueIndex = 0);
        void AddPipelineBarriers(VkCommandBuffer commandBuffer, std::vector<VkImageMemoryBarrier> renderBarriers, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags);
        void GenerateMipmaps(VkCommandBuffer commandBuffer, VkFormat imageFormat, VkImage image, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels, uint32_t layerCount = 1);
        uint32_t GetNextFrameIndex();

        /**
        * Vulkan clean and destroy
        **/
        void DestroyPipeline(VkPipeline pipeline);
        void DestroyPipelineData(VkPipelineLayout pipelineLayout, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);
        void DestroySwapchain(VkDevice device, VkSwapchainKHR swapChain, std::vector<VkFramebuffer> swapChainFramebuffers, std::vector<VkImageView> swapChainImageViews);
        void DestroySemaphores(std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores);
        void DestroyFences();
        void DestroyBuffer(VkBuffer buffer);
        void DestroyRenderPass(std::shared_ptr<VkRenderPass> renderPass, VkCommandPool commandPool, std::vector<VkCommandBuffer> commandBuffers);
        void Destroy();

        /*
        * Helper functions.
        */
        inline const std::vector<const char*> GetValidationLayers() const { return m_ValidationLayers; }
        inline const std::vector<VkExtensionProperties> GetExtensions() const { return m_Extensions; }
        inline const std::vector<VkLayerProperties> GetLayersAvailable() const { return m_LayersAvailable; }
        inline bool IsInstanceCreated() const { return m_InstanceCreated; }
        inline bool IsValidationLayersEnabled() const { return m_EnableValidationLayers; }
        inline uint32_t GetExtensionCount() const { return m_ExtensionCount; }
        inline uint32_t GetQueueFamily() const { return m_QueueFamilyIndices.graphicsFamily.value(); }
        inline VkInstance GetInstance() const { return m_Instance; };
        inline VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
        inline VkDevice GetDevice() const { return m_Device; }
        inline std::vector<VkQueue> GetGraphicsQueues() const { return m_GraphicsQueues; }
        inline VkPhysicalDeviceProperties GetDeviceProperties() const { return m_DeviceProps; }
        inline VkPhysicalDeviceFeatures GetDeviceFeatures() const { return m_DeviceFeatures; }
        inline bool IsFramebufferResized() { return m_FramebufferResized; }
        inline VkExtent2D GetSwapChainExtent() const { return m_SwapChainExtent; }
        inline VkSurfaceKHR GetSurface() const { return m_Surface; }
        inline void ResetCurrentFrameIndex() { m_CurrentFrame = 0; }
        inline int32_t GetCurrentFrame() const { return m_CurrentFrame; }
        inline VkFormat GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }
        inline VkSampleCountFlagBits GetMsaaSamples() const { return m_MsaaSamples; }
        void InitDetails();

        VkSampleCountFlagBits GetMaxUsableSampleCount();
        const SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;
        uint32_t GetImageCount() const;
        std::string GetAPIVersion();
        std::shared_ptr<DeviceMemoryPool> GetDeviceMemoryPool() { return m_DeviceMemoryPool; }
        void StartMarker(VkCommandBuffer buffer, const std::string& name, float r, float g, float b, float a = 1.0);
        void EndMarker(VkCommandBuffer buffer);
        uint32_t GetQueueCount() { return m_queueCount; }

        static const std::string GetVendor(int vendorID)
        {
            std::map<int, std::string> vendors;
            vendors[0x1002] = "AMD";
            vendors[0x1010] = "ImgTec";
            vendors[0x10DE] = "NVIDIA";
            vendors[0x13B5] = "ARM";
            vendors[0x5143] = "Qualcomm";
            vendors[0x8086] = "INTEL";

            return vendors[vendorID];
        }

    public:
        bool m_FramebufferResized = false;
    
    private:
        bool IsDeviceSuitable(VkPhysicalDevice device);
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        bool CheckValidationLayerSupport();
        void CreateInstance();
        void EnumerateExtensions();
        void LoadRequiredExtensions();
        void SetupDebugMessenger();
        void CreateLogicalDevice();
        void PickPhysicalDevice();
        void CreateSurface();

        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    private:
        const int m_MAX_FRAMES_IN_FLIGHT = 2;
        int32_t m_CurrentFrame = 0;
        uint32_t m_ExtensionCount;
        std::string m_apiVersion;
        const uint32_t m_queueCount = 2;

        std::shared_ptr<Window> m_Window = VK_NULL_HANDLE;

        const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> m_DeviceExtensions = {
            VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
            VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
            VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME
        };

        bool m_InstanceCreated = false;
        bool m_EnableValidationLayers = false;
    
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties m_DeviceProps;
        VkPhysicalDeviceFeatures m_DeviceFeatures;
        VkPhysicalDeviceProperties2 m_DeviceProperties2;
        VkPhysicalDeviceMaintenance3Properties m_DeviceMaintenance3Properties;

        VkDevice m_Device = VK_NULL_HANDLE;
        std::vector<VkQueue> m_GraphicsQueues;
        std::vector<VkQueue> m_PresentQueues;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        VkFormat m_SwapChainImageFormat;
        VkExtent2D m_SwapChainExtent;
        VkDebugUtilsMessengerEXT m_DebugMessengerCallback = VK_NULL_HANDLE;
        QueueFamilyIndices m_QueueFamilyIndices = {};
        VkSurfaceFormatKHR m_SurfaceFormat;
        VkPresentModeKHR m_PresentMode;
        SwapChainSupportDetails m_SwapChainSupport;

        std::vector<VkLayerProperties> m_LayersAvailable = {};
        std::vector<VkExtensionProperties> m_Extensions = {};
        std::vector<const char*> m_RequiredExtensions = {};
        std::vector<VkFence> m_InFlightFences = {};
        std::vector<VkFence> m_ImagesInFlight = {};
        VkFence m_Fence;

        VkSampleCountFlagBits m_MsaaSamples = VK_SAMPLE_COUNT_8_BIT;

        std::mutex m_MutexQueueSubmit;
        std::mutex m_MutexDraw;
        std::mutex m_MutexCmdBuffer;
        std::mutex m_MutexAcquireNextImage;
        std::mutex m_MutexGraphicsPipeline;

        VkDeviceSize m_MaxMemoryHeap;
        std::shared_ptr<DeviceMemoryPool> m_DeviceMemoryPool = nullptr;
    };
}