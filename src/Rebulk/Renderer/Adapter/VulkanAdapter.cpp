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

	void VulkanAdapter::AddTextureManager(TextureManager* textureManager)
	{
		m_TextureManager = textureManager;
	}


	void VulkanAdapter::AddShaderManager(ShaderManager* shaderManager)
	{
		m_ShaderManager = shaderManager;
	}

	void VulkanAdapter::AddCamera(Camera* camera)
	{
		m_Camera = camera;
	}

	void VulkanAdapter::AddMeshManager(MeshManager* meshManager)
	{
		m_MeshManager = meshManager;
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

			for (auto&& [textName, tex] : m_TextureManager->GetTextures()) {
				m_Renderer->CreateImage(tex.width, tex.height, tex.mipLevels, VK_SAMPLE_COUNT_1_BIT, m_Renderer->FindDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, tex.depthImage, tex.depthImageMemory);
				depthImageViews.emplace_back(tex.depthImageView);
				colorImageViews.emplace_back(tex.colorImageView);
			}

			m_SwapChainFramebuffers = m_Renderer->CreateFramebuffers(m_RenderPass, m_SwapChainImageViews, depthImageViews, colorImageViews);
			m_Semaphores = m_Renderer->CreateSyncObjects(m_SwapChainImages);
			m_Renderer->ResetCommandPool(m_CommandPool);
			m_CommandBuffers = m_Renderer->AllocateCommandBuffers(m_CommandPool, (uint32_t)m_SwapChainFramebuffers.size());
		}
	}

	void VulkanAdapter::PrepareWorld()
	{
		VulkanMesh* worldMeshes = m_MeshManager->GetWorld();

		for (auto item : worldMeshes->mesh.meshNames) {
			worldMeshes->totalInstances += item.second;
		}

		worldMeshes->maxUniformBufferRange = m_Renderer->GetDeviceProperties().limits.maxUniformBufferRange;
		worldMeshes->uniformBufferChunkSize = worldMeshes->maxUniformBufferRange / sizeof(UniformBufferObject);
		worldMeshes->uniformBuffersCount = std::ceil(worldMeshes->totalInstances / (float)worldMeshes->uniformBufferChunkSize);

		for (int i = 0; i < worldMeshes->uniformBuffersCount; i++) {
			std::pair<VkBuffer, VkDeviceMemory> uniformBuffer = m_Renderer->CreateUniformBuffers(worldMeshes->uniformBufferChunkSize);
			worldMeshes->uniformBuffers.emplace_back(uniformBuffer);
		}

		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
			m_SwapChainImageViews[i] = m_Renderer->CreateImageView(m_SwapChainImages[i], m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
		}

		std::vector<VkImageView> depthImageViews;
		std::vector<VkImageView> colorImageViews;
		VkImage depthImage;
		VkDeviceMemory depthImageMemory;

		for (auto&& [textName, tex] : m_TextureManager->GetTextures()) {
			depthImageViews.emplace_back(tex.depthImageView);
			colorImageViews.emplace_back(tex.colorImageView);
		}

		m_SwapChainFramebuffers = m_Renderer->CreateFramebuffers(m_RenderPass, m_SwapChainImageViews, depthImageViews, colorImageViews);

		m_CommandBuffers = m_Renderer->AllocateCommandBuffers(m_CommandPool, (uint32_t)m_SwapChainFramebuffers.size());
		m_Semaphores = m_Renderer->CreateSyncObjects(m_SwapChainImages);

		if (nullptr == worldMeshes->meshVBuffer.first)
			worldMeshes->meshVBuffer = m_Renderer->CreateVertexBuffer(m_CommandPool, worldMeshes->mesh.vertices);

		if (nullptr == worldMeshes->meshIBuffer.first)
			worldMeshes->meshIBuffer = m_Renderer->CreateIndexBuffer(m_CommandPool, worldMeshes->mesh.indices);

		//prepare for one mesh
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = worldMeshes->totalInstances;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = worldMeshes->totalInstances;

		VkDescriptorPool descriptorPool = m_Renderer->CreateDescriptorPool(poolSizes, worldMeshes->totalInstances * 4);

		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };

		VkDescriptorSetLayout desriptorSetLayout = m_Renderer->CreateDescriptorSetLayout(
			bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
		);

		std::vector<VkDescriptorSet>listDescriptorSet;

		//1 descriptor set per swapChainImage
		for (int i = 0; i < m_SwapChainImages.size(); i++) {
			listDescriptorSet.emplace_back(m_Renderer->CreateDescriptorSets(descriptorPool, { desriptorSetLayout }, 1));
		}

		//VkPushConstantRange pushConstantRange = {};
		/*pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(int);
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;*/

		VkPipelineLayout pipelineLayout = m_Renderer->CreatePipelineLayout(listDescriptorSet, { desriptorSetLayout });

		VulkanPipeline vPipeline;
		vPipeline.descriptorPool = descriptorPool;
		vPipeline.descriptorSets = listDescriptorSet;
		vPipeline.descriptorSetLayouts = { desriptorSetLayout };
		vPipeline.pipelineLayout = pipelineLayout;
		vPipeline.pipelineCache = 0;

		std::vector<VkPipelineShaderStageCreateInfo>shadersStageInfos;

		for (auto& shader : m_ShaderManager->GetShaders().shaders) {
			VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = shader.second[0];
			vertShaderStageInfo.pName = shader.first;
			shadersStageInfos.emplace_back(vertShaderStageInfo);

			VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = shader.second[1];
			fragShaderStageInfo.pName = shader.first;
			shadersStageInfos.emplace_back(fragShaderStageInfo);
		}

		vPipeline.graphicsPipeline.emplace_back(m_Renderer->CreateGraphicsPipeline(m_RenderPass, vPipeline, shadersStageInfos));

		m_Pipelines.emplace_back(vPipeline);

		for (int i = 0; i < worldMeshes->uniformBuffersCount; i++) {
			m_Renderer->UpdateDescriptorSets(*worldMeshes, worldMeshes->uniformBuffers[i], m_TextureManager->GetTextures(), vPipeline.descriptorSets[i]);
		}

		UpdateWorldPositions();
	}

	void VulkanAdapter::PrepareDraw()
	{
		if (m_IsPrepared) return;
 
		//for (auto item : m_MeshManager->GetMeshes()->mesh.meshNames) {
		//	m_MeshManager->GetMeshes()->totalInstances += item.second;
		//}

		//m_MeshManager->GetMeshes()->maxUniformBufferRange = m_Renderer->GetDeviceProperties().limits.maxUniformBufferRange;
		//m_MeshManager->GetMeshes()->uniformBufferChunkSize = m_MeshManager->GetMeshes()->maxUniformBufferRange / sizeof(UniformBufferObject);
		//m_MeshManager->GetMeshes()->uniformBuffersCount = std::ceil(m_MeshManager->GetMeshes()->totalInstances / (float)m_MeshManager->GetMeshes()->uniformBufferChunkSize);

		//for (int i = 0; i < m_MeshManager->GetMeshes()->uniformBuffersCount; i++) {
		//	std::pair<VkBuffer, VkDeviceMemory> uniformBuffer = m_Renderer->CreateUniformBuffers(m_MeshManager->GetMeshes()->uniformBufferChunkSize);
		//	m_MeshManager->GetMeshes()->uniformBuffers.emplace_back(uniformBuffer);
		//}
		//
		//m_SwapChainImageViews.resize(m_SwapChainImages.size());

		//for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
		//	m_SwapChainImageViews[i] = m_Renderer->CreateImageView(m_SwapChainImages[i], m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
		//}

		//std::vector<VkImageView> depthImageViews;
		//std::vector<VkImageView> colorImageViews;
		//VkImage depthImage;
		//VkDeviceMemory depthImageMemory;

		//for (auto&& [textName, tex]: m_TextureManager->GetTextures()) {
		//	depthImageViews.emplace_back(tex.depthImageView);
		//	colorImageViews.emplace_back(tex.colorImageView);
		//}

		//m_SwapChainFramebuffers = m_Renderer->CreateFramebuffers(m_RenderPass, m_SwapChainImageViews, depthImageViews, colorImageViews);

		//m_CommandBuffers = m_Renderer->AllocateCommandBuffers(m_CommandPool, (uint32_t)m_SwapChainFramebuffers.size());
		//m_Semaphores = m_Renderer->CreateSyncObjects(m_SwapChainImages);

		//if (nullptr == m_MeshManager->GetMeshes()->meshVBuffer.first)
		//	m_MeshManager->GetMeshes()->meshVBuffer = m_Renderer->CreateVertexBuffer(m_CommandPool, m_MeshManager->GetMeshes()->mesh.vertices);

		//if (nullptr == m_MeshManager->GetMeshes()->meshIBuffer.first)
		//	m_MeshManager->GetMeshes()->meshIBuffer = m_Renderer->CreateIndexBuffer(m_CommandPool, m_MeshManager->GetMeshes()->mesh.indices);
	
		//if (0 == m_Pipelines.size()) {
		//	VulkanPipeline vPipeline;
		//	vPipeline.pipelineCache = 0;
		//	vPipeline.descriptorPool = m_Renderer->CreateDescriptorPool(m_MeshManager->GetMeshes()->uniformBuffersCount);

		//	vPipeline.descriptorSetLayouts.emplace_back(m_Renderer->CreateDescriptorSetLayout());
		//	for (int i = 0; i < m_MeshManager->GetMeshes()->uniformBuffersCount; i++) {
		//		vPipeline.descriptorSets.emplace_back(m_Renderer->CreateDescriptorSets(vPipeline.descriptorPool, vPipeline.descriptorSetLayouts));
		//	}

		//	vPipeline.pipelineLayout = m_Renderer->CreatePipelineLayout(vPipeline.descriptorSets, vPipeline.descriptorSetLayouts);			
		//	vPipeline.graphicsPipeline.emplace_back(m_Renderer->CreateGraphicsPipeline(m_RenderPass, vPipeline, m_ShaderManager->GetShaders()));
		//	m_Pipelines.emplace_back(vPipeline);

		//	VulkanPipeline vPipelineWireFramed;
		//	vPipelineWireFramed.pipelineCache = 0;
		//	vPipelineWireFramed.descriptorPool = m_Renderer->CreateDescriptorPool(m_MeshManager->GetMeshes()->uniformBuffersCount);

		//	vPipelineWireFramed.descriptorSetLayouts.emplace_back(m_Renderer->CreateDescriptorSetLayout());
		//	for (int i = 0; i < m_MeshManager->GetMeshes()->uniformBuffersCount; i++) {
		//		vPipelineWireFramed.descriptorSets.emplace_back(m_Renderer->CreateDescriptorSets(vPipelineWireFramed.descriptorPool, vPipelineWireFramed.descriptorSetLayouts));
		//	}

		//	vPipelineWireFramed.pipelineLayout = m_Renderer->CreatePipelineLayout(vPipelineWireFramed.descriptorSets, vPipelineWireFramed.descriptorSetLayouts);
		//	vPipelineWireFramed.graphicsPipeline.emplace_back(m_Renderer->CreateGraphicsPipeline(m_RenderPass, vPipelineWireFramed, m_ShaderManager->GetShaders(), true));

		//	m_Pipelines.emplace_back(vPipelineWireFramed);

		//	for (int i = 0; i < m_MeshManager->GetMeshes()->uniformBuffersCount; i++) {
		//		m_Renderer->UpdateDescriptorSets(*m_MeshManager->GetMeshes(), m_MeshManager->GetMeshes()->uniformBuffers[i], m_TextureManager->GetTextures(), vPipeline.descriptorSets[i]);
		//		m_Renderer->UpdateDescriptorSets(*m_MeshManager->GetMeshes(), m_MeshManager->GetMeshes()->uniformBuffers[i], m_TextureManager->GetTextures(), vPipelineWireFramed.descriptorSets[i]);
		//	}
		//}


		m_IsPrepared = true;
	}

	void VulkanAdapter::UpdateWorldPositions()
	{
		VulkanMesh* worldMeshes = m_MeshManager->GetWorld();

		int32_t uboCount = 1, uboIndex = 0;
		std::vector<UniformBufferObject> chunk;
		int32_t beginRange, endRange = 0;
		int32_t nextChunk = worldMeshes->totalInstances - worldMeshes->uniformBufferChunkSize;

		if (nextChunk < 0) nextChunk = 0;

		for (int i = worldMeshes->totalInstances - 1; i >= 0; i--) {

			worldMeshes->mesh.ubos[i].view = m_Camera->LookAt();

			if (i == nextChunk) {

				nextChunk -= worldMeshes->uniformBufferChunkSize;

				if (nextChunk < 0) nextChunk = 0;
				endRange = worldMeshes->uniformBufferChunkSize * uboCount;
				beginRange = endRange - worldMeshes->uniformBufferChunkSize;

				if (endRange > worldMeshes->totalInstances) endRange = worldMeshes->totalInstances;

				chunk = { worldMeshes->mesh.ubos.rbegin() + beginRange, worldMeshes->mesh.ubos.rbegin() + endRange };

				m_Renderer->UpdateUniformBuffer(worldMeshes->uniformBuffers[uboIndex], chunk, chunk.size());
				worldMeshes->uniformUBOCount.emplace_back(chunk.size());
				uboIndex += 1;
				uboCount += 1;
			}
		}
	}

	void VulkanAdapter::Draw()
	{
		if (!m_IsPrepared) {
			throw std::runtime_error("Draw is not prepared. Forgot to calle Prepare() ?");
		}

		SouldResizeSwapChain();
		
		VulkanPipeline ppline = m_Pipelines[0];

		VkBuffer vertexBuffers[] = { m_MeshManager->GetWorld()->meshVBuffer.first};
		VkDeviceSize offsets[] = { 0 };

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
			m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], ppline.graphicsPipeline[0]);
			m_Renderer->Draw(m_CommandBuffers[m_ImageIndex], *m_MeshManager->GetWorld(), ppline);
			m_Renderer->EndRenderPass(m_CommandBuffers[m_ImageIndex]);
			m_Renderer->EndCommandBuffer(m_CommandBuffers[m_ImageIndex]);

			if (-1 != m_ImageIndex) {
				m_Renderer->QueueSubmit(m_ImageIndex, m_CommandBuffers[m_ImageIndex], m_Semaphores);
				m_Renderer->QueuePresent(m_ImageIndex, m_SwapChain, m_Semaphores);
			}

			//m_Renderer->WaitIdle();
		}
		//m_Renderer->ResetCommandPool(m_CommandPool);
	}

	void VulkanAdapter::Destroy()
	{
		//@todo refactor all the destroy system...
		m_Renderer->DestroySwapchain(m_Renderer->GetDevice(), m_SwapChain, m_SwapChainFramebuffers, m_SwapChainImageViews);
		m_Renderer->DestroySemaphores(m_Semaphores);
	
		for (int i = 0; i < m_MeshManager->GetWorld()->uniformBuffers.size(); i++) {
			m_Renderer->DestroyBuffer(m_MeshManager->GetWorld()->uniformBuffers[i].first);
			m_Renderer->DestroyDeviceMemory(m_MeshManager->GetWorld()->uniformBuffers[i].second);
		}

		for (auto item : m_TextureManager->GetTextures()) {
			vkDestroySampler(m_Renderer->GetDevice(), item.second.sampler, nullptr);

			vkDestroyImage(m_Renderer->GetDevice(), item.second.image, nullptr);
			m_Renderer->DestroyDeviceMemory(item.second.imageMemory);
			vkDestroyImageView(m_Renderer->GetDevice(), item.second.imageView, nullptr);

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

		for (auto shader : m_ShaderManager->GetShaders().shaders) {
			vkDestroyShaderModule(m_Renderer->GetDevice(), shader.second[0], nullptr);
			vkDestroyShaderModule(m_Renderer->GetDevice(), shader.second[1], nullptr);
		}

		m_Renderer->DestroyBuffer(m_MeshManager->GetWorld()->meshVBuffer.first);
		m_Renderer->DestroyDeviceMemory(m_MeshManager->GetWorld()->meshVBuffer.second);

		m_Renderer->DestroyBuffer(m_MeshManager->GetWorld()->meshIBuffer.first);
		m_Renderer->DestroyDeviceMemory(m_MeshManager->GetWorld()->meshIBuffer.second);

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