#include "rebulkpch.h"
#include "VulkanAdapter.h"
#include <volk.h>

namespace Rbk
{
    VulkanAdapter::VulkanAdapter(Window* window)
    {
        m_Window = window;
        m_Renderer = new VulkanRenderer(window->Get());
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

        if (Rbk::m_FramebufferResized == true) {

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

            Rbk::m_FramebufferResized = false;
        }
    }

    void VulkanAdapter::PrepareWorld()
    {
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

        std::vector<Mesh>* worldMeshes = m_MeshManager->GetWorldMeshes();
        std::map<const char*, std::array<uint32_t, 2>> worldMeshesLoaded = m_MeshManager->GetWoldMeshesLoaded();

        uint32_t maxUniformBufferRange = 0;
        uint32_t uniformBufferChunkSize = 0;
        uint32_t uniformBuffersCount = 0;

        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = 1000;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = 1000;

        VkDescriptorPool descriptorPool = m_Renderer->CreateDescriptorPool(poolSizes, 1000);

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

        for (Mesh& mesh : *worldMeshes) {
            uint32_t totalInstances = worldMeshesLoaded[mesh.name][0];

            maxUniformBufferRange = m_Renderer->GetDeviceProperties().limits.maxUniformBufferRange;
            uniformBufferChunkSize = maxUniformBufferRange / sizeof(UniformBufferObject);
            uniformBuffersCount = std::ceil(totalInstances / (float)uniformBufferChunkSize);


            for (int i = 0; i < uniformBuffersCount; i++) {
                std::pair<VkBuffer, VkDeviceMemory> uniformBuffer = m_Renderer->CreateUniformBuffers(uniformBufferChunkSize);
                mesh.uniformBuffers.emplace_back(uniformBuffer);
            }

            mesh.vertexBuffer = m_Renderer->CreateVertexBuffer(m_CommandPool, mesh.vertices);
            mesh.indicesBuffer = m_Renderer->CreateIndexBuffer(m_CommandPool, mesh.indices);

            for (int i = 0; i < m_SwapChainImages.size(); i++) {

                VkDescriptorSet descriptorSet = m_Renderer->CreateDescriptorSets(descriptorPool, { desriptorSetLayout }, 1);
                Texture tex = m_TextureManager->GetTextures()[mesh.texture];

                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = tex.imageView;
                imageInfo.sampler = tex.sampler;

                m_Renderer->UpdateDescriptorSets(mesh.uniformBuffers, tex, descriptorSet, imageInfo);
                mesh.descriptorSets.emplace_back(descriptorSet);
            }

            mesh.pipelineLayout = m_Renderer->CreatePipelineLayout(mesh.descriptorSets, { desriptorSetLayout });

            std::string shaderName = "main";

            std::vector<VkPipelineShaderStageCreateInfo>shadersStageInfos;

            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = m_ShaderManager->GetShaders()->shaders[shaderName][0];
            vertShaderStageInfo.pName = "main";
            shadersStageInfos.emplace_back(vertShaderStageInfo);

            VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = m_ShaderManager->GetShaders()->shaders[shaderName][1];
            fragShaderStageInfo.pName = "main";
            shadersStageInfos.emplace_back(fragShaderStageInfo);

            mesh.graphicsPipeline = m_Renderer->CreateGraphicsPipeline(m_RenderPass, mesh.pipelineLayout, mesh.pipelineCache, shadersStageInfos);
        }

        /// SKYBOX ///
        Mesh& skyboxMesh = *m_MeshManager->GetSkyboxMesh();

        std::pair<VkBuffer, VkDeviceMemory> uniformBuffer = m_Renderer->CreateUniformBuffers(1);
        skyboxMesh.uniformBuffers.emplace_back(uniformBuffer);

        skyboxMesh.vertexBuffer = m_Renderer->CreateVertexBuffer(m_CommandPool, skyboxMesh.vertices);
        skyboxMesh.indicesBuffer = m_Renderer->CreateIndexBuffer(m_CommandPool, skyboxMesh.indices);

        Texture tex = m_TextureManager->GetSkyboxTexture();
        //Texture tex = m_TextureManager->GetTextures()["skybox_tex"];

        std::array<VkDescriptorPoolSize, 2> skyPoolSizes{};
        skyPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        skyPoolSizes[0].descriptorCount = 10;
        skyPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        skyPoolSizes[1].descriptorCount = 10;

        VkDescriptorPool skyDescriptorPool = m_Renderer->CreateDescriptorPool(poolSizes, 100);

        VkDescriptorSetLayoutBinding skyUboLayoutBinding{};
        skyUboLayoutBinding.binding = 0;
        skyUboLayoutBinding.descriptorCount = 1;
        skyUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        skyUboLayoutBinding.pImmutableSamplers = nullptr;
        skyUboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding skySamplerLayoutBinding{};
        skySamplerLayoutBinding.binding = 1;
        skySamplerLayoutBinding.descriptorCount = 1;
        skySamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        skySamplerLayoutBinding.pImmutableSamplers = nullptr;
        skySamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkDescriptorSetLayoutBinding> skyBindings = { skyUboLayoutBinding, skySamplerLayoutBinding };

        VkDescriptorSetLayout skyDesriptorSetLayout = m_Renderer->CreateDescriptorSetLayout(
            skyBindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
        );

        VkDescriptorImageInfo skyDescriptorImageInfo{};
        skyDescriptorImageInfo.sampler = tex.sampler;
        skyDescriptorImageInfo.imageView = tex.imageView;
        skyDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


        VkDescriptorSet skyDescriptorSet = m_Renderer->CreateDescriptorSets(skyDescriptorPool, { skyDesriptorSetLayout }, 1);

        for (int i = 0; i < m_SwapChainImages.size(); i++) {
            m_Renderer->UpdateDescriptorSets(skyboxMesh.uniformBuffers, tex, skyDescriptorSet, skyDescriptorImageInfo);
            skyboxMesh.descriptorSets.emplace_back(skyDescriptorSet);
        }

        skyboxMesh.pipelineLayout = m_Renderer->CreatePipelineLayout(skyboxMesh.descriptorSets, { skyDesriptorSetLayout });

        const char* shaderName = "skybox";

        std::vector<VkPipelineShaderStageCreateInfo>shadersStageInfos;

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = m_ShaderManager->GetShaders()->shaders[shaderName][0];
        vertShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(vertShaderStageInfo);

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = m_ShaderManager->GetShaders()->shaders[shaderName][1];
        fragShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(fragShaderStageInfo);

        skyboxMesh.graphicsPipeline = m_Renderer->CreateGraphicsPipeline(m_RenderPass, skyboxMesh.pipelineLayout, skyboxMesh.pipelineCache, shadersStageInfos, VK_CULL_MODE_FRONT_BIT, false, false, false);

        UpdateWorldPositions();
    }

