#include <algorithm>
#include <optional>
#include <set>
#include <cstdint> 
#include <fstream>

#include "rebulkpch.h"
#include "vulkan/vulkan.h"
#include "Pattern/ISubject.h"

namespace Rebulk {

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
		
		void Init();
		bool DrawFrame(VkSwapchainKHR swapChain, std::vector<VkCommandBuffer> commandBuffers, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores);
		void CleanupSwapChain(
			VkSwapchainKHR swapChain, VkRenderPass renderPass, VkCommandPool commandPool, std::pair<VkPipeline, VkPipelineLayout>pipeline,
			std::vector<VkImageView> swapChainImageViews, std::vector<VkCommandBuffer> commandBuffers, std::vector<VkFramebuffer> swapChainFramebuffers,
			VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout
		);

		void RecreateSwapChain();
		void Destroy(VkCommandPool commandPool, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores);

		VkRenderPass CreateRenderPass();
		void BeginRenderPass(VkRenderPass renderPass, std::vector<VkCommandBuffer> commandBuffers, std::vector<VkFramebuffer> swapChainFramebuffers, std::pair<VkPipeline, VkPipelineLayout>pipeline);
		void EndRenderPass(std::vector<VkCommandBuffer> commandBuffers);

		VkCommandBuffer CreateCommandBuffer(VkCommandPool commandPool);
		VkSwapchainKHR CreateSwapChain();
		std::vector<VkImageView> CreateImageViews();
		std::pair<VkPipeline, VkPipelineLayout> CreateGraphicsPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
		std::vector<VkFramebuffer> CreateFramebuffers(VkRenderPass renderPass, std::vector<VkImageView> swapChainImageViews);
		VkCommandPool CreateCommandPool();
		VkDescriptorPool CreateDescriptorPool();
		VkDescriptorSetLayout CreateDescriptorSetLayout();
		std::vector<VkDescriptorSet> CreateDescriptorSets(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, std::vector<VkImageView> imageViews);
		std::vector<VkCommandBuffer> CreateCommandBuffers(VkRenderPass renderPass, VkCommandPool commandPool, std::pair<VkPipeline, VkPipelineLayout>pipeline, std::vector<VkFramebuffer> swapChainFramebuffers);
		std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> CreateSyncObjects();
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		inline const std::vector<const char*> GetValidationLayers() { return m_ValidationLayers; };
		inline std::vector<VkExtensionProperties> GetExtensions() { return m_Extensions; };
		inline std::vector<VkLayerProperties> GetLayersAvailable() { return m_LayersAvailable; };
		inline bool IsInstanceCreated() { return m_InstanceCreated; };
		inline bool IsValidationLayersEnabled() { return m_EnableValidationLayers; };
		inline uint32_t GetExtensionCount() { return m_ExtensionCount; };
		inline uint32_t GetQueueFamily() { return m_QueueFamilyIndices.graphicsFamily.value(); };
		inline VkInstance GetInstance() { return m_Instance; };
		inline VkRenderPass GetRenderPass() { return m_RenderPass; };
		inline VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; };
		inline VkDevice GetDevice() { return m_Device; };
		inline VkQueue GetGraphicsQueue() { return m_GraphicsQueue; };
		inline VkDescriptorPool GetDescriptorPool() { return m_DescriptorPool; };
		inline VkPhysicalDeviceProperties GetDeviceProperties() { return m_DeviceProps; };
		inline VkPhysicalDeviceFeatures GetDeviceFeatures() { return m_DeviceFeatures; };

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
		VkShaderModule CreateShaderModule(const std::vector<char>& code);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	private:
		const int m_MAX_FRAMES_IN_FLIGHT = 2;
		size_t m_CurrentFrame = 0;
		size_t m_SingleTimeCurrentFrame = 0;
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
		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;		
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkSemaphore m_ImageAvailableSemaphore = VK_NULL_HANDLE;
		VkSemaphore m_RenderFinishedSemaphore = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessengerCallback = VK_NULL_HANDLE;
		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		QueueFamilyIndices m_QueueFamilyIndices = {};
		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;

		std::list<IObserver*> m_Observers = {};
		std::vector<std::string> m_Messages = {};
		std::vector<VkLayerProperties> m_LayersAvailable = {};		
		std::vector<VkExtensionProperties> m_Extensions = {};
		std::vector<const char*> m_RequiredExtensions = {};
		std::vector<VkImage> m_SwapChainImages = {};
		std::vector<VkImageView> m_SwapChainImageViews = {};
		std::vector<VkFramebuffer> m_SwapChainFramebuffers = {};
		std::vector<VkCommandBuffer> m_CommandBuffers = {};
		std::vector<VkSemaphore> m_ImageAvailableSemaphores = {};
		std::vector<VkSemaphore> m_RenderFinishedSemaphores = {};
		std::vector<VkFence> m_InFlightFences = {};
		std::vector<VkFence> m_ImagesInFlight = {};
		std::vector<VkFence>m_SingleTimeInFlightFences = {};
	};
}
