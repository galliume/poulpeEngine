#include "VulkanLayer.h"
#include <inttypes.h>

namespace Rebulk
{
	VulkanLayer::VulkanLayer(GLFWwindow* window, VulkanRenderer* vulkanRenderer)
		: m_Window(window), m_VulkanRenderer(vulkanRenderer)
	{
	}
	
	void VulkanLayer::Create()
	{
		std::vector<VkExtensionProperties>extensions = m_VulkanRenderer->GetExtensions();

		Rebulk::Im::NewFrame();
		Rebulk::Im::Begin("Vulkan Infos");
		Rebulk::Im::Text("Extensions count : %u", m_VulkanRenderer->GetExtensionCount());

		Rebulk::Im::Text("\nExtensions loaded : ");
		for (const auto& extension : extensions) {
			Rebulk::Im::Text("\t%s", extension.extensionName);
		}

		const std::vector<const char*> validationLayers = m_VulkanRenderer->GetValidationLayers();

		Rebulk::Im::Text("\nValidation layers loaded : ");
		for (const auto& validation : validationLayers) {
			Rebulk::Im::Text("\t%s", validation);
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