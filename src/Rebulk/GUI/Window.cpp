#include "Window.h"

namespace Rbk
{
	Window::Window()
	{
		const uint32_t WIDTH = 2560;
		const uint32_t HEIGHT = 1440;

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Rebulkan Engine", nullptr, nullptr);

		glfwMakeContextCurrent(m_Window);
		glfwSwapInterval(m_ActiveVSync);
	}
}