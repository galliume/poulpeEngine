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
			void Init();
			inline GLFWwindow* Get() { return m_Window; };
			inline void SetVSync(bool active) { m_ActiveVSync = active; };

	private:
		GLFWwindow* m_Window;
		bool m_ActiveVSync = false;
	};
}