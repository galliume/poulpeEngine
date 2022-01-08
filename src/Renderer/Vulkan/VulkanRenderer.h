#include "vulkan/vulkan.h"

namespace Rebulk {

	class VulkanRenderer
	{
		public:
			VulkanRenderer(uint32_t extensionCount, const char* const* extensions);
			~VulkanRenderer();
		private:
			VkInstance m_Instance = VK_NULL_HANDLE;
	};
}
