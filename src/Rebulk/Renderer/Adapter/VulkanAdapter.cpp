#include "VulkanAdapter.h"

namespace Rbk
{
	VulkanAdapter::VulkanAdapter(GLFWwindow* window)
	{
		m_Renderer = new VulkanRenderer(window);
	}

	VulkanAdapter::~VulkanAdapter()
	{
		std::cout << "VulkanAdapter deleted." << std::endl;
	}

	void VulkanAdapter::Init()
	{
		m_RenderPass = m_Renderer->CreateRenderPass();
		m_DescriptorSetLayout = m_Renderer->CreateDescriptorSetLayout();
		m_PipelineLayout = m_Renderer->CreatePipelineLayout(m_DescriptorSetLayout);
		m_SwapChain = m_Renderer->CreateSwapChain(m_SwapChainImages);
		m_SwapChainImageViews = m_Renderer->CreateImageViews(m_SwapChainImages);
		m_SwapChainFramebuffers = m_Renderer->CreateFramebuffers(m_RenderPass, m_SwapChainImageViews);
		m_CommandPool = m_Renderer->CreateCommandPool();
		m_CommandBuffers = m_Renderer->AllocateCommandBuffers(m_CommandPool, (uint32_t)m_SwapChainFramebuffers.size());
		m_DescriptorPool = m_Renderer->CreateDescriptorPool(m_SwapChainImages);
		m_Semaphores = m_Renderer->CreateSyncObjects(m_SwapChainImages);
		m_UniformBuffers = m_Renderer->CreateUniformBuffers(m_SwapChainImageViews);
		m_DescriptorSets = m_Renderer->CreateDescriptorSets(m_DescriptorPool, m_DescriptorSetLayout, m_SwapChainImages, m_UniformBuffers);
	}

	void VulkanAdapter::SouldResizeSwapChain()
	{
		if (m_Renderer->SouldResizeSwapChain(m_SwapChain)) {
			VkSwapchainKHR old = m_SwapChain;
			m_SwapChain = m_Renderer->CreateSwapChain(m_SwapChainImages, old);
			m_Renderer->DestroySwapchain(m_Renderer->GetDevice(), old, m_SwapChainFramebuffers, m_SwapChainImageViews);
			m_Renderer->DestroySemaphores(m_Semaphores);
			m_Renderer->ResetCurrentFrameIndex();
			m_SwapChainImageViews = m_Renderer->CreateImageViews(m_SwapChainImages);
			m_SwapChainFramebuffers = m_Renderer->CreateFramebuffers(m_RenderPass, m_SwapChainImageViews);
			m_Semaphores = m_Renderer->CreateSyncObjects(m_SwapChainImages);
			m_Renderer->ResetCommandPool(m_CommandPool);
			m_CommandBuffers = m_Renderer->AllocateCommandBuffers(m_CommandPool, (uint32_t)m_SwapChainFramebuffers.size());
		}
	}

