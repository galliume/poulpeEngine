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

	VkSwapchainKHR swapChain = renderer->CreateSwapChain();
	std::vector<VkImageView> swapChainImageViews = renderer->CreateImageViews();
	VkRenderPass renderPass = renderer->CreateRenderPass();
	std::vector<VkFramebuffer> swapChainFramebuffers = renderer->CreateFramebuffers(renderPass, swapChainImageViews);
	VkCommandPool commandPool = renderer->CreateCommandPool();
	VkCommandBuffer commandBuffer = renderer->CreateCommandBuffer(commandPool);
	VkDescriptorSetLayout descriptorSetLayout = renderer->CreateDescriptorSetLayout();
	std::pair<VkPipeline, VkPipelineLayout>pipeline = renderer->CreateGraphicsPipeline(renderPass, commandBuffer, descriptorSetLayout);
	renderer->CreateVertexBuffer();
	VkDescriptorPool descriptorPool = renderer->CreateDescriptorPool();
	std::vector<VkCommandBuffer> commandBuffers = renderer->CreateCommandBuffers(renderPass, commandPool, pipeline, swapChainFramebuffers);
	std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores = renderer->CreateSyncObjects();

	//ImGui_ImplVulkan_InitInfo info = {};
	//info.Instance = renderer->GetInstance();
	//info.PhysicalDevice = renderer->GetPhysicalDevice();
	//info.Device = renderer->GetDevice();
	//info.QueueFamily = renderer->GetQueueFamily();
	//info.Queue = renderer->GetGraphicsQueue();
	//info.PipelineCache = nullptr;               
	//info.DescriptorPool = descriptorPool;
	//info.Subpass = 0;
	//info.MinImageCount = 2;
	//info.ImageCount = 2;
	//info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	//info.Allocator = nullptr;
	//info.CheckVkResultFn = [](VkResult err) {
	//	std::cerr << "IMGUI VULKAN ERROR " + std::to_string(err) << std::endl;
	//};

	//Rebulk::Im::Init(window, &info, renderPass);
	//Rebulk::Im::CreateFontsTexture(commandBuffer);
	//renderer->BeginRenderPass(renderPass, commandBuffer, swapChainFramebuffers);
	//renderer->EndRenderPass(commandBuffer, commandPool);

	//Rebulk::VulkanLayer* vulkanLayer = new Rebulk::VulkanLayer(window, renderer);
	
	glfwSetWindowUserPointer(window, renderer);
	glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);

	double lastTime = glfwGetTime();
	bool show_demo_window = true;

	while (!glfwWindowShouldClose(window)) {

		commandBuffer = renderer->CreateCommandBuffer(commandPool);
		double currentTime = glfwGetTime();
		double timeStep = currentTime - lastTime;

		glfwPollEvents();
			
		//Rebulk::Im::NewFrame();

		//vulkanLayer->DisplayFpsCounter(timeStep);
		//vulkanLayer->DisplayLogs();
		//vulkanLayer->DisplayAPI(renderer->GetDeviceProperties());
	
		//Rebulk::Im::Render(window, commandBuffer, pipeline.first);

		//ImDrawData* main_draw_data = ImGui::GetDrawData();
		//const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
	
		if (true) {
			renderer->BeginRenderPass(renderPass, commandBuffer, swapChainFramebuffers);
			renderer->EndRenderPass(commandBuffer, commandPool);
			bool isExpired = renderer->DrawFrame(swapChain, commandBuffers, semaphores);

			if (isExpired ) {
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
		}

		glfwSwapBuffers(window);
		lastTime = currentTime;		
	}

	//Rebulk::Im::Destroy();

	renderer->CleanupSwapChain(swapChain, renderPass, commandPool, pipeline, swapChainImageViews, commandBuffers, swapChainFramebuffers, descriptorPool, descriptorSetLayout);
	renderer->Destroy(commandPool, semaphores);

	//vulkanLayer->Destroy();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}