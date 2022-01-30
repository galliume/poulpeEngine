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
	VkDescriptorPool descriptorPool = renderer->CreateDescriptorPool();
	std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores = renderer->CreateSyncObjects();	

	const std::vector<Rebulk::Vertex> vertices = {
		{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};

	std::pair<VkBuffer, VkDeviceMemory> vertexBuffer = renderer->CreateVertexBuffer(vertices);

	glfwSetWindowUserPointer(window, renderer);
	glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);

	double lastTime = glfwGetTime();
	bool show_demo_window = true;

	uint32_t imageIndex = 0;


	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		if (renderer->SouldResizeSwapChain(swapChain)) {
			VkSwapchainKHR old = swapChain;
			swapChain = renderer->CreateSwapChain(old);
			renderer->DestroySwapchain(renderer->GetDevice(), old, swapChainFramebuffers, swapChainImageViews);
			renderer->DestroySemaphores(semaphores);
			renderer->ResetCurrentFrameIndex();
			swapChainImageViews = renderer->CreateImageViews();
			swapChainFramebuffers = renderer->CreateFramebuffers(renderPass, swapChainImageViews);
			semaphores = renderer->CreateSyncObjects();
			renderer->ResetCommandPool(commandPool);
			commandBuffers = renderer->AllocateCommandBuffers(commandPool, swapChainFramebuffers.size());
		}

		for (size_t i = 0; i < commandBuffers.size(); i++) {
			imageIndex = renderer->AcquireNextImageKHR(swapChain, semaphores);

			renderer->BeginCommandBuffer(commandBuffers[imageIndex]);

			VkImageMemoryBarrier renderBeginBarrier = renderer->SetupImageMemoryBarrier(
				renderer->GetSwapChainImages()[imageIndex], 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			);

			renderer->AddPipelineBarrier(
				commandBuffers[imageIndex], renderBeginBarrier, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT
			);

			renderer->BeginRenderPass(renderPass, commandBuffers[imageIndex], swapChainFramebuffers[imageIndex]);
			renderer->SetViewPort(commandBuffers[imageIndex]);
			renderer->SetScissor(commandBuffers[imageIndex]);
			renderer->BindPipeline(commandBuffers[imageIndex], pipeline);
			renderer->Draw(commandBuffers[imageIndex], vertexBuffer.first, vertices);
			renderer->EndRenderPass(commandBuffers[imageIndex]);

			VkImageMemoryBarrier renderEndBarrier = renderer->SetupImageMemoryBarrier(
				renderer->GetSwapChainImages()[imageIndex], VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			);

			renderer->AddPipelineBarrier(
				commandBuffers[imageIndex], renderEndBarrier, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT
			);

			renderer->EndCommandBuffer(commandBuffers[imageIndex]);

			if (-1 != imageIndex) {
				renderer->QueueSubmit(imageIndex, commandBuffers[imageIndex], semaphores);
				renderer->QueuePresent(imageIndex, swapChain, semaphores);
			}	
		}

		//glfwSwapBuffers(window);
		renderer->WaitIdle();
	}
	
	renderer->DestroySwapchain(renderer->GetDevice(), swapChain, swapChainFramebuffers, swapChainImageViews);
	renderer->DestroySemaphores(semaphores);
	renderer->DestroyVertexBuffer(vertexBuffer.first, vertexBuffer.second);
	renderer->DestroyPipeline(pipeline, pipelineLayout, descriptorPool, descriptorSetLayout);
	renderer->Destroy(renderPass, commandPool, commandBuffers);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}