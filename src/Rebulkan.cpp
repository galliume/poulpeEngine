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

	Rebulk::VulkanRenderer* imguiRenderer = new Rebulk::VulkanRenderer(window);

	VkSwapchainKHR imguiSwapChain = imguiRenderer->CreateSwapChain();
	std::vector<VkImageView> imguiSwapChainImageViews = imguiRenderer->CreateImageViews();
	VkRenderPass imguiRenderPass = imguiRenderer->CreateRenderPass();
	std::vector<VkFramebuffer> imguiSwapChainFramebuffers = imguiRenderer->CreateFramebuffers(imguiRenderPass, imguiSwapChainImageViews);
	VkCommandPool imguiCommandPool = imguiRenderer->CreateCommandPool();
	VkCommandBuffer imguiCommandBuffer = imguiRenderer->CreateCommandBuffer(imguiCommandPool);
	VkDescriptorSetLayout imguiDescriptorSetLayout = imguiRenderer->CreateDescriptorSetLayout();
	std::pair<VkPipeline, VkPipelineLayout>imguiPipeline = imguiRenderer->CreateGraphicsPipeline(imguiRenderPass, imguiCommandBuffer, imguiDescriptorSetLayout);
	VkDescriptorPool imguiDescriptorPool = imguiRenderer->CreateDescriptorPool();
	std::vector<VkCommandBuffer> imguiCommandBuffers = imguiRenderer->CreateCommandBuffers(imguiRenderPass, imguiCommandPool, imguiPipeline, imguiSwapChainFramebuffers);
	std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> imguiSemaphores = imguiRenderer->CreateSyncObjects();

	ImGui_ImplVulkan_InitInfo info = {};
	info.Instance = imguiRenderer->GetInstance();
	info.PhysicalDevice = imguiRenderer->GetPhysicalDevice();
	info.Device = imguiRenderer->GetDevice();
	info.QueueFamily = imguiRenderer->GetQueueFamily();
	info.Queue = imguiRenderer->GetGraphicsQueue();
	info.PipelineCache = nullptr;//to implement VkPipelineCache                 
	info.DescriptorPool = imguiDescriptorPool;
	info.Subpass = 0;
	info.MinImageCount = 2;
	info.ImageCount = 2;
	info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	info.Allocator = nullptr;
	info.CheckVkResultFn = [](VkResult err) {
		std::cerr << "IMGUI VULKAN ERROR " + std::to_string(err) << std::endl;
	};

	Rebulk::Im::Init(window, &info, imguiRenderPass);

	Rebulk::Im::CreateFontsTexture(imguiCommandBuffer);
	imguiRenderer->BeginRenderPass(imguiRenderPass, imguiCommandBuffer, imguiSwapChainFramebuffers);
	imguiRenderer->EndRenderPass(imguiCommandBuffer, imguiCommandPool);


	Rebulk::VulkanRenderer* renderer = new Rebulk::VulkanRenderer(window);
	//renderer->Init();

	VkSwapchainKHR swapChain = renderer->CreateSwapChain();
	std::vector<VkImageView> swapChainImageViews = renderer->CreateImageViews();
	VkRenderPass renderPass = renderer->CreateRenderPass();
	std::vector<VkFramebuffer> swapChainFramebuffers = renderer->CreateFramebuffers(renderPass, swapChainImageViews);
	VkCommandPool commandPool = renderer->CreateCommandPool();
	VkCommandBuffer commandBuffer = renderer->CreateCommandBuffer(commandPool);
	VkDescriptorSetLayout descriptorSetLayout = renderer->CreateDescriptorSetLayout();
	std::pair<VkPipeline, VkPipelineLayout>pipeline = renderer->CreateGraphicsPipeline(renderPass, commandBuffer, descriptorSetLayout);
	VkDescriptorPool descriptorPool = renderer->CreateDescriptorPool();
	std::vector<VkCommandBuffer> commandBuffers = renderer->CreateCommandBuffers(renderPass, commandPool, pipeline, swapChainFramebuffers);
	std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores = renderer->CreateSyncObjects();

	glfwSetWindowUserPointer(window, renderer);
	glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);

	Rebulk::VulkanLayer* vulkanLayer = new Rebulk::VulkanLayer(window, imguiRenderer);
	
	double lastTime = glfwGetTime();
	bool show_demo_window = true;

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		Rebulk::Im::NewFrame();

		double currentTime = glfwGetTime();
		double timeStep = currentTime - lastTime;

		vulkanLayer->DisplayFpsCounter(timeStep);
		vulkanLayer->DisplayLogs();
		vulkanLayer->DisplayAPI(imguiRenderer->GetDeviceProperties());

		lastTime = currentTime;		
		
		//Rebulk::Im::Render(window, imguiCommandBuffer, pipeline.first);
		vulkanLayer->Render(imguiRenderPass, imguiCommandPool, imguiSwapChain, imguiSwapChainFramebuffers, imguiPipeline.first);

		commandBuffer = renderer->CreateCommandBuffer(commandPool);
		renderer->BeginRenderPass(renderPass, commandBuffer, swapChainFramebuffers);
		bool isExpired = renderer->DrawFrame(swapChain, commandBuffers, semaphores);
		renderer->EndRenderPass(commandBuffer, commandPool);
		
		if (isExpired) {
			renderer->CleanupSwapChain(swapChain, renderPass, commandPool, pipeline, swapChainImageViews, commandBuffers, swapChainFramebuffers, descriptorPool, descriptorSetLayout);
			commandBuffer = renderer->CreateCommandBuffer(commandPool);
			swapChain = renderer->CreateSwapChain();
			swapChainImageViews = renderer->CreateImageViews();
			renderPass = renderer->CreateRenderPass();
			descriptorSetLayout = renderer->CreateDescriptorSetLayout();
			pipeline = renderer->CreateGraphicsPipeline(renderPass, commandBuffer, descriptorSetLayout);
			swapChainFramebuffers = renderer->CreateFramebuffers(renderPass, swapChainImageViews);
			commandPool = renderer->CreateCommandPool();			
			commandBuffers = renderer->CreateCommandBuffers(renderPass, commandPool, pipeline, swapChainFramebuffers);
			semaphores = renderer->CreateSyncObjects();
			descriptorPool = renderer->CreateDescriptorPool();
		} 

		glfwSwapBuffers(window);
	}

	Rebulk::Im::Destroy();

	renderer->CleanupSwapChain(swapChain, renderPass, commandPool, pipeline, swapChainImageViews, commandBuffers, swapChainFramebuffers, descriptorPool, descriptorSetLayout);
	renderer->Destroy(commandPool, semaphores);

	vulkanLayer->Destroy();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}