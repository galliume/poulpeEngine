#pragma once
#include "rebulkpch.h"
#include "Renderer\Vulkan\VulkanRenderer.h"
#include "GUI\Renderer\Im.h"

namespace Rebulk
{
	class VulkanLayer
	{
	public:
		VulkanLayer(GLFWwindow* window, VulkanRenderer* vulkanRenderer);
		void Create();
		void Render();
		void Destroy();

	private:
		GLFWwindow* m_Window = nullptr;
		VulkanRenderer* m_VulkanRenderer = nullptr;
	};
}