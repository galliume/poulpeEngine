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

		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);

		for (const auto& message : m_Messages) {
			Rebulk::Log::GetLogger()->debug(message);
			Rebulk::Im::Text("\t%s", message.c_str());
			ImGui::Separator();
		}

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
		ImGui::EndChild();
		Rebulk::Im::End();
		m_HasBeenUpdated = true;
	}

	void VulkanLayer::Render()
	{
		Rebulk::Im::Render(m_Window);
	}

	void VulkanLayer::Destroy()
	{
		Rebulk::Im::Destroy();
	}

	void VulkanLayer::Update(std::vector<std::string>& messages)
	{		
		for (const auto& message : messages) {
			m_Messages.push_back(message);
		}
	}
}