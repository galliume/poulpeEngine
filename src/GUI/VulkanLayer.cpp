#include "VulkanLayer.h"
#include <inttypes.h>

namespace Rebulk
{
	VulkanLayer::VulkanLayer(GLFWwindow* window, VulkanRenderer* vulkanRenderer)
		: m_Window(window), m_VulkanRenderer(vulkanRenderer)
	{
		m_VulkanRenderer->Attach(this);
	}
	
	void VulkanLayer::Render(VkRenderPass renderPass, VkCommandPool commandPool, VkSwapchainKHR swapChain, std::vector<VkFramebuffer> swapChainFramebuffers, VkPipeline pipeline)
	{
		VkCommandBuffer commandBuffer = m_VulkanRenderer->BeginSingleTimeCommands(renderPass, commandPool, swapChainFramebuffers);
		Im::Render(m_Window, commandBuffer, pipeline);
		m_VulkanRenderer->DrawSingleTimeCommands(commandBuffer, swapChain);
		m_VulkanRenderer->EndSingleTimeCommands(commandBuffer, commandPool);
	}

	void VulkanLayer::DisplayLogs()
	{
		Rebulk::Im::Begin("Vulkan Infos");
		ImGui::PushID("##VulkanInfos");
		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

		Rebulk::Im::BeginChild("VulkanLogs", ImVec2(0, -footer_height_to_reserve), true, ImGuiWindowFlags_HorizontalScrollbar);

		for (auto& message : m_Messages) {
			Rebulk::Im::Text("\t%s", message.c_str());
			Rebulk::Im::Separator();
		}

		Rebulk::Im::EndChild();
		ImGui::PopID();
		Rebulk::Im::End();
	}

	void VulkanLayer::Destroy()
	{
		Rebulk::Im::Destroy();
	}

	void VulkanLayer::Update(std::vector<std::string>& messages)
	{		
		if (m_Messages.size() >= m_MaxMessages) {
			m_Messages.erase(m_Messages.begin(), m_Messages.begin() + messages.size());
		}

		for (const auto& message : messages) {
			m_Messages.emplace_back(message);
		}
	}

	void VulkanLayer::DisplayFpsCounter(double timeStep)
	{
		Rebulk::Im::Begin("Performances stats");
		Rebulk::Im::Text("FPS : %f", 1/timeStep);
		Rebulk::Im::Text("Frametime : %f", timeStep);
		Rebulk::Im::End();
	}

	void VulkanLayer::DisplayAPI(VkPhysicalDeviceProperties devicesProps)
	{
		Rebulk::Im::Begin("API");
		Rebulk::Im::Text("API Version : %d", devicesProps.apiVersion);
		Rebulk::Im::Text("Drivers version : %d", devicesProps.driverVersion);
		Rebulk::Im::Text("Vendor id : %d", devicesProps.vendorID);
		Rebulk::Im::Text("GPU : %s" , devicesProps.deviceName);
		Rebulk::Im::End();
	}
}