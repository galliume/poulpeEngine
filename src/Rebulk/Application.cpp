#include "Application.h"

namespace Rbk
{
	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto renderer = reinterpret_cast<Rbk::VulkanRenderer*>(glfwGetWindowUserPointer(window));
		renderer->m_FramebufferResized = true;
	}
	
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		if (s_Instance == nullptr) {
			s_Instance = this;
		}
	}

	Application::~Application()
	{
		std::cout << "Application deleted" << std::endl;
	}

	void Application::Run()
	{
		Rbk::Log::Init();

		const uint32_t WIDTH = 2560;
		const uint32_t HEIGHT = 1440;

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Rbkan", nullptr, nullptr);

		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);//vsync

		std::unique_ptr<Rbk::VulkanRenderer> renderer = std::make_unique<Rbk::VulkanRenderer>(Rbk::VulkanRenderer(window));

		VkRenderPass renderPass = renderer->CreateRenderPass();

		auto vertShaderCode = ReadFile("shaders/spv/vert.spv");
		auto fragShaderCode = ReadFile("shaders/spv/frag.spv");

		VkShaderModule vertShaderModule = renderer->CreateShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = renderer->CreateShaderModule(fragShaderCode);

		std::vector<VkImage> swapChainImages = {};

		VkDescriptorSetLayout descriptorSetLayout = renderer->CreateDescriptorSetLayout();
		VkPipelineLayout pipelineLayout = renderer->CreatePipelineLayout(descriptorSetLayout);
		VkPipelineCache pipelineCache = 0;
		VkPipeline pipeline = renderer->CreateGraphicsPipeline(renderPass, pipelineLayout, pipelineCache, vertShaderModule, fragShaderModule);
		VkSwapchainKHR swapChain = renderer->CreateSwapChain(swapChainImages);
		std::vector<VkImageView> swapChainImageViews = renderer->CreateImageViews(swapChainImages);
		std::vector<VkFramebuffer> swapChainFramebuffers = renderer->CreateFramebuffers(renderPass, swapChainImageViews);
		VkCommandPool commandPool = renderer->CreateCommandPool();
		std::vector<VkCommandBuffer> commandBuffers = renderer->AllocateCommandBuffers(commandPool, (uint32_t)swapChainFramebuffers.size());
		VkDescriptorPool descriptorPool = renderer->CreateDescriptorPool(swapChainImages);
		std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores = renderer->CreateSyncObjects(swapChainImages);

		glfwSetWindowUserPointer(window, renderer.get());
		glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);

		double lastTime = glfwGetTime();
		bool show_demo_window = true;

		uint32_t imageIndex = 0;

		Rbk::Mesh meshObj;
		Rbk::TinyObjLoader::LoadMesh(meshObj, "mesh/moon.obj");

		std::pair<VkBuffer, VkDeviceMemory> meshVBuffer = renderer->CreateVertexBuffer(commandPool, meshObj.vertices);
		std::pair<VkBuffer, VkDeviceMemory> meshIBuffer = renderer->CreateIndexBuffer(commandPool, meshObj.indices);
		std::pair<std::vector<VkBuffer>, std::vector<VkDeviceMemory>> uniformBuffers = renderer->CreateUniformBuffers(swapChainImageViews);

		std::vector<VkDescriptorSet> descriptorSets = renderer->CreateDescriptorSets(descriptorPool, descriptorSetLayout, swapChainImages, uniformBuffers);

		while (!glfwWindowShouldClose(window)) {

			double currentTime = glfwGetTime();
			double timeStep = currentTime - lastTime;

			glfwPollEvents();

			if (renderer->SouldResizeSwapChain(swapChain)) {
				VkSwapchainKHR old = swapChain;
				swapChain = renderer->CreateSwapChain(swapChainImages, old);
				renderer->DestroySwapchain(renderer->GetDevice(), old, swapChainFramebuffers, swapChainImageViews);
				renderer->DestroySemaphores(semaphores);
				renderer->ResetCurrentFrameIndex();
				swapChainImageViews = renderer->CreateImageViews(swapChainImages);
				swapChainFramebuffers = renderer->CreateFramebuffers(renderPass, swapChainImageViews);
				semaphores = renderer->CreateSyncObjects(swapChainImages);
				renderer->ResetCommandPool(commandPool);
				commandBuffers = renderer->AllocateCommandBuffers(commandPool, (uint32_t)swapChainFramebuffers.size());
			}

			for (size_t i = 0; i < commandBuffers.size() - 1; i++) {

				imageIndex = renderer->AcquireNextImageKHR(swapChain, semaphores);

				renderer->BeginCommandBuffer(commandBuffers[imageIndex]);

				VkImageMemoryBarrier renderBeginBarrier = renderer->SetupImageMemoryBarrier(
					swapChainImages[imageIndex], 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				);

				renderer->AddPipelineBarrier(
					commandBuffers[imageIndex], renderBeginBarrier, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT
				);

				renderer->BeginRenderPass(renderPass, commandBuffers[imageIndex], swapChainFramebuffers[imageIndex]);
				renderer->SetViewPort(commandBuffers[imageIndex]);
				renderer->SetScissor(commandBuffers[imageIndex]);
				renderer->BindPipeline(commandBuffers[imageIndex], pipeline);

				renderer->Draw(commandBuffers[imageIndex], meshVBuffer.first, meshIBuffer.first, meshObj.indices, uniformBuffers.first[imageIndex], descriptorSets[imageIndex], pipelineLayout);

				renderer->UpdateUniformBuffer(uniformBuffers.second[imageIndex]);
				renderer->EndRenderPass(commandBuffers[imageIndex]);

				VkImageMemoryBarrier renderEndBarrier = renderer->SetupImageMemoryBarrier(
					swapChainImages[imageIndex], VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
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

			renderer->WaitIdle();
			renderer->ResetCommandPool(commandPool);

			glfwSwapBuffers(window);
			lastTime = currentTime;
		}

		//renderer->DestroySwapchain(renderer->GetDevice(), imguiSwapChain, imguiSwapChainFramebuffers, imguiSwapChainImageViews);
		//renderer->DestroyRenderPass(imguiRenderPass, imguiCommandPool, imguiCommandBuffers);
		renderer->DestroySwapchain(renderer->GetDevice(), swapChain, swapChainFramebuffers, swapChainImageViews);
		renderer->DestroySemaphores(semaphores);

		renderer->DestroyBuffer(meshVBuffer.first);
		renderer->DestroyDeviceMemory(meshVBuffer.second);
		renderer->DestroyBuffer(meshIBuffer.first);
		renderer->DestroyDeviceMemory(meshIBuffer.second);
		for (auto& buffer : uniformBuffers.first) {
			renderer->DestroyBuffer(buffer);
		}
		for (auto& deviceMemory : uniformBuffers.second) {
			renderer->DestroyDeviceMemory(deviceMemory);
		}

		/*renderer->DestroyVertexBuffer(vertexBuffer.first, vertexBuffer.second);
		renderer->DestroyVertexBuffer(indexBuffer.first, indexBuffer.second);*/
		renderer->DestroyPipeline(pipeline, pipelineLayout, descriptorPool, descriptorSetLayout);
		renderer->DestroyRenderPass(renderPass, commandPool, commandBuffers);
		renderer->Destroy();

		glfwDestroyWindow(window);
		glfwTerminate();
	}
}