#pragma once
#include "rebulkpch.h"
#include "Renderer\Vulkan\VulkanRenderer.h"
#include "GUI\Renderer\Im.h"
#include "Pattern\IObserver.h"

namespace Rebulk 
{
	class VulkanLayer : public IObserver
	{
	public:
		VulkanLayer(GLFWwindow* window, VulkanRenderer* vulkanRenderer);
		void Render();
		void DisplayLogs();
		void Destroy();
		void Update(std::vector<std::string>& messages);
		void DisplayFpsCounter(double timeStep);

	private:
		std::string m_Message;
		std::vector<std::string>m_Messages = {};
		int m_MaxMessages = 500;
		GLFWwindow* m_Window = nullptr;
		VulkanRenderer* m_VulkanRenderer = nullptr;
	};
}