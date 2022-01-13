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
		void Create();
		void Render();
		void Destroy();
		void Update(std::vector<std::string>& messages);

	private:
		std::string m_Message;
		bool m_HasBeenUpdated = false;
		std::vector<std::string>m_Messages = {};
		GLFWwindow* m_Window = nullptr;
		VulkanRenderer* m_VulkanRenderer = nullptr;
	};
}