    void VulkanAdapter::PrepareDraw()
    {
        if (m_IsPrepared) return;

        m_IsPrepared = true;
    }

    void VulkanAdapter::UpdateWorldPositions()
    {
        Mesh* skyMesh = m_MeshManager->GetSkyboxMesh();
        skyMesh->ubos[0].view = m_Camera->LookAt();
        m_Renderer->UpdateUniformBuffer(skyMesh->uniformBuffers[0], skyMesh->ubos, skyMesh->ubos.size());

        std::vector<Mesh>* worldMeshes = m_MeshManager->GetWorldMeshes();
        std::map<const char*, std::array<uint32_t, 2>> worldMeshesLoaded = m_MeshManager->GetWoldMeshesLoaded();

        int32_t uboCount = 1, uboIndex = 0;
        std::vector<UniformBufferObject> chunk;
        int32_t beginRange, endRange = 0;
        uint32_t totalInstances = 0;
        uint32_t maxUniformBufferRange = 0;
        uint32_t uniformBufferChunkSize = 0;
        int32_t nextChunk = 0;

        for (Mesh& mesh : *worldMeshes) {

            uboCount = 1, uboIndex = 0;
            chunk = {};
            beginRange, endRange = 0;
            totalInstances = worldMeshesLoaded[mesh.name][0];
            maxUniformBufferRange = m_Renderer->GetDeviceProperties().limits.maxUniformBufferRange;
            uniformBufferChunkSize = maxUniformBufferRange / sizeof(UniformBufferObject);
            nextChunk = totalInstances - uniformBufferChunkSize;

            if (nextChunk < 0) nextChunk = 0;

            for (int i = totalInstances - 1; i >= 0; i--) {

                mesh.ubos[i].view = m_Camera->LookAt();

                if (i == nextChunk) {

                    nextChunk -= uniformBufferChunkSize;

                    if (nextChunk < 0) nextChunk = 0;
                    endRange = uniformBufferChunkSize * uboCount;
                    beginRange = endRange - uniformBufferChunkSize;

                    if (endRange > totalInstances) endRange = totalInstances;

                    chunk = { mesh.ubos.rbegin() + beginRange, mesh.ubos.rbegin() + endRange };

                    m_Renderer->UpdateUniformBuffer(mesh.uniformBuffers[uboIndex], chunk, chunk.size());
                    uboIndex += 1;
                    uboCount += 1;
                }
            }
        }
    }

