#pragma once
#include "rebulkpch.h"

namespace Rbk
{
	//static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	//{
	//	auto renderer = reinterpret_cast<Rbk::VulkanRenderer*>(glfwGetWindowUserPointer(window));
	//	renderer->m_FramebufferResized = true;
	//};

	//glfwSetFramebufferSizeCallback(window->Get(), FramebufferResizeCallback);

	class Window
	{
		public:
			Window();
			inline GLFWwindow* Get() { return m_Window; };

	private:
		GLFWwindow* m_Window;
		bool m_ActiveVSync = false;
	};
}