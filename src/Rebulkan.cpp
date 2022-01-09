#include "rebulkpch.h"
#include "spdlog/details/null_mutex.h"
#include "GUI/VulkanLayer.h"

int main(int argc, char** argv)
{
	Rebulk::Log::Init();

	glfwInit();

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	Rebulk::VulkanRenderer* renderer = new Rebulk::VulkanRenderer(glfwExtensionCount, glfwExtensions);

	const uint32_t WIDTH = 2560;
	const uint32_t HEIGHT = 1440;

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Rebulkan", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	Rebulk::Im::Init(window);
	Rebulk::VulkanLayer* vulkanLayer = new Rebulk::VulkanLayer(window, renderer);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		vulkanLayer->Create();
		vulkanLayer->Render();

		glfwSwapBuffers(window);
	}

	vulkanLayer->Destroy();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}