#include "Rebulk/Renderer/Mesh.h"
#include "Rebulk/Renderer/IRenderer.h"

namespace Rbk {

	struct VulkanShaders
	{
		std::map<std::string, std::array<VkShaderModule, 2>> shaders;
	};

	struct VulkanTexture
	{
		const char* name;
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler sampler;
		uint32_t mipLevels;
		uint32_t texWidth;
		uint32_t texHeight;
		uint32_t texChannels;
		VkImage colorImage;
		VkDeviceMemory colorImageMemory;
		VkImageView colorImageView;
		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		VkImageView depthImageView;
	};

	struct VulkanMesh
	{
		Rbk::Mesh mesh;
		uint32_t vertexIndicesCount = 0;
		std::pair<VkBuffer, VkDeviceMemory> meshVBuffer = { nullptr, nullptr };
		std::pair<VkBuffer, VkDeviceMemory> meshIBuffer = { nullptr, nullptr };
		std::vector<std::pair<VkBuffer, VkDeviceMemory>>uniformBuffers;
		int32_t count = 0;
		int32_t totalInstances = 0;
		int32_t uniformBuffersCount = 0;
		int32_t uniformBufferChunkSize = 0;
	};

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
		VulkanRenderer(GLFWwindow* window);
		~VulkanRenderer();		