	void VulkanAdapter::AddMesh(Rbk::Mesh mesh)
	{			
		m_Meshes.mesh.vertices.insert(m_Meshes.mesh.vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
		m_Meshes.mesh.indices.insert(m_Meshes.mesh.indices.end(), mesh.indices.begin(), mesh.indices.end());
		m_Meshes.indexCount.emplace_back(mesh.indices.size());

		uint32_t vertexOffset = (m_Meshes.vertexOffset.size() > 0) ? m_Meshes.vertexOffset.back() + mesh.vertices.size() : mesh.vertices.size();
		m_Meshes.vertexOffset.emplace_back(vertexOffset);
		m_Meshes.count += 1;
	}

	void VulkanAdapter::AddShader(std::vector<char> vertexShaderCode, std::vector<char> fragShaderCode)
	{
		VkShaderModule vertexShaderModule = m_Renderer->CreateShaderModule(vertexShaderCode);
		VkShaderModule fragShaderModule = m_Renderer->CreateShaderModule(fragShaderCode);

		VkPipelineCache pipelineCache = 0;
		VkPipeline pipeline = m_Renderer->CreateGraphicsPipeline(m_RenderPass, m_PipelineLayout, pipelineCache, vertexShaderModule, fragShaderModule);

		VulkanShader shader;
		shader.vertex = vertexShaderModule;
		shader.frag = fragShaderModule;
		shader.pipeline = pipeline;

		m_Shaders.emplace_back(shader);
	}

	void VulkanAdapter::Clear()
	{
		m_Meshes.mesh.vertices.clear();
		m_Meshes.mesh.indices.clear();
		m_Meshes.indexCount.clear();
		m_Meshes.vertexOffset.clear();
	}

	void VulkanAdapter::Draw()
	{
		SouldResizeSwapChain();

		if (nullptr == m_Meshes.meshVBuffer.first) 
			m_Meshes.meshVBuffer = m_Renderer->CreateVertexBuffer(m_CommandPool, m_Meshes.mesh.vertices);

		if (nullptr == m_Meshes.meshIBuffer.first) 
			m_Meshes.meshIBuffer = m_Renderer->CreateIndexBuffer(m_CommandPool, m_Meshes.mesh.indices);
	
		Draw(m_Shaders[0], m_Meshes);
	}

	void VulkanAdapter::Draw(VulkanShader vShader, VulkanMesh vMesh)
	{
		for (size_t i = 0; i < m_CommandBuffers.size() - 1; i++) {

			m_ImageIndex = m_Renderer->AcquireNextImageKHR(m_SwapChain, m_Semaphores);
			m_Renderer->BeginCommandBuffer(m_CommandBuffers[m_ImageIndex]);

			VkImageMemoryBarrier renderBeginBarrier = m_Renderer->SetupImageMemoryBarrier(
				m_SwapChainImages[m_ImageIndex], 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			);
			m_Renderer->AddPipelineBarrier(
				m_CommandBuffers[m_ImageIndex], renderBeginBarrier, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT
			);

			m_Renderer->BeginRenderPass(m_RenderPass, m_CommandBuffers[m_ImageIndex], m_SwapChainFramebuffers[m_ImageIndex]);
			m_Renderer->SetViewPort(m_CommandBuffers[m_ImageIndex]);
			m_Renderer->SetScissor(m_CommandBuffers[m_ImageIndex]);
			m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], vShader.pipeline);			
			m_Renderer->Draw(
				m_CommandBuffers[m_ImageIndex],
				vMesh,
				m_UniformBuffers.first[m_ImageIndex], 
				m_DescriptorSets[m_ImageIndex], 
				m_PipelineLayout
			);
			m_Renderer->UpdateUniformBuffer(m_UniformBuffers.second[m_ImageIndex]);
			m_Renderer->EndRenderPass(m_CommandBuffers[m_ImageIndex]);

			VkImageMemoryBarrier renderEndBarrier = m_Renderer->SetupImageMemoryBarrier(
				m_SwapChainImages[m_ImageIndex], VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			);

			m_Renderer->AddPipelineBarrier(
				m_CommandBuffers[m_ImageIndex], renderEndBarrier, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT
			);
			m_Renderer->EndCommandBuffer(m_CommandBuffers[m_ImageIndex]);

			if (-1 != m_ImageIndex) {
				m_Renderer->QueueSubmit(m_ImageIndex, m_CommandBuffers[m_ImageIndex], m_Semaphores);
				m_Renderer->QueuePresent(m_ImageIndex, m_SwapChain, m_Semaphores);
			}
		}
		m_Renderer->WaitIdle();
		m_Renderer->ResetCommandPool(m_CommandPool);
	}

	void VulkanAdapter::Destroy()
	{
		//renderer->DestroySwapchain(renderer->GetDevice(), imguiSwapChain, imguiSwapChainFramebuffers, imguiSwapChainImageViews);
		//renderer->DestroyRenderPass(imguiRenderPass, imguiCommandPool, imguiCommandBuffers);
		m_Renderer->DestroySwapchain(m_Renderer->GetDevice(), m_SwapChain, m_SwapChainFramebuffers, m_SwapChainImageViews);
		m_Renderer->DestroySemaphores(m_Semaphores);

		for (auto vShader : m_Shaders) {
			m_Renderer->DestroyPipeline(vShader.pipeline);
		}

		m_Renderer->DestroyBuffer(m_Meshes.meshVBuffer.first);
		m_Renderer->DestroyDeviceMemory(m_Meshes.meshVBuffer.second);

		m_Renderer->DestroyBuffer(m_Meshes.meshIBuffer.first);
		m_Renderer->DestroyDeviceMemory(m_Meshes.meshIBuffer.second);

		for (auto& buffer : m_UniformBuffers.first) {
			m_Renderer->DestroyBuffer(buffer);
		}
		for (auto& deviceMemory : m_UniformBuffers.second) {
			m_Renderer->DestroyDeviceMemory(deviceMemory);
		}

		/*renderer->DestroyVertexBuffer(vertexBuffer.first, vertexBuffer.second);
		renderer->DestroyVertexBuffer(indexBuffer.first, indexBuffer.second);*/
		m_Renderer->DestroyPipelineData(m_PipelineLayout, m_DescriptorPool, m_DescriptorSetLayout);
		m_Renderer->DestroyRenderPass(m_RenderPass, m_CommandPool, m_CommandBuffers);
		m_Renderer->Destroy();
	}

	VImGuiInfo VulkanAdapter::GetVImGuiInfo()
	{
		ImGui_ImplVulkan_InitInfo info = {};

		info.Instance = m_Renderer->GetInstance();
		info.PhysicalDevice = m_Renderer->GetPhysicalDevice();
		info.Device = m_Renderer->GetDevice();
		info.QueueFamily = m_Renderer->GetQueueFamily();
		info.Queue = m_Renderer->GetGraphicsQueue();
		info.PipelineCache = nullptr;//to implement VkPipelineCache                 
		info.DescriptorPool = m_Renderer->CreateDescriptorPool(m_SwapChainImages);
		info.Subpass = 0;
		info.MinImageCount = 2;
		info.ImageCount = 2;
		info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		info.Allocator = nullptr;
		info.CheckVkResultFn = [](VkResult err) {
			std::cerr << "IMGUI VULKAN ERROR " + std::to_string(err) << std::endl;
		};


		VImGuiInfo vImGuiInfo;
		vImGuiInfo.info = info;
		vImGuiInfo.cmdBuffer = m_Renderer->AllocateCommandBuffers(m_CommandPool)[0];
		vImGuiInfo.pipeline = m_Pipeline;

		return vImGuiInfo;
	}
}