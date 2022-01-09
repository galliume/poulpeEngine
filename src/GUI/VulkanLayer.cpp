#include "VulkanLayer.h"

namespace Rebulk
{
	VulkanLayer::VulkanLayer(GLFWwindow* window, VulkanRenderer* vulkanRenderer)
		: m_Window(window), m_VulkanRenderer(vulkanRenderer)
	{
	}
	
	void VulkanLayer::Create()
	{
		std::vector<VkExtensionProperties>extensions = m_VulkanRenderer->GetExtensions();
		std::string extensionsCount = "Extensions count " + std::to_string(m_VulkanRenderer->GetExtensionCount());

		Rebulk::Im::NewFrame();
		Rebulk::Im::Begin("Vulkan Infos");
		Rebulk::Im::Text(extensionsCount.c_str());

		for (const auto& extension : extensions) {
			Rebulk::Im::Text(extension.extensionName);
		}
		Rebulk::Im::End();
	}

	void VulkanLayer::Render()
	{
		Rebulk::Im::Render(m_Window);
	}

	void VulkanLayer::Destroy()
	{
		Rebulk::Im::Destroy();
	}
}