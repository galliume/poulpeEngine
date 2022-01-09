#include "rebulkpch.h"
#include "vulkan/vulkan.h"

namespace Rebulk {

	class VulkanRenderer
	{
	public:
		VulkanRenderer(uint32_t extensionCount, const char* const* extensions);
		~VulkanRenderer();
		inline uint32_t GetExtensionCount() { return m_ExtensionCount; };
		inline std::vector<VkExtensionProperties> GetExtensions() { return m_Extensions; };
		inline const std::vector<const char*> GetValidationLayers() { return m_ValidationLayers; };

	private:
		void CreateInstance(uint32_t extensionCount, const char* const* extensions);
		void EnumerateExtensions();
		bool CheckValidationLayerSupport();

	private:
		const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VkInstance m_Instance = VK_NULL_HANDLE;
		uint32_t m_ExtensionCount = 0;
		std::vector<VkExtensionProperties> m_Extensions;
	};
}