    void VulkanAdapter::Draw()
    {
        if (!m_IsPrepared) {
            throw std::runtime_error("Draw is not prepared. Forgot to calle Prepare() ?");
        }

        UpdateWorldPositions();

        SouldResizeSwapChain();
                        
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

        //draw the world !
        for (Mesh mesh : *m_MeshManager->GetWorldMeshes()) {
            m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], mesh.graphicsPipeline);
            m_Renderer->Draw(m_CommandBuffers[m_ImageIndex], &mesh, m_ImageIndex);
        }

        //draw the skybox !
        m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], m_MeshManager->GetSkyboxMesh()->graphicsPipeline);
        m_Renderer->Draw(m_CommandBuffers[m_ImageIndex], m_MeshManager->GetSkyboxMesh(), m_ImageIndex);

        m_Renderer->EndRenderPass(m_CommandBuffers[m_ImageIndex]);
        m_Renderer->EndCommandBuffer(m_CommandBuffers[m_ImageIndex]);

        if (-1 != m_ImageIndex) {
            m_Renderer->QueueSubmit(m_ImageIndex, m_CommandBuffers[m_ImageIndex], m_Semaphores);
            m_Renderer->QueuePresent(m_ImageIndex, m_SwapChain, m_Semaphores);
        }
    }

    void VulkanAdapter::Destroy()
    {
        //@todo refactor all the destroy system...
        m_Renderer->DestroySwapchain(m_Renderer->GetDevice(), m_SwapChain, m_SwapChainFramebuffers, m_SwapChainImageViews);
        m_Renderer->DestroySemaphores(m_Semaphores);
    
        for (Mesh mesh : *m_MeshManager->GetWorldMeshes()) {
            for (auto buffer : mesh.uniformBuffers) {
                m_Renderer->DestroyBuffer(buffer.first);
                m_Renderer->DestroyDeviceMemory(buffer.second);
            }

            m_Renderer->DestroyBuffer(mesh.vertexBuffer.first);
            m_Renderer->DestroyDeviceMemory(mesh.vertexBuffer.second);

            m_Renderer->DestroyBuffer(mesh.indicesBuffer.first);
            m_Renderer->DestroyDeviceMemory(mesh.indicesBuffer.second);
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

        for (auto shader : m_ShaderManager->GetShaders()->shaders) {
            vkDestroyShaderModule(m_Renderer->GetDevice(), shader.second[0], nullptr);
            vkDestroyShaderModule(m_Renderer->GetDevice(), shader.second[1], nullptr);
        }

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

    VkRenderPass VulkanAdapter::CreateImGuiRenderPass()
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

        VkResult result = vkCreateRenderPass(m_Renderer->GetDevice(), &info, nullptr, &renderPass);

        if (result != VK_SUCCESS) {
            Rbk::Log::GetLogger()->critical("failed to create imgui render pass : {}", result);
        } else {
            Rbk::Log::GetLogger()->debug("created successfully imgui render pass");
        }

        return renderPass;
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
