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
		void DrawFrame();
		void Destroy();

		inline const std::vector<const char*> GetValidationLayers() { return m_ValidationLayers; };
		inline uint32_t GetExtensionCount() { return m_ExtensionCount; };
		inline bool IsInstanceCreated() { return m_InstanceCreated; };
		inline bool IsValidationLayersEnabled() { return m_EnableValidationLayers; };
		inline std::vector<VkExtensionProperties> GetExtensions() { return m_Extensions; };
		inline std::vector<VkLayerProperties> GetLayersAvailable() { return m_LayersAvailable; };
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
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateSyncObjects();
		void RecreateSwapChain();
		void CleanupSwapChain();

		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkShaderModule CreateShaderModule(const std::vector<char>& code);

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
		VkSwapchainKHR m_SwapChain;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;		
		VkRenderPass m_RenderPass;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;
		VkSemaphore m_ImageAvailableSemaphore;
		VkSemaphore m_RenderFinishedSemaphore;
		VkDebugUtilsMessengerEXT m_DebugMessengerCallback = VK_NULL_HANDLE;
		VkCommandPool m_CommandPool;

		std::list<IObserver*> m_Observers = {};
		std::vector<std::string> m_Messages = {};
		std::vector<VkLayerProperties> m_LayersAvailable = {};		
		std::vector<VkExtensionProperties> m_Extensions = {};
		std::vector<const char*> m_RequiredExtensions = {};
		std::vector<VkImage> m_SwapChainImages;
		std::vector<VkImageView> m_SwapChainImageViews;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;
		std::vector<VkCommandBuffer> m_CommandBuffers;
		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;
		std::vector<VkFence> m_ImagesInFlight;
	};
}
