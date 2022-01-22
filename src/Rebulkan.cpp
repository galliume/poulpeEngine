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

	ImGui_ImplVulkan_InitInfo info = {};

	info.Instance = renderer->GetInstance();
	info.PhysicalDevice = renderer->GetPhysicalDevice();
	info.Device = renderer->GetDevice();
	info.QueueFamily = renderer->GetQueueFamily();
	info.Queue = renderer->GetGraphicsQueue();
	info.PipelineCache = nullptr;//to implement VkPipelineCache                 
	info.DescriptorPool = descriptorPool;
	info.Subpass = 0;
	info.MinImageCount = 2;
	info.ImageCount = 2;
	info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	info.Allocator = nullptr;
	info.CheckVkResultFn = [](VkResult err) {
		std::cerr << "IMGUI VULKAN ERROR " + std::to_string(err) << std::endl;
	};

	VkRenderPass imguiRenderPass = renderer->CreateRenderPass();
	VkCommandPool imguiCommandPool = renderer->CreateCommandPool();
	Rebulk::Im::Init(window, &info, imguiRenderPass);

	VkCommandBuffer commandBuffer = renderer->CreateCommandBuffer(commandPool);
	Rebulk::Im::CreateFontsTexture(commandBuffer);
	renderer->BeginRenderPass(imguiRenderPass, commandBuffer, swapChainFramebuffers);
	renderer->EndSingleTimeCommands(commandBuffer, commandPool);

	glfwSetWindowUserPointer(window, renderer);
	glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);

	Rebulk::VulkanLayer* vulkanLayer = new Rebulk::VulkanLayer(window, renderer);
	
	double lastTime = glfwGetTime();
	bool show_demo_window = true;

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		Rebulk::Im::NewFrame();

		double currentTime = glfwGetTime();
		double timeStep = currentTime - lastTime;

		vulkanLayer->DisplayFpsCounter(timeStep);
		vulkanLayer->DisplayLogs();
		vulkanLayer->DisplayAPI(renderer->GetDeviceProperties());

		lastTime = currentTime;		
		
		commandBuffer = renderer->CreateCommandBuffer(imguiCommandPool);
		Rebulk::Im::Render(window, commandBuffer, pipeline.first);

		commandBuffer = renderer->CreateCommandBuffer(commandPool);
		bool isExpired = renderer->DrawFrame(swapChain, commandBuffers, semaphores);
		renderer->BeginRenderPass(renderPass, commandBuffer, swapChainFramebuffers);
		renderer->EndSingleTimeCommands(commandBuffer, commandPool);
		
		if (isExpired) {
			renderer->CleanupSwapChain(swapChain, renderPass, commandPool, pipeline, swapChainImageViews, commandBuffers, swapChainFramebuffers);
			swapChain = renderer->CreateSwapChain();
			swapChainImageViews = renderer->CreateImageViews();
			renderPass = renderer->CreateRenderPass();
			pipeline = renderer->CreateGraphicsPipeline(renderPass);
			swapChainFramebuffers = renderer->CreateFramebuffers(renderPass, swapChainImageViews);
			commandPool = renderer->CreateCommandPool();			
			commandBuffers = renderer->CreateCommandBuffers(renderPass, commandPool, pipeline, swapChainFramebuffers);
			semaphores = renderer->CreateSyncObjects();
		} 

		glfwSwapBuffers(window);
	}

	
	
	Rebulk::Im::Destroy();

	vkDestroyRenderPass(renderer->GetDevice(), imguiRenderPass, nullptr);
	renderer->CleanupSwapChain(swapChain, renderPass, commandPool, pipeline, swapChainImageViews, commandBuffers, swapChainFramebuffers);
	renderer->Destroy(commandPool, semaphores);
	vkDestroyCommandPool(renderer->GetDevice(), imguiCommandPool, nullptr);

	vulkanLayer->Destroy();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}