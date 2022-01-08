#include "vulkan/vulkan.h"

namespace Rebulk {

	class VulkanRenderer
	{
	public:
		VulkanRenderer(uint32_t extensionCount, const char* const* extensions);
		~VulkanRenderer();

	private:
		void CreateInstance(uint32_t extensionCount, const char* const* extensions);
		void EnumerateExtensions();

	private:
		VkInstance m_Instance = VK_NULL_HANDLE;
	};
}
