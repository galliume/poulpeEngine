#include <algorithm>
#include "rebulkpch.h"
#include "vulkan/vulkan.h"
#include "Pattern\ISubject.h"

namespace Rebulk {

	class VulkanRenderer : public ISubject
	{
	public:
		VulkanRenderer();
		~VulkanRenderer();
		void Init();
		inline uint32_t GetExtensionCount() { return m_ExtensionCount; };
		inline std::vector<VkExtensionProperties> GetExtensions() { return m_Extensions; };
		inline std::vector<VkLayerProperties> GetLayersAvailable() { return m_LayersAvailable; };
		inline const std::vector<const char*> GetValidationLayers() { return m_ValidationLayers; };
		inline bool IsValidationLayersEnabled() { return m_EnableValidationLayers; };
		inline bool IsInstanceCreated() { return m_InstanceCreated; };
		VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData
		);
		void Attach(IObserver* observer) override;
		void Detach(IObserver* observer) override;
		void Notify() override;
	private:
		void CreateInstance();
		void EnumerateExtensions();
		bool CheckValidationLayerSupport();
		void LoadRequiredExtensions();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void SetupDebugMessenger();

	private:
		std::list<IObserver*> m_Observers;
		std::string m_Title;
		std::vector<std::string> m_Messages;
		const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VkInstance m_Instance = VK_NULL_HANDLE;
		uint32_t m_ExtensionCount = 0;
		std::vector<VkExtensionProperties> m_Extensions = {};
		std::vector<VkLayerProperties> m_LayersAvailable = {};
		bool m_EnableValidationLayers = false;
		bool m_InstanceCreated = false;
		std::vector<const char*> m_RequiredExtensions = {};
		VkDebugUtilsMessengerEXT callback = VK_NULL_HANDLE;
	};
}
