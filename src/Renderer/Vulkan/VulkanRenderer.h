#include <algorithm>
#include <optional>
#include <set>
#include <cstdint> 

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
		inline bool IsInstanceCreated() { return m_InstanceCreated; };

		inline uint32_t GetExtensionCount() { return m_ExtensionCount; };
		inline std::vector<VkExtensionProperties> GetExtensions() { return m_Extensions; };

		inline std::vector<VkLayerProperties> GetLayersAvailable() { return m_LayersAvailable; };
		inline const std::vector<const char*> GetValidationLayers() { return m_ValidationLayers; };
		inline bool IsValidationLayersEnabled() { return m_EnableValidationLayers; };

		void PickPhysicalDevice();
		inline VkPhysicalDeviceProperties GetDeviceProperties() { return m_DeviceProps; };
		inline VkPhysicalDeviceFeatures GetDeviceFeatures() { return m_DeviceFeatures; };

		void CreateSurface();
		void CreateSwapChain();

		void Attach(IObserver* observer) override;
		void Detach(IObserver* observer) override;
		void Notify() override;
	
	private:
		void CreateInstance();
		void EnumerateExtensions();
		bool CheckValidationLayerSupport();
		void LoadRequiredExtensions();
		void SetupDebugMessenger();
		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		void CreateLogicalDevice();
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	private:
		std::list<IObserver*> m_Observers = {};
		std::vector<std::string> m_Messages = {};

		bool m_InstanceCreated = false;
		VkInstance m_Instance = VK_NULL_HANDLE;
		
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties m_DeviceProps = {};
		VkPhysicalDeviceFeatures m_DeviceFeatures = {};
		
		VkDevice m_Device = VK_NULL_HANDLE;
		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;

		GLFWwindow* m_Window = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;

		bool m_EnableValidationLayers = false;
		const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
		std::vector<VkLayerProperties> m_LayersAvailable = {};
		
		const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		std::vector<VkExtensionProperties> m_Extensions = {};
		std::vector<const char*> m_RequiredExtensions = {};
		uint32_t m_ExtensionCount = 0;

		VkSwapchainKHR m_SwapChain;
		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;

		VkDebugUtilsMessengerEXT m_DebugMessengerCallback = VK_NULL_HANDLE;
	};
}
