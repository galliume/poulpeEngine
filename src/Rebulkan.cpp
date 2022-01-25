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
	VkDescriptorPool descriptorPool = renderer->CreateDescriptorPool();
	VkDescriptorSetLayout descriptorSetLayout = renderer->CreateDescriptorSetLayout();
	std::vector<VkDescriptorSet>descriporSets = renderer->CreateDescriptorSets(descriptorPool, descriptorSetLayout, swapChainImageViews);
	std::pair<VkPipeline, VkPipelineLayout>pipeline = renderer->CreateGraphicsPipeline(renderPass, descriptorSetLayout);
	std::vector<VkCommandBuffer> commandBuffers;// = renderer->CreateCommandBuffers(renderPass, commandPool, pipeline, swapChainFramebuffers);
	std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores = renderer->CreateSyncObjects();

	//VkDescriptorPool imguiDescriptorPool = renderer->CreateDescriptorPool();
	//std::vector<VkDescriptorSet>imguiDescriporSets = renderer->CreateDescriptorSets(imguiDescriptorPool, descriptorSetLayout, swapChainImageViews);

	//ImGui_ImplVulkan_InitInfo info = {};
	//info.Instance = renderer->GetInstance();
	//info.PhysicalDevice = renderer->GetPhysicalDevice();
	//info.Device = renderer->GetDevice();
	//info.QueueFamily = renderer->GetQueueFamily();
	//info.Queue = renderer->GetGraphicsQueue();
	//info.PipelineCache = nullptr;               
	//info.DescriptorPool = imguiDescriptorPool;
	//info.Subpass = 0;
	//info.MinImageCount = 2;
	//info.ImageCount = 2;
	//info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	//info.Allocator = nullptr;
	//info.CheckVkResultFn = [](VkResult err) {
	//	std::cerr << "IMGUI VULKAN ERROR " + std::to_string(err) << std::endl;
	//};

	//Rebulk::Im::Init(window, &info, renderPass);
		//renderer->BeginRenderPass(renderPass, commandBuffers, swapChainFramebuffers, pipeline);
		//for (size_t i = 0; i < commandBuffers.size(); i++) {
		//Rebulk::Im::CreateFontsTexture(commandBuffers[0]);
	//}
		//renderer->EndRenderPass(commandBuffers);

	//Rebulk::VulkanLayer* vulkanLayer = new Rebulk::VulkanLayer(window, renderer);
	
	glfwSetWindowUserPointer(window, renderer);
	glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);

	double lastTime = glfwGetTime();
	bool show_demo_window = true;

	while (!glfwWindowShouldClose(window)) {


		double currentTime = glfwGetTime();
		double timeStep = currentTime - lastTime;

		glfwPollEvents();

		//Rebulk::Im::NewFrame();

		//vulkanLayer->DisplayFpsCounter(timeStep);
		//vulkanLayer->DisplayLogs();
		//vulkanLayer->DisplayAPI(renderer->GetDeviceProperties());

		//Rebulk::Im::Render(window);

		//ImDrawData* mainDrawData = ImGui::GetDrawData();
		//const bool mainIsMinimized = (mainDrawData->DisplaySize.x <= 0.0f || mainDrawData->DisplaySize.y <= 0.0f);
		//std::cout << "x :" << mainDrawData->DisplaySize.x << std::endl;
		//std::cout << "y :" << mainDrawData->DisplaySize.y << std::endl;
		//std::cout << "mainIsMinimized :" << mainIsMinimized << std::endl;

		if (true) {
			commandBuffers = renderer->CreateCommandBuffers(renderPass, commandPool, pipeline, swapChainFramebuffers);
			renderer->BeginRenderPass(renderPass, commandBuffers, swapChainFramebuffers, pipeline);
			//for (size_t i = 0; i < commandBuffers.size(); i++) {
				//Rebulk::Im::RenderDrawData(commandBuffers[0], pipeline.first);
			//}
			//Rebulk::Im::RenderPlatformWindows();
			for (size_t i = 0; i < commandBuffers.size(); i++) {
				vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
			}

			renderer->EndRenderPass(commandBuffers);

			bool isExpired = renderer->DrawFrame(swapChain, commandBuffers, semaphores);

			if (isExpired) {
				renderer->CleanupSwapChain(swapChain, renderPass, commandPool, pipeline, swapChainImageViews, commandBuffers, swapChainFramebuffers, descriptorPool, descriptorSetLayout);
				swapChain = renderer->CreateSwapChain();
				swapChainImageViews = renderer->CreateImageViews();
				renderPass = renderer->CreateRenderPass();
				descriptorSetLayout = renderer->CreateDescriptorSetLayout();
				descriptorPool = renderer->CreateDescriptorPool();
				pipeline = renderer->CreateGraphicsPipeline(renderPass, descriptorSetLayout);
				swapChainFramebuffers = renderer->CreateFramebuffers(renderPass, swapChainImageViews);
				commandPool = renderer->CreateCommandPool();
				commandBuffers = renderer->CreateCommandBuffers(renderPass, commandPool, pipeline, swapChainFramebuffers);
				semaphores = renderer->CreateSyncObjects();
				//ImGui_ImplVulkan_SetMinImageCount(renderer->QuerySwapChainSupport(renderer->GetPhysicalDevice()).capabilities.minImageCount);
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