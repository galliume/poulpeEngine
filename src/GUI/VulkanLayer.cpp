#include "VulkanLayer.h"
#include <inttypes.h>

namespace Rbk
{
	VulkanLayer::VulkanLayer(GLFWwindow* window, VulkanRenderer* vulkanRenderer)
		: m_Window(window), m_VulkanRenderer(vulkanRenderer)
	{
		m_VulkanRenderer->Attach(this);
	}
	
	void VulkanLayer::Render(VkRenderPass renderPass, VkCommandPool commandPool, VkSwapchainKHR swapChain, std::vector<VkFramebuffer> swapChainFramebuffers, VkPipeline pipeline)
	{
		//VkCommandBuffer commandBuffer = m_VulkanRenderer->CreateCommandBuffer(commandPool);
		//m_VulkanRenderer->BeginRenderPass(renderPass, commandBuffer, swapChainFramebuffers);
		//Im::Render(m_Window, commandBuffer, pipeline);
		//m_VulkanRenderer->DrawSingleTimeCommands(commandBuffer, swapChain);
		//m_VulkanRenderer->EndRenderPass(commandBuffer, commandPool);
	}

	void VulkanLayer::DisplayLogs()
	{
		Rbk::Im::Begin("Vulkan Infos");
		ImGui::PushID("##VulkanInfos");
		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

		Rbk::Im::BeginChild("VulkanLogs", ImVec2(0, -footer_height_to_reserve), true, ImGuiWindowFlags_HorizontalScrollbar);

		for (auto& message : m_Messages) {
			Rbk::Im::Text("\t%s", message.c_str());
			Rbk::Im::Separator();
		}

		Rbk::Im::EndChild();
		ImGui::PopID();
		Rbk::Im::End();
	}

	void VulkanLayer::Destroy()
	{
		Rbk::Im::Destroy();
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
		Rbk::Im::Begin("Performances stats");
		Rbk::Im::Text("FPS : %f", 1/timeStep);
		Rbk::Im::Text("Frametime : %f", timeStep);
		Rbk::Im::End();
	}

	void VulkanLayer::DisplayAPI(VkPhysicalDeviceProperties devicesProps)
	{
		Rbk::Im::Begin("API");
		Rbk::Im::Text("API Version : %d", devicesProps.apiVersion);
		Rbk::Im::Text("Drivers version : %d", devicesProps.driverVersion);
		Rbk::Im::Text("Vendor id : %d", devicesProps.vendorID);
		Rbk::Im::Text("GPU : %s" , devicesProps.deviceName);
		Rbk::Im::End();
	}
}