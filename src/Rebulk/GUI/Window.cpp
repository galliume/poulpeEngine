#include "Window.h"

namespace Rbk
{
	Window::Window()
	{
	}

	void Window::Init()
	{
		const uint32_t WIDTH = 2560;
		const uint32_t HEIGHT = 1440;

		const char* glsl_version = "#version 150";
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		
		m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Rebulkan Engine", nullptr, nullptr);

		glfwMakeContextCurrent(m_Window);
		glfwSwapInterval(m_ActiveVSync);

		glfwSetWindowUserPointer(m_Window, this);

		glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* glfwWindow, int width, int height) {
			Rbk::m_FramebufferResized = true;
		});

		Rbk::m_FramebufferResized = false;
		//glfwSetInputMode(m_Window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
		//glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}