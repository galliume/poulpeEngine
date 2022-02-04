#include "Rebulk/Pattern/ISubject.h"
#include "Rebulk/Renderer/Mesh.h"

namespace Rbk {

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

	class VulkanRenderer : public ISubject
	{
	public:
		VulkanRenderer(GLFWwindow* window);
		~VulkanRenderer();		

		/**
		* Vulkan init functions, before main loop.
		**/
		VkRenderPass CreateRenderPass();
		VkShaderModule CreateShaderModule(const std::vector<char>& code);
		VkDescriptorSetLayout CreateDescriptorSetLayout();
		VkPipelineLayout CreatePipelineLayout(VkDescriptorSetLayout descriptorSetLayout);
		VkPipeline CreateGraphicsPipeline(VkRenderPass renderPass, VkPipelineLayout pipelineLayout, VkPipelineCache pipelineCache, VkShaderModule vs, VkShaderModule fs);
		VkSwapchainKHR CreateSwapChain(std::vector<VkImage>& swapChainImages, VkSwapchainKHR oldSwapChain = VK_NULL_HANDLE);
		std::vector<VkImageView> CreateImageViews(std::vector<VkImage> swapChainImages);
		std::vector<VkFramebuffer> CreateFramebuffers(VkRenderPass renderPass, std::vector<VkImageView> swapChainImageViews);
		VkCommandPool CreateCommandPool();
		std::vector<VkCommandBuffer> AllocateCommandBuffers(VkCommandPool commandPool, uint32_t size = 1);
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		std::pair<VkBuffer, VkDeviceMemory> CreateVertexBuffer(VkCommandPool commandPool, std::vector<Rbk::Vertex> vertices);
		std::pair<VkBuffer, VkDeviceMemory> CreateIndexBuffer(VkCommandPool commandPool, std::vector<uint32_t> indices);
		VkDescriptorPool CreateDescriptorPool(std::vector<VkImage> swapChainImages);
		std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> CreateSyncObjects(std::vector<VkImage> swapChainImages);
		VkImageMemoryBarrier SetupImageMemoryBarrier(VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBuffer(VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		bool SouldResizeSwapChain(VkSwapchainKHR swapChain);
		std::pair<std::vector<VkBuffer>, std::vector<VkDeviceMemory>> CreateUniformBuffers(std::vector<VkImageView> swapChainImages);
		void UpdateUniformBuffer(VkDeviceMemory uniformBufferMemory);
		std::vector<VkDescriptorSet> CreateDescriptorSets(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, std::vector<VkImage> swapChainImages, std::pair<std::vector<VkBuffer>, std::vector<VkDeviceMemory>> uniformBuffers);

		/**
		* Vulkan drawing functions, in main loop
		**/
		void ResetCommandPool(VkCommandPool commandPool);
		void BeginCommandBuffer(VkCommandBuffer commandBuffer);
		void BeginRenderPass(VkRenderPass renderPass, VkCommandBuffer commandBuffer, VkFramebuffer swapChainFramebuffer);
		void SetViewPort(VkCommandBuffer commandBuffer);
		void SetScissor(VkCommandBuffer commandBuffer);
		void BindPipeline(VkCommandBuffer commandBuffer, VkPipeline pipeline);
		void Draw(VkCommandBuffer commandBuffer, VkBuffer vertexBuffer, VkBuffer indexBuffer, std::vector<uint32_t> indices,
			VkBuffer uniformBuffer, VkDescriptorSet descriptorSet, VkPipelineLayout pipelineLayout
		);
		void EndRenderPass(VkCommandBuffer commandBuffer);
		void EndCommandBuffer(VkCommandBuffer commandBuffer);
		uint32_t AcquireNextImageKHR(VkSwapchainKHR swapChain, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>& semaphores);
		void QueueSubmit(uint32_t imageIndex, VkCommandBuffer commandBuffer, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>& semaphores);
		size_t QueuePresent(uint32_t imageIndex, VkSwapchainKHR swapChain, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>& semaphores);
		void AddPipelineBarrier(VkCommandBuffer commandBuffer, VkImageMemoryBarrier renderBarrier, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags);
		void WaitIdle();

		/**
		* Vulkan clean and destroy
		**/
		void DestroyPipeline(VkPipeline pipeline, VkPipelineLayout pipelineLayout, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);
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

		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

		void Attach(IObserver* observer) override;
		void Detach(IObserver* observer) override;
		void Notify() override;
	
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
		const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

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

		std::list<IObserver*> m_Observers = {};
		std::vector<std::string> m_Messages = {};
		std::vector<VkLayerProperties> m_LayersAvailable = {};		
		std::vector<VkExtensionProperties> m_Extensions = {};
		std::vector<const char*> m_RequiredExtensions = {};		
		std::vector<VkFence> m_InFlightFences = {};
		std::vector<VkFence> m_ImagesInFlight = {};
	};
}