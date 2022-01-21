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

	Rebulk::VulkanRenderer* renderer = new Rebulk::VulkanRenderer(window);
	//renderer->Init();
	
	VkSwapchainKHR swapChain = renderer->CreateSwapChain();
	std::vector<VkImageView> swapChainImageViews = renderer->CreateImageViews();
	VkRenderPass renderPass = renderer->CreateRenderPass();
	std::pair<VkPipeline, VkPipelineLayout>pipeline = renderer->CreateGraphicsPipeline(renderPass);
	
	std::vector<VkFramebuffer> swapChainFramebuffers = renderer->CreateFramebuffers(renderPass, swapChainImageViews);
	VkCommandPool commandPool = renderer->CreateCommandPool();
	VkDescriptorPool descriptorPool = renderer->CreateDescriptorPool();
	std::vector<VkCommandBuffer> commandBuffers = renderer->CreateCommandBuffers(renderPass, commandPool, pipeline, swapChainFramebuffers);
	std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores = renderer->CreateSyncObjects();
	/*m_ImageAvailableSemaphores = semaphores.first;
	m_RenderFinishedSemaphores = semaphores.second;*/

	ImGui_ImplVulkan_InitInfo info = {};

	//info.Instance = renderer->GetInstance();
	//info.PhysicalDevice = renderer->GetPhysicalDevice();
	//info.Device = renderer->GetDevice();
	//info.QueueFamily = renderer->GetQueueFamily();
	//info.Queue = renderer->GetGraphicsQueue();
	//info.PipelineCache = nullptr;//to implement VkPipelineCache                 
	//info.DescriptorPool = renderer->GetDescriptorPool();
	//info.Subpass = 0;
	//info.MinImageCount = 2;
	//info.ImageCount = 2;
	//info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	//info.Allocator = nullptr;
	//info.CheckVkResultFn = [](VkResult err) {
	//	std::cerr << "IMGUI VULKAN ERROR " + std::to_string(err) << std::endl;
	//};

	//Rebulk::Im::Init(window, &info, renderer->GetRenderPass());
	//VkCommandBuffer commandBuffer = renderer->BeginSingleTimeCommands();
	//ebulk::Im::CreateFontsTexture(commandBuffer);
	//renderer->EndSingleTimeCommands(commandBuffer);

	glfwSetWindowUserPointer(window, renderer);
	glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
	//Rebulk::VulkanLayer* vulkanLayer = new Rebulk::VulkanLayer(window, renderer);
	
	double lastTime = glfwGetTime();
	bool show_demo_window = true;

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		//Rebulk::Im::NewFrame();

		double currentTime = glfwGetTime();
		double timeStep = currentTime - lastTime;

		//vulkanLayer->DisplayFpsCounter(timeStep);
		//vulkanLayer->DisplayLogs();
		//vulkanLayer->DisplayAPI(renderer->GetDeviceProperties());

		lastTime = currentTime;
		
		//ImGui::ShowDemoWindow(&show_demo_window);
		
		//vulkanLayer->Render();
		renderer->DrawFrame(swapChain, commandBuffers, semaphores);

		glfwSwapBuffers(window);
	}

	//renderer->Destroy();	
	//vulkanLayer->Destroy();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}