#include "rebulkpch.h"
#include "Renderer\Vulkan\VulkanRenderer.h"

int main(int argc, char** argv)
{
	Rebulk::Log::Init();

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	Rebulk::VulkanRenderer renderer = Rebulk::VulkanRenderer(glfwExtensionCount, glfwExtensions);

	const uint32_t WIDTH = 2560;
	const uint32_t HEIGHT = 1440;

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Rebulkan", nullptr, nullptr);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}