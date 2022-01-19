#include "rebulkpch.h"
#include "GUI/VulkanLayer.h"

static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto renderer = reinterpret_cast<Rebulk::VulkanRenderer*>(glfwGetWindowUserPointer(window));
	renderer->m_FramebufferResized = true;
}

int main(int argc, char** argv)
{
	Rebulk::Log::Init();
	
	const uint32_t WIDTH = 2560;
	const uint32_t HEIGHT = 1440;

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Rebulkan", nullptr, nullptr);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);//vsync

	//Rebulk::Im::Init(window);
	Rebulk::VulkanRenderer* renderer = new Rebulk::VulkanRenderer(window);
	glfwSetWindowUserPointer(window, renderer);
	glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
	//Rebulk::VulkanLayer* vulkanLayer = new Rebulk::VulkanLayer(window, renderer);
	
	renderer->Init();
	double lastTime = glfwGetTime();
	//bool show_demo_window = true;

	while (!glfwWindowShouldClose(window)) {

		//Rebulk::Im::NewFrame();

		double currentTime = glfwGetTime();
		double timeStep = currentTime - lastTime;
		renderer->DrawFrame();

		//vulkanLayer->DisplayFpsCounter(timeStep);
		//vulkanLayer->DisplayLogs();
		//vulkanLayer->DisplayAPI(renderer->GetDeviceProperties());

		lastTime = currentTime;
		
		glfwPollEvents();

		//ImGui::ShowDemoWindow(&show_demo_window);
		//vulkanLayer->Render();

		glfwSwapBuffers(window);
	}

	renderer->Destroy();	
	//vulkanLayer->Destroy();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}