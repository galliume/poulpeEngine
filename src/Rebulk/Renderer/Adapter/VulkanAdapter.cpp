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
		m_SwapChain = m_Renderer->CreateSwapChain(m_SwapChainImages);
		m_CommandPool = m_Renderer->CreateCommandPool();
		//m_DescriptorPool = m_Renderer->CreateDescriptorPool(m_SwapChainImages);
		VulkanShaders m_Shaders;
	}

	void VulkanAdapter::SouldResizeSwapChain()
	{
		if (m_Renderer->SouldResizeSwapChain(m_SwapChain)) {
			m_Renderer->InitDetails();
			VkSwapchainKHR old = m_SwapChain;
			m_SwapChain = m_Renderer->CreateSwapChain(m_SwapChainImages, old);
			m_Renderer->DestroySwapchain(m_Renderer->GetDevice(), old, m_SwapChainFramebuffers, m_SwapChainImageViews);
			m_Renderer->DestroySemaphores(m_Semaphores);
			m_Renderer->ResetCurrentFrameIndex();
			m_SwapChainImageViews.resize(m_SwapChainImages.size());

			for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
				m_SwapChainImageViews[i] = m_Renderer->CreateImageView(m_SwapChainImages[i], m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
			}
			m_SwapChainFramebuffers = m_Renderer->CreateFramebuffers(m_RenderPass, m_SwapChainImageViews, m_Meshes.depthImageView);
			m_Semaphores = m_Renderer->CreateSyncObjects(m_SwapChainImages);
			m_Renderer->ResetCommandPool(m_CommandPool);
			m_CommandBuffers = m_Renderer->AllocateCommandBuffers(m_CommandPool, (uint32_t)m_SwapChainFramebuffers.size());
		}
	}

	void VulkanAdapter::AddMesh(Rbk::Mesh mesh, UniformBufferObject ubo)
	{			
		m_Meshes.mesh.vertices.insert(m_Meshes.mesh.vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
		m_Meshes.mesh.indices.insert(m_Meshes.mesh.indices.end(), mesh.indices.begin(), mesh.indices.end());
		m_Meshes.mesh.ubos.emplace_back(ubo);
		m_Meshes.indexCount.emplace_back(mesh.indices.size());

		uint32_t vertexOffset = (m_Meshes.vertexOffset.size() > 0) ? m_Meshes.vertexOffset.back() + mesh.vertices.size() : mesh.vertices.size();
		m_Meshes.vertexOffset.emplace_back(vertexOffset);
		m_Meshes.count += 1;
	}

	void VulkanAdapter::AddTexture(Rbk::Mesh& mesh, const char* texturePath)
	{
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;

		VkCommandBuffer commandBuffer = m_Renderer->AllocateCommandBuffers(m_CommandPool)[0];
		m_Renderer->BeginCommandBuffer(commandBuffer);
		m_Renderer->CreateTextureImage(commandBuffer, texturePath, textureImage, textureImageMemory);

		VkImageView textureImageView = m_Renderer->CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
		VkSampler textureSampler = m_Renderer->CreateTextureSampler(textureImageView);

		m_Meshes.textureImages.emplace_back(textureImage);
		m_Meshes.textureImageMemorys.emplace_back(textureImageMemory);
		m_Meshes.textureImageViews.emplace_back(textureImageView);
		m_Meshes.samplers.emplace_back(textureSampler);
	}

	void VulkanAdapter::AddShader(std::string name, std::vector<char> vertexShaderCode, std::vector<char> fragShaderCode)
	{
		VkShaderModule vertexShaderModule = m_Renderer->CreateShaderModule(vertexShaderCode);
		VkShaderModule fragShaderModule = m_Renderer->CreateShaderModule(fragShaderCode);
		std::array<VkShaderModule, 2> module = { vertexShaderModule, fragShaderModule };
		m_Shaders.shaders.emplace(name, module);
	}

	void VulkanAdapter::Clear()
	{
		m_Meshes.mesh.vertices.clear();
		m_Meshes.mesh.indices.clear();
		m_Meshes.indexCount.clear();
		m_Meshes.vertexOffset.clear();
	}

	void VulkanAdapter::PrepareDraw()
	{
		if (m_IsPrepared) return;

		for (int i = 0; i < m_Meshes.count; i++) {
			std::pair<VkBuffer, VkDeviceMemory> uniformBuffer = m_Renderer->CreateUniformBuffers(m_Meshes.count);
			m_Meshes.uniformBuffers.emplace_back(uniformBuffer);
		}

		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		m_Renderer->CreateImage(m_Renderer->GetSwapChainExtent().width, m_Renderer->GetSwapChainExtent().height, m_Renderer->FindDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
		VkImageView depthImageView = m_Renderer->CreateImageView(depthImage, m_Renderer->FindDepthFormat(), VK_IMAGE_ASPECT_DEPTH_BIT);

		m_Meshes.depthImage = depthImage;
		m_Meshes.depthImageView = depthImageView;
		m_Meshes.depthImageMemory = depthImageMemory;

		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
			m_SwapChainImageViews[i] = m_Renderer->CreateImageView(m_SwapChainImages[i], m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
		}
		m_SwapChainFramebuffers = m_Renderer->CreateFramebuffers(m_RenderPass, m_SwapChainImageViews, depthImageView);
		m_CommandBuffers = m_Renderer->AllocateCommandBuffers(m_CommandPool, (uint32_t)m_SwapChainFramebuffers.size());
		m_Semaphores = m_Renderer->CreateSyncObjects(m_SwapChainImages);

		if (nullptr == m_Meshes.meshVBuffer.first) 
			m_Meshes.meshVBuffer = m_Renderer->CreateVertexBuffer(m_CommandPool, m_Meshes.mesh.vertices);

		if (nullptr == m_Meshes.meshIBuffer.first) 
			m_Meshes.meshIBuffer = m_Renderer->CreateIndexBuffer(m_CommandPool, m_Meshes.mesh.indices);
	
		if (0 == m_Pipelines.size()) {
			VulkanPipeline vPipeline;
			vPipeline.pipelineCache = 0;
			vPipeline.descriptorPool = m_Renderer->CreateDescriptorPool(m_SwapChainImages);			
			vPipeline.descriptorSetLayouts.emplace_back(m_Renderer->CreateDescriptorSetLayout(m_Meshes.count));
			vPipeline.descriptorSets.emplace_back(m_Renderer->CreateDescriptorSets(vPipeline.descriptorPool, m_SwapChainImages, vPipeline.descriptorSetLayouts));		
			vPipeline.pipelineLayout = m_Renderer->CreatePipelineLayout(vPipeline.descriptorSets, vPipeline.descriptorSetLayouts);			
			vPipeline.graphicsPipeline = m_Renderer->CreateGraphicsPipeline(m_RenderPass, vPipeline, m_Shaders);

			m_Pipelines.emplace_back(vPipeline);
		}

		m_IsPrepared = true;
	}

	void VulkanAdapter::Draw()
	{
		if (!m_IsPrepared) {
			throw std::runtime_error("Draw is not prepared. Forgot to calle Prepare() ?");
		}

		SouldResizeSwapChain();

		for (size_t i = 0; i < m_SwapChainImages.size(); i++) {

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
			m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], m_Pipelines[0].graphicsPipeline);
			m_Renderer->Draw(m_CommandBuffers[m_ImageIndex], m_Meshes, m_Pipelines[0]);
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
			m_Renderer->WaitIdle();
		}
		m_Renderer->ResetCommandPool(m_CommandPool);
	}

	void VulkanAdapter::Destroy()
	{
		//@todo refactor all the destroy system...
		m_Renderer->DestroySwapchain(m_Renderer->GetDevice(), m_SwapChain, m_SwapChainFramebuffers, m_SwapChainImageViews);
		m_Renderer->DestroySemaphores(m_Semaphores);

	
		vkDestroyImageView(m_Renderer->GetDevice(), m_Meshes.depthImageView, nullptr);
		m_Renderer->DestroyDeviceMemory(m_Meshes.depthImageMemory);
		vkDestroyImage(m_Renderer->GetDevice(), m_Meshes.depthImage, nullptr);

		for (int i = 0; i < m_Meshes.uniformBuffers.size(); i++) {
			m_Renderer->DestroyBuffer(m_Meshes.uniformBuffers[i].first);
			m_Renderer->DestroyDeviceMemory(m_Meshes.uniformBuffers[i].second);
		}

		for (auto sampler : m_Meshes.samplers) {
			vkDestroySampler(m_Renderer->GetDevice(), sampler, nullptr);
		}
		for (auto textureView : m_Meshes.textureImageViews) {
			vkDestroyImageView(m_Renderer->GetDevice(), textureView, nullptr);
		}
		for (auto texture : m_Meshes.textureImages) {
			vkDestroyImage(m_Renderer->GetDevice(), texture, nullptr);
		}
		for (auto textureMemory : m_Meshes.textureImageMemorys) {
			m_Renderer->DestroyDeviceMemory(textureMemory);
		}
		
		for (auto pipeline : m_Pipelines) {
			m_Renderer->DestroyPipeline(pipeline.graphicsPipeline);
			vkDestroyDescriptorPool(m_Renderer->GetDevice(), pipeline.descriptorPool, nullptr);
			vkDestroyPipelineLayout(m_Renderer->GetDevice(), pipeline.pipelineLayout, nullptr);

			for (auto descriptorSetLayout : pipeline.descriptorSetLayouts) {
				vkDestroyDescriptorSetLayout(m_Renderer->GetDevice(), descriptorSetLayout, nullptr);
			}
		}

		for (auto shader : m_Shaders.shaders) {
			vkDestroyShaderModule(m_Renderer->GetDevice(), shader.second[0], nullptr);
			vkDestroyShaderModule(m_Renderer->GetDevice(), shader.second[1], nullptr);
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
		vImGuiInfo.pipeline = m_Pipelines[0].graphicsPipeline;

		return vImGuiInfo;
	}
}