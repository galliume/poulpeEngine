#include "rebulkpch.h"
#include "GUI/VulkanLayer.h"

static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto renderer = reinterpret_cast<Rebulk::VulkanRenderer*>(glfwGetWindowUserPointer(window));
	renderer->m_FramebufferResized = true;
}

static std::vector<char> ReadFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
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

	VkRenderPass renderPass = renderer->CreateRenderPass();

	auto vertShaderCode = ReadFile("shaders/spv/vert.spv");
	auto fragShaderCode = ReadFile("shaders/spv/frag.spv");

	VkShaderModule vertShaderModule = renderer->CreateShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = renderer->CreateShaderModule(fragShaderCode);

	VkDescriptorSetLayout descriptorSetLayout = renderer->CreateDescriptorSetLayout();
	VkPipelineLayout pipelineLayout = renderer->CreatePipelineLayout(descriptorSetLayout);
	VkPipelineCache pipelineCache = 0;
	VkPipeline pipeline = renderer->CreateGraphicsPipeline(renderPass, pipelineLayout, pipelineCache, vertShaderModule, fragShaderModule);
	VkSwapchainKHR swapChain = renderer->CreateSwapChain();
	std::vector<VkImageView> swapChainImageViews = renderer->CreateImageViews();
	std::vector<VkFramebuffer> swapChainFramebuffers = renderer->CreateFramebuffers(renderPass, swapChainImageViews);
	VkCommandPool commandPool = renderer->CreateCommandPool();
	std::vector<VkCommandBuffer> commandBuffers = renderer->AllocateCommandBuffers(commandPool, swapChainFramebuffers.size());
	renderer->CreateVertexBuffer();
	VkDescriptorPool descriptorPool = renderer->CreateDescriptorPool();
	std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores = renderer->CreateSyncObjects();

	//VkCommandBuffer commandBuffer = renderer->CreateCommandBuffer(commandPool);
	//VkDescriptorPool descriptorPool = renderer->CreateDescriptorPool();
	//std::vector<VkCommandBuffer> commandBuffers = renderer->CreateCommandBuffers(renderPass, commandPool, pipeline, swapChainFramebuffers);

	ImGui_ImplVulkan_InitInfo info = {};
	info.Instance = renderer->GetInstance();
	info.PhysicalDevice = renderer->GetPhysicalDevice();
	info.Device = renderer->GetDevice();
	info.QueueFamily = renderer->GetQueueFamily();
	info.Queue = renderer->GetGraphicsQueue();
	info.PipelineCache = nullptr;               
	info.DescriptorPool = descriptorPool;
	info.Subpass = 0;
	info.MinImageCount = 2;
	info.ImageCount = 2;
	info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	info.Allocator = nullptr;
	info.CheckVkResultFn = [](VkResult err) {
		std::cerr << "IMGUI VULKAN ERROR " + std::to_string(err) << std::endl;
	};


	VkCommandPool imguiCommandPool = renderer->CreateCommandPool();
	std::vector<VkCommandBuffer> imguiCommandBuffers = renderer->AllocateCommandBuffers(imguiCommandPool);

	Rebulk::Im::Init(window, &info, renderPass);
	renderer->BeginCommandBuffer(imguiCommandBuffers[0]);
	Rebulk::Im::CreateFontsTexture(imguiCommandBuffers[0]);
	renderer->EndCommandBuffer(imguiCommandBuffers[0]);
	
	Rebulk::VulkanLayer* vulkanLayer = new Rebulk::VulkanLayer(window, renderer);
	
	glfwSetWindowUserPointer(window, renderer);
	glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);

	double lastTime = glfwGetTime();
	bool show_demo_window = true;

	renderer->CreateVertexBuffer();
	uint32_t imageIndex = 0;

	while (!glfwWindowShouldClose(window)) {

		double currentTime = glfwGetTime();
		double timeStep = currentTime - lastTime;

		Rebulk::Im::NewFrame();
		vulkanLayer->DisplayFpsCounter(timeStep);
		vulkanLayer->DisplayLogs();
		vulkanLayer->DisplayAPI(renderer->GetDeviceProperties());

		renderer->ResetCommandPool(imguiCommandPool);
		renderer->BeginCommandBuffer(imguiCommandBuffers[0]);

		ImGuiIO& io = ImGui::GetIO();
		ImGui::Render();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imguiCommandBuffers[0], pipeline);
		renderer->Draw(imguiCommandBuffers[0]);
		ImGui::EndFrame();

		//Rebulk::Im::Render(window, imguiCommandBuffers[0], pipeline);
		renderer->EndCommandBuffer(imguiCommandBuffers[0]);


		renderer->ResetCommandPool(commandPool);

		/*for (size_t i = 0; i < commandBuffers.size(); i++) {
			renderer->BeginCommandBuffer(commandBuffers[i]);
			renderer->BeginRenderPass(renderPass, commandBuffers[i], swapChainFramebuffers[imageIndex]);
			renderer->SetViewPort(commandBuffers[i]);
			renderer->SetScissor(commandBuffers[i]);
			renderer->BindPipeline(commandBuffers[i], pipeline);
			renderer->Draw(commandBuffers[i]);
			renderer->EndRenderPass(commandBuffers[i]);
			renderer->EndCommandBuffer(commandBuffers[i]);
			imageIndex = renderer->AcquireNextImageKHR(swapChain, semaphores);

			if (-1 != imageIndex) {
				renderer->QueueSubmit(imageIndex, commandBuffers[imageIndex], semaphores);
				renderer->QueuePresent(imageIndex, swapChain, semaphores);
			}

			if (renderer->IsFramebufferResized()) {
				imageIndex = 0;

				int width = 0, height = 0;
				glfwGetFramebufferSize(window, &width, &height);

				while (width == 0 || height == 0)
				{
					glfwGetFramebufferSize(window, &width, &height);
					glfwWaitEvents();
				}

				renderer->WaitIdle();

				renderer->CleanupSwapChain(swapChain, renderPass, commandPool, pipeline, pipelineLayout, swapChainImageViews, commandBuffers, swapChainFramebuffers, descriptorPool, descriptorSetLayout);
				
				renderPass = renderer->CreateRenderPass();

				descriptorSetLayout = renderer->CreateDescriptorSetLayout();
				pipelineLayout = renderer->CreatePipelineLayout(descriptorSetLayout);
				pipelineCache = 0;

				vertShaderModule = renderer->CreateShaderModule(vertShaderCode);
				fragShaderModule = renderer->CreateShaderModule(fragShaderCode);

				pipeline = renderer->CreateGraphicsPipeline(renderPass, pipelineLayout, pipelineCache, vertShaderModule, fragShaderModule);
				swapChain = renderer->CreateSwapChain();
				swapChainImageViews = renderer->CreateImageViews();
				swapChainFramebuffers = renderer->CreateFramebuffers(renderPass, swapChainImageViews);
				commandPool = renderer->CreateCommandPool();
				commandBuffers = renderer->AllocateCommandBuffers(commandPool, swapChainFramebuffers.size());
				renderer->CreateVertexBuffer();
				renderer->CreateDescriptorPool();
				semaphores = renderer->CreateSyncObjects();
				renderer->SetFramebufferResized();
			}

			renderer->WaitIdle();
		}*/

		/*ImDrawData* main_draw_data = ImGui::GetDrawData();
		const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);*/

		glfwSwapBuffers(window);
		lastTime = currentTime;		
	}

	//Rebulk::Im::Destroy();

	//renderer->CleanupSwapChain(swapChain, renderPass, commandPool, pipeline, swapChainImageViews, commandBuffers, swapChainFramebuffers, descriptorPool, descriptorSetLayout);
	//renderer->Destroy(commandPool, semaphores);

	//vulkanLayer->Destroy();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}