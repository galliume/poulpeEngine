#pragma once
#include "rebulkpch.h"
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"
#include "Rebulk/GUI/ImGui/Im.h"
#include "Rebulk/Pattern/IObserver.h"

namespace Rbk 
{
	class VulkanLayer : public IObserver
	{
	public:
		VulkanLayer(GLFWwindow* window, VulkanRenderer* vulkanRenderer);
		void Render(VkRenderPass renderPass, VkCommandPool commandPool, VkSwapchainKHR swapChain, std::vector<VkFramebuffer> swapChainFramebuffers, VkPipeline pipeline);
		void DisplayLogs();
		void Destroy();
		void Update(std::vector<std::string>& messages);
		void DisplayFpsCounter(double timeStep);
		void DisplayAPI(VkPhysicalDeviceProperties devicesProps);

	private:
		std::string m_Message;
		std::vector<std::string>m_Messages = {};
		int m_MaxMessages = 500;
		GLFWwindow* m_Window = nullptr;
		VulkanRenderer* m_VulkanRenderer = nullptr;
	};
}