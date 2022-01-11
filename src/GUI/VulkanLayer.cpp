#include "VulkanLayer.h"
#include <inttypes.h>

namespace Rebulk
{
	VulkanLayer::VulkanLayer(GLFWwindow* window, VulkanRenderer* vulkanRenderer)
		: m_Window(window), m_VulkanRenderer(vulkanRenderer)
	{
		m_VulkanRenderer->Attach(this);
	}
	
	void VulkanLayer::Create()
	{
		std::vector<VkExtensionProperties>extensions = m_VulkanRenderer->GetExtensions();

		Rebulk::Im::NewFrame();
		Rebulk::Im::Begin("Vulkan Infos");
		//Rebulk::Im::Text("Extensions count : %u", m_VulkanRenderer->GetExtensionCount());

		//Rebulk::Im::Text("\nExtensions loaded : ");
		//for (const auto& extension : extensions) {
		//	Rebulk::Im::Text("\t%s", extension.extensionName);
		//}

		//std::vector<VkLayerProperties> layersAvailable = m_VulkanRenderer->GetLayersAvailable();

		//Rebulk::Im::Text("\nAvailable layers");
		//for (const auto& layer : layersAvailable) {
		//	Rebulk::Im::Text("\t%s", layer);
		//}

		//const std::vector<const char*> validationLayers = m_VulkanRenderer->GetValidationLayers();

		//Rebulk::Im::Text("\nValidation layers status : %s", m_VulkanRenderer->IsValidationLayersEnabled() ? "enabled" : "disabled");

		//Rebulk::Im::Text("\nValidation layers loaded : ");
		//for (const auto& validation : validationLayers) {
		//	Rebulk::Im::Text("\t%s", validation);
		//}

		//Rebulk::Im::Text("\nVulkan cration instance status : %s", m_VulkanRenderer->IsInstanceCreated() ? "created" : "failed");
	}

	void VulkanLayer::Render()
	{
		Rebulk::Im::End();
		Rebulk::Im::Render(m_Window);
	}

	void VulkanLayer::Destroy()
	{
		Rebulk::Im::Destroy();
	}

	void VulkanLayer::Update(std::string& title, std::vector<std::string>& messages)
	{
		Rebulk::Im::Text(title);
		Rebulk::Log::GetLogger()->debug(title);
		for (const auto& message : messages) {
			Rebulk::Log::GetLogger()->debug(message);
			Rebulk::Im::Text("\t%s", message);
		}
	}
}