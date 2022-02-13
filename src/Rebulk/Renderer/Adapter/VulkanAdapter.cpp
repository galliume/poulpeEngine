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
		m_RenderPass = m_Renderer->CreateRenderPass(m_Renderer->GetMsaaSamples());
		m_SwapChain = m_Renderer->CreateSwapChain(m_SwapChainImages);
		m_CommandPool = m_Renderer->CreateCommandPool();
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
			std::vector<VkImageView> depthImageViews;
			std::vector<VkImageView> colorImageViews;

			for (auto&& [textName, tex] : m_Textures) {
				m_Renderer->CreateImage(tex.texWidth, tex.texHeight, tex.mipLevels, VK_SAMPLE_COUNT_1_BIT, m_Renderer->FindDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, tex.depthImage, tex.depthImageMemory);
				depthImageViews.emplace_back(tex.depthImageView);
				colorImageViews.emplace_back(tex.colorImageView);
			}

			m_SwapChainFramebuffers = m_Renderer->CreateFramebuffers(m_RenderPass, m_SwapChainImageViews, depthImageViews, colorImageViews);
			m_Semaphores = m_Renderer->CreateSyncObjects(m_SwapChainImages);
			m_Renderer->ResetCommandPool(m_CommandPool);
			m_CommandBuffers = m_Renderer->AllocateCommandBuffers(m_CommandPool, (uint32_t)m_SwapChainFramebuffers.size());
		}
	}

	void VulkanAdapter::AddCamera(Camera* camera)
	{
		m_Camera = camera;
	}

	void VulkanAdapter::AddMesh(Rbk::Mesh mesh, const char* textureName, glm::vec3 pos)
	{			
		glm::mat4 view = glm::mat4(1.0f);

		UniformBufferObject ubo;
		ubo.model = glm::mat4(1.0f);
		ubo.model = glm::translate(ubo.model, pos);
		ubo.model = glm::scale(ubo.model, glm::vec3(0.1f, 0.1f, 0.1f));
		ubo.view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

		glm::mat4 projection;
		ubo.proj = glm::perspective(glm::radians(45.0f), 800 / (float) 600, 0.1f, 100.0f);
		ubo.proj[1][1] *= -1;

		m_Meshes.mesh.textureNames.emplace(m_Meshes.count, textureName);
		m_Meshes.mesh.vertices.insert(m_Meshes.mesh.vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
		m_Meshes.mesh.indices.insert(m_Meshes.mesh.indices.end(), mesh.indices.begin(), mesh.indices.end());
		m_Meshes.mesh.ubos.emplace_back(ubo);
		m_Meshes.indexCount.emplace_back(mesh.indices.size());

		uint32_t vertexOffset = (m_Meshes.vertexOffset.size() > 0) ? m_Meshes.vertexOffset.back() + mesh.vertices.size() : mesh.vertices.size();
		m_Meshes.vertexOffset.emplace_back(vertexOffset);
		m_Meshes.count += 1;
	}

	void VulkanAdapter::AddTexture(const char* name, const char* path)
	{
		if (0 != m_Textures.count(name)) {
			std::cout << "Texture " << name << " already imported" << std::endl;
			return;
		}

		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels) {
			Rbk::Log::GetLogger()->warn("failed to load texture image %s", name);
			return;
		}

		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

		VkCommandBuffer commandBuffer = m_Renderer->AllocateCommandBuffers(m_CommandPool)[0];
		m_Renderer->BeginCommandBuffer(commandBuffer);
		m_Renderer->CreateTextureImage(commandBuffer, pixels, texWidth, texHeight, mipLevels, textureImage, textureImageMemory, VK_FORMAT_R8G8B8A8_SRGB);

		VkImageView textureImageView = m_Renderer->CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);
		VkSampler textureSampler = m_Renderer->CreateTextureSampler(mipLevels);

		VkDeviceMemory colorImageMemory;
		VkImage colorImage;
		m_Renderer->CreateImage(m_Renderer->GetSwapChainExtent().width, m_Renderer->GetSwapChainExtent().height, 1, m_Renderer->GetMsaaSamples(), m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
		VkImageView colorImageView = m_Renderer->CreateImageView(colorImage, m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT, 1);

		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		m_Renderer->CreateImage(m_Renderer->GetSwapChainExtent().width, m_Renderer->GetSwapChainExtent().height, 1, m_Renderer->GetMsaaSamples(), m_Renderer->FindDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
		VkImageView depthImageView = m_Renderer->CreateImageView(depthImage, m_Renderer->FindDepthFormat(), 1, VK_IMAGE_ASPECT_DEPTH_BIT);

		VulkanTexture vTexture;
		vTexture.name = name;
		vTexture.textureImage = textureImage;
		vTexture.textureImageMemory = textureImageMemory;
		vTexture.textureImageView = textureImageView;
		vTexture.sampler = textureSampler;
		vTexture.mipLevels = mipLevels;
		vTexture.texWidth = texWidth;
		vTexture.texHeight = texHeight;
		vTexture.texChannels = texChannels;
		vTexture.colorImageView = colorImageView;
		vTexture.colorImage = colorImage;
		vTexture.colorImageMemory = colorImageMemory;
		vTexture.depthImage = depthImage;
		vTexture.depthImageView = depthImageView;
		vTexture.depthImageMemory = depthImageMemory;

		m_Textures.emplace(name, vTexture);
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

		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
			m_SwapChainImageViews[i] = m_Renderer->CreateImageView(m_SwapChainImages[i], m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
		}

		std::vector<VkImageView> depthImageViews;
		std::vector<VkImageView> colorImageViews;
		VkImage depthImage;
		VkDeviceMemory depthImageMemory;

		for (auto&& [textName, tex]: m_Textures) {
			depthImageViews.emplace_back(tex.depthImageView);
			colorImageViews.emplace_back(tex.colorImageView);
		}

		m_SwapChainFramebuffers = m_Renderer->CreateFramebuffers(m_RenderPass, m_SwapChainImageViews, depthImageViews, colorImageViews);

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
			vPipeline.graphicsPipeline.emplace_back(m_Renderer->CreateGraphicsPipeline(m_RenderPass, vPipeline, m_Shaders));
			m_Pipelines.emplace_back(vPipeline);

			VulkanPipeline vPipelineWireFramed;
			vPipelineWireFramed.pipelineCache = 0;
			vPipelineWireFramed.descriptorPool = m_Renderer->CreateDescriptorPool(m_SwapChainImages);
			vPipelineWireFramed.descriptorSetLayouts.emplace_back(m_Renderer->CreateDescriptorSetLayout(m_Meshes.count));
			vPipelineWireFramed.descriptorSets.emplace_back(m_Renderer->CreateDescriptorSets(vPipeline.descriptorPool, m_SwapChainImages, vPipeline.descriptorSetLayouts));
			vPipelineWireFramed.pipelineLayout = m_Renderer->CreatePipelineLayout(vPipeline.descriptorSets, vPipeline.descriptorSetLayouts);
			vPipelineWireFramed.graphicsPipeline.emplace_back(m_Renderer->CreateGraphicsPipeline(m_RenderPass, vPipeline, m_Shaders, true));

			m_Pipelines.emplace_back(vPipelineWireFramed);
		}

		m_IsPrepared = true;
	}

	void VulkanAdapter::UpdatePositions()
	{
		for (auto& ubo : m_Meshes.mesh.ubos) {
			ubo.view = m_Camera->LookAt();
			
			if (m_MakeSpin) {		
				ubo.model = glm::rotate(ubo.model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			}
		}
	}

	void VulkanAdapter::Draw()
	{
		if (!m_IsPrepared) {
			throw std::runtime_error("Draw is not prepared. Forgot to calle Prepare() ?");
		}

		SouldResizeSwapChain();
		UpdatePositions();

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

			VulkanPipeline ppline = (!m_WireFrameModeOn) ? m_Pipelines[0] : m_Pipelines[1];

			m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], ppline.graphicsPipeline[0]);
			m_Renderer->Draw(m_CommandBuffers[m_ImageIndex], m_Meshes, m_Textures, ppline);
			m_Renderer->EndRenderPass(m_CommandBuffers[m_ImageIndex]);
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
	
		for (int i = 0; i < m_Meshes.uniformBuffers.size(); i++) {
			m_Renderer->DestroyBuffer(m_Meshes.uniformBuffers[i].first);
			m_Renderer->DestroyDeviceMemory(m_Meshes.uniformBuffers[i].second);
		}

		for (auto item : m_Textures) {
			vkDestroySampler(m_Renderer->GetDevice(), item.second.sampler, nullptr);

			vkDestroyImage(m_Renderer->GetDevice(), item.second.textureImage, nullptr);
			m_Renderer->DestroyDeviceMemory(item.second.textureImageMemory);
			vkDestroyImageView(m_Renderer->GetDevice(), item.second.textureImageView, nullptr);

			vkDestroyImage(m_Renderer->GetDevice(), item.second.depthImage, nullptr);
			m_Renderer->DestroyDeviceMemory(item.second.depthImageMemory);
			vkDestroyImageView(m_Renderer->GetDevice(), item.second.depthImageView, nullptr);

			vkDestroyImage(m_Renderer->GetDevice(), item.second.colorImage, nullptr);
			m_Renderer->DestroyDeviceMemory(item.second.colorImageMemory);
			vkDestroyImageView(m_Renderer->GetDevice(), item.second.colorImageView, nullptr);
		}
		
		for (auto pipeline : m_Pipelines) {
			for (auto gp : pipeline.graphicsPipeline) {
				m_Renderer->DestroyPipeline(gp);
			}
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

	void VulkanAdapter::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
	{
		VkCommandBuffer cmd = m_Renderer->AllocateCommandBuffers(m_CommandPool)[0];
		m_Renderer->BeginCommandBuffer(cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		function(cmd);
		m_Renderer->EndCommandBuffer(cmd);
		m_Renderer->QueueSubmit(cmd);
		m_Renderer->WaitForFence();
	}


	VkRenderPass* VulkanAdapter::CreateImGuiRenderPass()
	{
		VkRenderPass renderPass;

		VkAttachmentDescription attachment = {};
		attachment.format = VK_FORMAT_B8G8R8A8_UNORM;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAttachmentReference color_attachment = {};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment;
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 1;
		info.pDependencies = &dependency;

		vkCreateRenderPass(m_Renderer->GetDevice(), &info, nullptr, &renderPass);

		return &renderPass;
	}

	VImGuiInfo VulkanAdapter::GetVImGuiInfo()
	{
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		VkDescriptorPool imguiPool;
		vkCreateDescriptorPool(m_Renderer->GetDevice(), &pool_info, nullptr, &imguiPool);

		ImGui_ImplVulkan_InitInfo info = {};

		info.Instance = m_Renderer->GetInstance();
		info.PhysicalDevice = m_Renderer->GetPhysicalDevice();
		info.Device = m_Renderer->GetDevice();
		info.QueueFamily = m_Renderer->GetQueueFamily();
		info.Queue = m_Renderer->GetGraphicsQueue();
		info.PipelineCache = nullptr;//to implement VkPipelineCache                 
		info.DescriptorPool = imguiPool;
		info.Subpass = 0;
		info.MinImageCount = 3;
		info.ImageCount = 3;
		info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		info.Allocator = nullptr;
		info.CheckVkResultFn = [](VkResult err) {
			if (0 == err) return;
			Rbk::Log::GetLogger()->warn("ImGui error {}", err);
		};


		VImGuiInfo vImGuiInfo;
		vImGuiInfo.info = info;
		vImGuiInfo.rdrPass = CreateImGuiRenderPass();
		vImGuiInfo.cmdBuffer = m_Renderer->AllocateCommandBuffers(m_CommandPool)[0];
		//vImGuiInfo.pipeline = m_Pipelines[0].graphicsPipeline;

		return vImGuiInfo;
	}
}