		/**
		* Vulkan init functions, before main loop.
		**/
		VkRenderPass CreateRenderPass(VkSampleCountFlagBits msaaSamples);
		VkShaderModule CreateShaderModule(const std::vector<char>& code);
		VkDescriptorPool CreateDescriptorPool(std::vector<VkImage> swapChainImages);
		VkDescriptorSetLayout CreateDescriptorSetLayout(uint32_t meshCount);
		VkDescriptorSet CreateDescriptorSets(VkDescriptorPool descriptorPool, std::vector<VkImage> swapChainImages, std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
		void UpdateDescriptorSets(VulkanMesh vMesh, std::map<const char*, VulkanTexture> vTextures, VulkanPipeline pipeline);
		VkPipelineLayout CreatePipelineLayout(std::vector<VkDescriptorSet> descriptorSets, std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
		VkPipeline CreateGraphicsPipeline(VkRenderPass renderPass, VulkanPipeline pipeline, VulkanShaders shaders, bool wireFrameModeOn = false);
		VkSwapchainKHR CreateSwapChain(std::vector<VkImage>& swapChainImages, VkSwapchainKHR oldSwapChain = VK_NULL_HANDLE);
		std::vector<VkFramebuffer> CreateFramebuffers(VkRenderPass renderPass, std::vector<VkImageView> swapChainImageViews, std::vector<VkImageView> depthImageView, std::vector<VkImageView> colorImageView);
		VkCommandPool CreateCommandPool();
		std::vector<VkCommandBuffer> AllocateCommandBuffers(VkCommandPool commandPool, uint32_t size = 1);
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		std::pair<VkBuffer, VkDeviceMemory> CreateVertexBuffer(VkCommandPool commandPool, std::vector<Rbk::Vertex> vertices);
		std::pair<VkBuffer, VkDeviceMemory> CreateIndexBuffer(VkCommandPool commandPool, std::vector<uint32_t> indices);
		std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> CreateSyncObjects(std::vector<VkImage> swapChainImages);
		VkImageMemoryBarrier SetupImageMemoryBarrier(VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = VK_REMAINING_MIP_LEVELS, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);
		void CopyBuffer(VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		bool SouldResizeSwapChain(VkSwapchainKHR swapChain);
		std::pair<VkBuffer, VkDeviceMemory> CreateUniformBuffers(uint32_t uniformBuffersCount);
		void UpdateUniformBuffer(std::pair<VkBuffer, VkDeviceMemory>uniformBuffer, std::vector<UniformBufferObject> uniformBufferObjects, uint32_t uniformBuffersCount);
		void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		VkImageView CreateImageView(VkImage image, VkFormat format, uint32_t mipLevels, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
		void CreateTextureImage(VkCommandBuffer commandBuffer, stbi_uc* pixels, int texWidth, int texHeight, uint32_t mipLevels, VkImage& textureImage, VkDeviceMemory& textureImageMemory, VkFormat format);
		void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		VkSampler CreateTextureSampler(uint32_t mipLevels);
		VkImageView CreateDepthResources(VkCommandBuffer commandBuffer);
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat FindDepthFormat();
		bool HasStencilComponent(VkFormat format);

		/**
		* Vulkan drawing functions, in main loop
		**/
		void ResetCommandPool(VkCommandPool commandPool);
		void BeginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlagBits flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT);
		void EndCommandBuffer(VkCommandBuffer commandBuffer);
		void BeginRenderPass(VkRenderPass renderPass, VkCommandBuffer commandBuffer, VkFramebuffer swapChainFramebuffer);
		void EndRenderPass(VkCommandBuffer commandBuffer);
		void SetViewPort(VkCommandBuffer commandBuffer);
		void SetScissor(VkCommandBuffer commandBuffer);
		void BindPipeline(VkCommandBuffer commandBuffer, VkPipeline pipeline);
		void Draw(VkCommandBuffer commandBuffer, VulkanMesh vMesh, VulkanPipeline pipeline);
		uint32_t AcquireNextImageKHR(VkSwapchainKHR swapChain, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>& semaphores);
		void QueueSubmit(VkCommandBuffer commandBuffer);
		void QueueSubmit(uint32_t imageIndex, VkCommandBuffer commandBuffer, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>& semaphores);
		size_t QueuePresent(uint32_t imageIndex, VkSwapchainKHR swapChain, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>& semaphores);
		void AddPipelineBarrier(VkCommandBuffer commandBuffer, VkImageMemoryBarrier renderBarrier, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags);
		void WaitIdle();
		void GenerateMipmaps(VkCommandBuffer commandBuffer, VkFormat imageFormat, VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

		/**
		* Vulkan clean and destroy
		**/
		void DestroyPipeline(VkPipeline pipeline);
		void DestroyPipelineData(VkPipelineLayout pipelineLayout, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);
		void DestroySwapchain(VkDevice device, VkSwapchainKHR swapChain, std::vector<VkFramebuffer> swapChainFramebuffers, std::vector<VkImageView> swapChainImageViews);
		void DestroySemaphores(std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores);
		void DestroyBuffer(VkBuffer buffer);
		void DestroyDeviceMemory(VkDeviceMemory deviceMemory);
		void DestroyRenderPass(VkRenderPass renderPass, VkCommandPool commandPool, std::vector<VkCommandBuffer> commandBuffers);
		void Destroy();

		/*
		* Helper functions.
		*/
		inline const std::vector<const char*> GetValidationLayers() { return m_ValidationLayers; };
		inline std::vector<VkExtensionProperties> GetExtensions() { return m_Extensions; };
		inline std::vector<VkLayerProperties> GetLayersAvailable() { return m_LayersAvailable; };
		inline bool IsInstanceCreated() { return m_InstanceCreated; };
		inline bool IsValidationLayersEnabled() { return m_EnableValidationLayers; };
		inline uint32_t GetExtensionCount() { return m_ExtensionCount; };
		inline uint32_t GetQueueFamily() { return m_QueueFamilyIndices.graphicsFamily.value(); };
		inline VkInstance GetInstance() { return m_Instance; };
		inline VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; };
		inline VkDevice GetDevice() { return m_Device; };
		inline VkQueue GetGraphicsQueue() { return m_GraphicsQueue; };
		inline VkPhysicalDeviceProperties GetDeviceProperties() { return m_DeviceProps; };
		inline VkPhysicalDeviceFeatures GetDeviceFeatures() { return m_DeviceFeatures; };
		inline bool IsFramebufferResized() { return m_FramebufferResized; };
		inline VkExtent2D GetSwapChainExtent() { return m_SwapChainExtent; };
		inline VkSurfaceKHR GetSurface() { return m_Surface; };
		inline void ResetCurrentFrameIndex() { m_CurrentFrame = 0; };
		inline uint32_t GetCurrentFrame() { return m_CurrentFrame; };
		inline VkFormat GetSwapChainImageFormat() { return m_SwapChainImageFormat; };
		inline VkSampleCountFlagBits GetMsaaSamples() { return m_MsaaSamples; };
		void InitDetails();
		void CreateFence();
		void WaitForFence();
		VkSampleCountFlagBits GetMaxUsableSampleCount();
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

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
		size_t m_CurrentFrame = 0;
		uint32_t m_ExtensionCount = 0;

		GLFWwindow* m_Window = VK_NULL_HANDLE;

		const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME };

		bool m_InstanceCreated = false;
		bool m_EnableValidationLayers = false;
	
		VkInstance m_Instance = VK_NULL_HANDLE;		
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties m_DeviceProps = {};
		VkPhysicalDeviceFeatures m_DeviceFeatures = {};		
		VkDevice m_Device = VK_NULL_HANDLE;
		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;
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

		VkSampleCountFlagBits m_MsaaSamples = VK_SAMPLE_COUNT_1_BIT;
	};
}
