#include "rebulkpch.h"
#include "spdlog/details/null_mutex.h"
#include "GUI/VulkanLayer.h"

int main(int argc, char** argv)
{
	Rebulk::Log::Init();

	glfwInit();

	const uint32_t WIDTH = 2560;
	const uint32_t HEIGHT = 1440;

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Rebulkan", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	Rebulk::Im::Init(window);
	Rebulk::VulkanRenderer* renderer = new Rebulk::VulkanRenderer();
	Rebulk::VulkanLayer* vulkanLayer = new Rebulk::VulkanLayer(window, renderer);
	renderer->Init();
	//bool show_demo_window = true;

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		vulkanLayer->Create();
		//ImGui::ShowDemoWindow(&show_demo_window);
		vulkanLayer->Render();

		glfwSwapBuffers(window);
	}

	vulkanLayer->Destroy();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}