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

	private:
		void CreateInstance(uint32_t extensionCount, const char* const* extensions);
		void EnumerateExtensions();

	private:
		VkInstance m_Instance = VK_NULL_HANDLE;
		uint32_t m_ExtensionCount = 0;
		std::vector<VkExtensionProperties> m_Extensions;
	};
}
