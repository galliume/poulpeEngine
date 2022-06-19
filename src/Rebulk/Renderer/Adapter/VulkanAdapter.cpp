#include "rebulkpch.h"
#include "VulkanAdapter.h"
#include "Rebulk/GUI/Window.h"
#include <volk.h>
#include "Rebulk/Component/Mesh.h"

namespace Rbk
{
    float VulkanAdapter::s_AmbiantLight = 0.1f;
    float VulkanAdapter::s_FogDensity = 0.0f;
    float VulkanAdapter::s_FogColor[3] = { 25 / 255.0f, 25 / 255.0f, 25 / 255.0f };
    int VulkanAdapter::s_Crosshair = 0;

    struct constants {
        uint32_t textureID;
        glm::vec3 cameraPos;
        float ambiantLight;
        float fogDensity;
        glm::vec3 fogColor;
        glm::vec3 lightPos;
    };

    struct cPC {
        uint32_t textureID;
    };

    VulkanAdapter::VulkanAdapter(std::shared_ptr<Window> window) :
        m_Renderer(std::make_shared<VulkanRenderer>(window)),
        m_Window(window)
    {
    }

    VulkanAdapter::~VulkanAdapter()
    {
        std::cout << "VulkanAdapter deleted." << std::endl;
    }

    void VulkanAdapter::Init()
    {
        m_LightsPos.emplace_back(glm::vec3(0.5f, 4.5f, -3.00f));
        SetPerspective();
        m_RenderPass = m_Renderer->CreateRenderPass(m_Renderer->GetMsaaSamples());
        m_SwapChain = m_Renderer->CreateSwapChain(m_SwapChainImages);
        m_CommandPool = m_Renderer->CreateCommandPool();
        VulkanShaders m_Shaders;
        m_ThreadPool = std::make_shared<ThreadPool>();
        m_ThreadPool->Start();
    }

    void VulkanAdapter::AddTextureManager(std::shared_ptr<TextureManager> textureManager)
    {
        m_TextureManager = textureManager;
    }

    void VulkanAdapter::AddShaderManager(std::shared_ptr<ShaderManager> shaderManager)
    {
        m_ShaderManager = shaderManager;
    }

    void VulkanAdapter::AddCamera(std::shared_ptr<Camera> camera)
    {
        m_Camera = camera;
    }

    void VulkanAdapter::AddEntityManager(std::shared_ptr<EntityManager> entityManager)
    {
        m_EntityManager = entityManager;
    }

    void VulkanAdapter::RecreateSwapChain()
    {
        m_Renderer->WaitIdle();
        m_Renderer->InitDetails();
        VkSwapchainKHR old = m_SwapChain;
        m_SwapChain = m_Renderer->CreateSwapChain(m_SwapChainImages, old);
        m_Renderer->DestroySwapchain(m_Renderer->GetDevice(), old, m_SwapChainFramebuffers, m_SwapChainImageViews);
        m_Renderer->DestroySemaphores(m_Semaphores);
        m_Renderer->ResetCurrentFrameIndex();
        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        m_DepthImageViews.resize(m_SwapChainImages.size());
        m_ColorImageViews.resize(m_SwapChainImages.size());

        for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {

            m_SwapChainImageViews[i] = m_Renderer->CreateImageView(m_SwapChainImages[i], m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
            VkDeviceMemory colorImageMemory;
            VkImage colorImage;
            m_Renderer.get()->CreateImage(m_Renderer.get()->GetSwapChainExtent().width, m_Renderer.get()->GetSwapChainExtent().height, 1, m_Renderer.get()->GetMsaaSamples(), m_Renderer.get()->GetSwapChainImageFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
            VkImageView colorImageView = m_Renderer->CreateImageView(colorImage, m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT, 1);

            m_ColorImageViews[i] = colorImageView;

            VkImage depthImage;
            VkDeviceMemory depthImageMemory;
            m_Renderer.get()->CreateImage(m_Renderer.get()->GetSwapChainExtent().width, m_Renderer.get()->GetSwapChainExtent().height, 1, m_Renderer.get()->GetMsaaSamples(), m_Renderer.get()->FindDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
            VkImageView depthImageView = m_Renderer.get()->CreateImageView(depthImage, m_Renderer.get()->FindDepthFormat(), 1, VK_IMAGE_ASPECT_DEPTH_BIT);

            m_DepthImageViews[i] = depthImageView;
        }

        m_SwapChainFramebuffers = m_Renderer->CreateFramebuffers(m_RenderPass, m_SwapChainImageViews, m_DepthImageViews, m_ColorImageViews);
        m_Semaphores = m_Renderer->CreateSyncObjects(m_SwapChainImages);
        /*  m_Renderer->ResetCommandPool(m_CommandPool); */
        m_CommandBuffers = m_Renderer->AllocateCommandBuffers(m_CommandPool, static_cast<uint32_t>(m_SwapChainImageViews.size()));
    }

    void VulkanAdapter::ShouldRecreateSwapChain()
    {
        if (Rbk::Window::m_FramebufferResized == true) {

            while (m_Window->IsMinimized()) {
                m_Window->Wait();
            }

            RecreateSwapChain();

            Rbk::Window::m_FramebufferResized = false;
        }
    }

    void VulkanAdapter::Prepare()
    {
        bool wireFrame = false;

        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        m_DepthImageViews.resize(m_SwapChainImages.size());
        m_ColorImageViews.resize(m_SwapChainImages.size());
        m_CommandBuffers = m_Renderer->AllocateCommandBuffers(m_CommandPool, static_cast<uint32_t>(m_SwapChainImageViews.size()));

        VkVertexInputBindingDescription bDesc = Vertex::GetBindingDescription();
        VkDeviceMemory colorImageMemory;
        VkDeviceMemory depthImageMemory;

        for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
            m_SwapChainImageViews[i] = m_Renderer->CreateImageView(m_SwapChainImages[i], m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);

            VkImage colorImage;
            m_Renderer.get()->CreateImage(m_Renderer.get()->GetSwapChainExtent().width, m_Renderer.get()->GetSwapChainExtent().height, 1, m_Renderer.get()->GetMsaaSamples(), m_Renderer.get()->GetSwapChainImageFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
            VkImageView colorImageView = m_Renderer->CreateImageView(colorImage, m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT, 1);

            m_ColorImageViews[i] = colorImageView;

            VkImage depthImage;
            m_Renderer.get()->CreateImage(m_Renderer.get()->GetSwapChainExtent().width, m_Renderer.get()->GetSwapChainExtent().height, 1, m_Renderer.get()->GetMsaaSamples(), m_Renderer.get()->FindDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
            VkImageView depthImageView = m_Renderer.get()->CreateImageView(depthImage, m_Renderer.get()->FindDepthFormat(), 1, VK_IMAGE_ASPECT_DEPTH_BIT);
            
            m_DepthImageViews[i]  = depthImageView;
        }

        m_Semaphores = m_Renderer->CreateSyncObjects(m_SwapChainImages);

        std::vector<std::shared_ptr<Entity>>* entities = m_EntityManager->GetEntities();

        uint32_t maxUniformBufferRange = 0;
        uint32_t uniformBufferChunkSize = 0;
        uint32_t uniformBuffersCount = 0;

        std::vector<VkDescriptorPoolSize> poolSizes{};
        VkDescriptorPoolSize cp1;
        cp1.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        cp1.descriptorCount = 1000;
        VkDescriptorPoolSize cp2;
        cp2.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cp2.descriptorCount = 1000;
        poolSizes.emplace_back(cp1);
        poolSizes.emplace_back(cp2);

        VkDescriptorPool descriptorPool = m_Renderer->CreateDescriptorPool(poolSizes, 1000);
        m_DescriptorPools.emplace_back(descriptorPool);

        std::vector<VkPushConstantRange> pushConstants = {};
        VkPushConstantRange vkPushconstants;
        vkPushconstants.offset = 0;
        vkPushconstants.size = sizeof(constants);
        vkPushconstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        pushConstants.emplace_back(vkPushconstants);

        m_EntitiesCommandPool = m_Renderer->CreateCommandPool();
        m_EntitiesCommandBuffers = m_Renderer->AllocateCommandBuffers(m_EntitiesCommandPool, static_cast<uint32_t>(m_SwapChainImageViews.size()), true);

        for (std::shared_ptr<Entity>& entity : *entities) {

            std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);

            mesh->m_CameraPos = m_Camera->GetPos();
            uint32_t totalInstances = static_cast<uint32_t>(entities->size());

            maxUniformBufferRange = m_Renderer->GetDeviceProperties().limits.maxUniformBufferRange;
            uniformBufferChunkSize = maxUniformBufferRange / sizeof(UniformBufferObject);
            uniformBuffersCount = static_cast<uint32_t>(std::ceil(static_cast<float>(totalInstances) / static_cast<float>(uniformBufferChunkSize)));

            for (uint32_t i = 0; i < uniformBuffersCount; i++) {
                std::pair<VkBuffer, VkDeviceMemory> uniformBuffer = m_Renderer->CreateUniformBuffers(uniformBufferChunkSize);
                mesh->m_UniformBuffers.emplace_back(uniformBuffer);
            }

            std::vector<VkDescriptorImageInfo> imageInfos;

            uint32_t index = 0;
            for (Data& data : *mesh->GetData()) {

                data.m_VertexBuffer = m_Renderer->CreateVertexBuffer(m_EntitiesCommandPool, data.m_Vertices);
                data.m_IndicesBuffer = m_Renderer->CreateIndexBuffer(m_EntitiesCommandPool, data.m_Indices);
                data.m_TextureIndex = index;
                Texture tex = m_TextureManager->GetTextures()[data.m_Texture];

                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = tex.imageView;
                imageInfo.sampler = tex.sampler;

                imageInfos.emplace_back(imageInfo);
                index++;
            }

            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding = 1;
            samplerLayoutBinding.descriptorCount = index;
            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers = nullptr;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };

            VkDescriptorSetLayout desriptorSetLayout = m_Renderer->CreateDescriptorSetLayout(
                bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
            );

            m_DescriptorSetLayouts.emplace_back(desriptorSetLayout);

            for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
                VkDescriptorSet meshDescriptorSets = m_Renderer->CreateDescriptorSets(descriptorPool, { desriptorSetLayout }, 1);
                m_Renderer->UpdateDescriptorSets(mesh->m_UniformBuffers, meshDescriptorSets, imageInfos);

                mesh->m_DescriptorSets.emplace_back(meshDescriptorSets);
            }

            mesh->m_PipelineLayout = m_Renderer->CreatePipelineLayout(mesh->m_DescriptorSets, { desriptorSetLayout }, pushConstants);

            std::vector<VkPipelineShaderStageCreateInfo>shadersStageInfos;

            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = m_ShaderManager->GetShaders()->shaders[mesh->GetShaderName()][0];
            vertShaderStageInfo.pName = "main";
            shadersStageInfos.emplace_back(vertShaderStageInfo);

            VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = m_ShaderManager->GetShaders()->shaders[mesh->GetShaderName()][1];
            fragShaderStageInfo.pName = "main";
            shadersStageInfos.emplace_back(fragShaderStageInfo);

            auto desc = Vertex::GetAttributeDescriptions();
            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
            vertexInputInfo.pVertexBindingDescriptions = &bDesc;
            vertexInputInfo.pVertexAttributeDescriptions = desc.data();

            mesh->m_GraphicsPipeline = m_Renderer->CreateGraphicsPipeline(
                m_RenderPass,
                mesh->m_PipelineLayout,
                mesh->m_PipelineCache,
                shadersStageInfos,
                vertexInputInfo,
                VK_CULL_MODE_BACK_BIT,
                false, true, true, true, wireFrame
            );
        }

        /// SKYBOX ///
        const std::vector<Vertex> skyVertices = {
            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

            {{-1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}
        };

        m_SkyboxCommandPool = m_Renderer->CreateCommandPool();
        m_SkyboxCommandBuffers = m_Renderer->AllocateCommandBuffers(m_SkyboxCommandPool, static_cast<uint32_t>(m_SwapChainImageViews.size()), true);

        std::shared_ptr<Mesh> skyboxMesh = std::make_shared<Mesh>();
   
        UniformBufferObject skyUbo;
        skyUbo.model = glm::mat4(0.0f);
        skyUbo.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        skyUbo.proj = m_Perspective;
        skyUbo.proj[1][1] *= -1;

        Data skyboxMeshData;
        skyboxMeshData.m_Texture = "skybox";
        skyboxMeshData.m_Vertices = skyVertices;
        skyboxMeshData.m_VertexBuffer = m_Renderer->CreateVertexBuffer(m_SkyboxCommandPool, skyVertices);
        skyboxMeshData.m_Ubos.emplace_back(skyUbo);
        skyboxMeshData.m_TextureIndex = 0;

        std::pair<VkBuffer, VkDeviceMemory> uniformBuffer = m_Renderer->CreateUniformBuffers(1);
        skyboxMesh->m_UniformBuffers.emplace_back(uniformBuffer);

        Texture tex = m_TextureManager->GetSkyboxTexture();

        VkDescriptorPool skyDescriptorPool = m_Renderer->CreateDescriptorPool(poolSizes, 10);
        m_DescriptorPools.emplace_back(skyDescriptorPool);

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
        m_DescriptorSetLayouts.emplace_back(skyDesriptorSetLayout);

        VkDescriptorImageInfo skyDescriptorImageInfo{};
        skyDescriptorImageInfo.sampler = tex.sampler;
        skyDescriptorImageInfo.imageView = tex.imageView;
        skyDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
            VkDescriptorSet skyDescriptorSet = m_Renderer->CreateDescriptorSets(skyDescriptorPool, { skyDesriptorSetLayout }, 1);
            m_Renderer->UpdateDescriptorSets(skyboxMesh->m_UniformBuffers, skyDescriptorSet, { skyDescriptorImageInfo });
            skyboxMesh->m_DescriptorSets.emplace_back(skyDescriptorSet);
        }

        skyboxMesh->m_PipelineLayout = m_Renderer->CreatePipelineLayout(skyboxMesh->m_DescriptorSets, { skyDesriptorSetLayout }, pushConstants);

        std::string shaderName = "skybox";

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

        auto skyDesc = Vertex::GetAttributeDescriptions();
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
        vertexInputInfo.pVertexBindingDescriptions = &bDesc;
        vertexInputInfo.pVertexAttributeDescriptions = skyDesc.data();

        skyboxMesh->m_GraphicsPipeline = m_Renderer->CreateGraphicsPipeline(
            m_RenderPass,
            skyboxMesh->m_PipelineLayout,
            skyboxMesh->m_PipelineCache,
            shadersStageInfos,
            vertexInputInfo,
            VK_CULL_MODE_NONE,
            false
        );

        skyboxMesh->GetData()->emplace_back(skyboxMeshData);
        m_EntityManager->SetSkyboxMesh(skyboxMesh);

        //crosshair
        const std::vector<Vertex2D> vertices = {
            {{-0.025f, -0.025f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.025f, -0.025f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.025f, 0.025f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.025f, 0.025f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
        };
        const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0
        };

        UniformBufferObject ubo;
        ubo.view = glm::mat4(0.0f);

        m_CrosshairCommandPool = m_Renderer->CreateCommandPool();
        m_CrosshairCommandBuffers = m_Renderer->AllocateCommandBuffers(m_CrosshairCommandPool, static_cast<uint32_t>(m_SwapChainImageViews.size()), true);

        Data crossHairData;
        crossHairData.m_Texture = "crosshair";
        crossHairData.m_TextureIndex = 0;
        crossHairData.m_VertexBuffer = m_Renderer->CreateVertex2DBuffer(m_CrosshairCommandPool, vertices);
        crossHairData.m_IndicesBuffer = m_Renderer->CreateIndexBuffer(m_CrosshairCommandPool, indices);
        crossHairData.m_Ubos.emplace_back(ubo);
        crossHairData.m_Indices = indices;

        m_Crosshair = std::make_shared<Mesh2D>();
        m_Crosshair->m_Name = "crosshair";
      
        std::pair<VkBuffer, VkDeviceMemory> crossHairuniformBuffer = m_Renderer->CreateUniformBuffers(1);
        m_Crosshair->m_UniformBuffers.emplace_back(crossHairuniformBuffer);

        VkDescriptorPool cdescriptorPool = m_Renderer->CreateDescriptorPool(poolSizes, 10);
        m_DescriptorPools.emplace_back(cdescriptorPool);

        Texture ctex = m_TextureManager->GetTextures()["crosshair"];
        Texture ctex2 = m_TextureManager->GetTextures()["crosshair2"];

        std::vector<VkDescriptorImageInfo>cimageInfos;
        VkDescriptorImageInfo cimageInfo{};
        cimageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        cimageInfo.imageView = ctex.imageView;
        cimageInfo.sampler = ctex.sampler;

        VkDescriptorImageInfo cimageInfo2{};
        cimageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        cimageInfo2.imageView = ctex2.imageView;
        cimageInfo2.sampler = ctex2.sampler;

        cimageInfos.emplace_back(cimageInfo);
        cimageInfos.emplace_back(cimageInfo2);

        VkDescriptorSetLayoutBinding cuboLayoutBinding{};
        cuboLayoutBinding.binding = 0;
        cuboLayoutBinding.descriptorCount = 1;
        cuboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        cuboLayoutBinding.pImmutableSamplers = nullptr;
        cuboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding csamplerLayoutBinding{};
        csamplerLayoutBinding.binding = 1;
        csamplerLayoutBinding.descriptorCount = cimageInfos.size();
        csamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        csamplerLayoutBinding.pImmutableSamplers = nullptr;
        csamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkDescriptorSetLayoutBinding> cbindings = { cuboLayoutBinding, csamplerLayoutBinding };

        VkDescriptorSetLayout cdesriptorSetLayout = m_Renderer->CreateDescriptorSetLayout(
            cbindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
        );
        m_DescriptorSetLayouts.emplace_back(cdesriptorSetLayout);

        for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
            VkDescriptorSet cdescriptorSet = m_Renderer->CreateDescriptorSets(cdescriptorPool, { cdesriptorSetLayout }, 1);
            m_Renderer->UpdateDescriptorSets(m_Crosshair->m_UniformBuffers, cdescriptorSet, cimageInfos);
            m_Crosshair->m_DescriptorSets.emplace_back(cdescriptorSet);
        }

        std::vector<VkPushConstantRange> cpushConstants = {};
        VkPushConstantRange cPushconstant;
        cPushconstant.offset = 0;
        cPushconstant.size = sizeof(cPC);
        cPushconstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        cpushConstants.emplace_back(cPushconstant);

        m_Crosshair->m_PipelineLayout = m_Renderer->CreatePipelineLayout(m_Crosshair->m_DescriptorSets, { cdesriptorSetLayout }, cpushConstants);

        std::vector<VkPipelineShaderStageCreateInfo>cshadersStageInfos;

        VkPipelineShaderStageCreateInfo cvertShaderStageInfo{};
        cvertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        cvertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        cvertShaderStageInfo.module = m_ShaderManager->GetShaders()->shaders["2d"][0];
        cvertShaderStageInfo.pName = "main";
        cshadersStageInfos.emplace_back(cvertShaderStageInfo);

        VkPipelineShaderStageCreateInfo cfragShaderStageInfo{};
        cfragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        cfragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        cfragShaderStageInfo.module = m_ShaderManager->GetShaders()->shaders["2d"][1];
        cfragShaderStageInfo.pName = "main";
        cshadersStageInfos.emplace_back(cfragShaderStageInfo);

        VkVertexInputBindingDescription bDesc2D = Vertex::GetBindingDescription();
        auto crossDesc = Vertex2D::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo2D{};
        vertexInputInfo2D.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo2D.vertexBindingDescriptionCount = 1;
        vertexInputInfo2D.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex2D::GetAttributeDescriptions().size());
        vertexInputInfo2D.pVertexBindingDescriptions = &bDesc2D;
        vertexInputInfo2D.pVertexAttributeDescriptions = crossDesc.data();

        m_Crosshair->m_GraphicsPipeline = m_Renderer->CreateGraphicsPipeline(
            m_RenderPass,
            m_Crosshair->m_PipelineLayout,
            m_Crosshair->m_PipelineCache,
            cshadersStageInfos,
            vertexInputInfo2D,
            VK_CULL_MODE_FRONT_BIT,
            false
        );

        m_Crosshair->GetData()->emplace_back(crossHairData);

        //command buffer
        m_SwapChainFramebuffers = m_Renderer->CreateFramebuffers(m_RenderPass, m_SwapChainImageViews, m_DepthImageViews, m_ColorImageViews);
        
        vkFreeMemory(m_Renderer->GetDevice(), colorImageMemory, nullptr);
        vkFreeMemory(m_Renderer->GetDevice(), depthImageMemory, nullptr);

        m_lastLookAt = glm::mat4(1.0f);
    }

    void VulkanAdapter::SetPerspective()
    {        
        m_Perspective = glm::perspective(
            glm::radians(60.0f), 
            static_cast<float>(m_Renderer->GetSwapChainExtent().width) / static_cast<float>(m_Renderer->GetSwapChainExtent().height),
            0.1f, 
            10.f
        );
    }

    void VulkanAdapter::SetDeltatime(float deltaTime)
    {
        m_Deltatime = deltaTime;
    }

    void VulkanAdapter::Draw()
    {
        ShouldRecreateSwapChain();

        m_ImageIndex = m_Renderer->AcquireNextImageKHR(m_SwapChain, m_Semaphores);

        if (m_ImageIndex == VK_ERROR_OUT_OF_DATE_KHR || m_ImageIndex == VK_SUBOPTIMAL_KHR) {
            RecreateSwapChain();
        }

        m_Renderer->BeginCommandBuffer(m_CommandBuffers[m_ImageIndex]);

        VkImageMemoryBarrier renderBeginBarrier = m_Renderer->SetupImageMemoryBarrier(
            m_SwapChainImages[m_ImageIndex], 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        );
        m_Renderer->AddPipelineBarrier(
            m_CommandBuffers[m_ImageIndex], renderBeginBarrier, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT
        );

        m_Renderer->BeginRenderPass(m_RenderPass, m_CommandBuffers[m_ImageIndex], m_SwapChainFramebuffers[m_ImageIndex]);
        //m_Renderer->BeginRendering(m_CommandBuffers[m_ImageIndex], m_SwapChainImageViews[m_ImageIndex], m_DepthImageViews[m_ImageIndex], m_ColorImageViews[m_ImageIndex]);
        //m_Renderer->SetViewPort(m_CommandBuffers[m_ImageIndex]);
        //m_Renderer->SetScissor(m_CommandBuffers[m_ImageIndex]);

        constants pushConstants;
        pushConstants.cameraPos = m_Camera->GetPos();
        pushConstants.ambiantLight = Rbk::VulkanAdapter::s_AmbiantLight;
        pushConstants.fogDensity = Rbk::VulkanAdapter::s_FogDensity;
        pushConstants.fogColor = glm::vec3({ Rbk::VulkanAdapter::s_FogColor[0], Rbk::VulkanAdapter::s_FogColor[1], Rbk::VulkanAdapter::s_FogColor[2] });
        pushConstants.lightPos = m_LightsPos.at(0);

        glm::mat4 lookAt = m_Camera->LookAt();

        //if (lookAt == m_lastLookAt) return;

        glm::mat4 proj = m_Perspective;
        proj[1][1] *= -1;
        glm::vec4 cameraPos = m_Camera->GetPos();

        VkCommandBufferInheritanceInfo inheritanceInfo;
        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritanceInfo.renderPass = *m_RenderPass;
        inheritanceInfo.framebuffer = m_SwapChainFramebuffers[m_ImageIndex];
        inheritanceInfo.occlusionQueryEnable = VK_FALSE;
        inheritanceInfo.queryFlags = 0;
        inheritanceInfo.pipelineStatistics = 0;
        inheritanceInfo.pNext = nullptr;
        inheritanceInfo.subpass = 0;

        //draw the mesh entities !
        m_ThreadPool->Queue([=, &pushConstants]() {

            m_Renderer->BeginCommandBuffer(m_EntitiesCommandBuffers[m_ImageIndex], VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, inheritanceInfo);
            m_Renderer->SetViewPort(m_EntitiesCommandBuffers[m_ImageIndex]);
            m_Renderer->SetScissor(m_EntitiesCommandBuffers[m_ImageIndex]);

            std::vector<std::shared_ptr<Entity>> entities = *m_EntityManager->GetEntities();

            for (std::shared_ptr<Entity> entity : entities) {
                std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);

                for (Data& data : *mesh->GetData()) {

                    for (uint32_t i = 0; i < data.m_Ubos.size(); i++) {
                        data.m_Ubos[i].view = lookAt;
                        //mesh->cameraPos = cameraPos * mesh->ubos[i].view * mesh->ubos[i].model * mesh->ubos[i].proj;
                        data.m_Ubos[i].proj = proj;

                        if (mesh->m_Name == "moon_moon_0") {
                            /*mesh->ubos[i].model = glm::rotate(mesh->ubos[i].model, 0.05f * m_Deltatime, glm::vec3(1.0f, 0.0f, 0.0f));
                            mesh->ubos[i].model = glm::translate(mesh->ubos[i].model, m_Deltatime * glm::vec3(1.0f, 0.0f, 0.0f));
                            glm::vec3 lightPos = m_LightsPos.at(0) *  m_Deltatime * glm::vec3(1.0f, 0.0f, 0.0f);
                            m_LightsPos.at(0) = lightPos;*/
                        }

                    }
                    for (uint32_t i = 0; i < mesh->m_UniformBuffers.size(); i++) {
                        m_Renderer->UpdateUniformBuffer(
                            mesh->m_UniformBuffers[i],
                            data.m_Ubos,
                            data.m_Ubos.size()
                        );
                    }

                    pushConstants.textureID = data.m_TextureIndex;

                    m_Renderer->BindPipeline(m_EntitiesCommandBuffers[m_ImageIndex], mesh->m_GraphicsPipeline);
                    vkCmdPushConstants(m_EntitiesCommandBuffers[m_ImageIndex], mesh->m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants), &pushConstants);
                    m_Renderer->Draw(m_EntitiesCommandBuffers[m_ImageIndex], mesh.get(), data, m_ImageIndex);
                }
            }
            m_Renderer->EndCommandBuffer(m_EntitiesCommandBuffers[m_ImageIndex]);
        });

        //draw the skybox !
        m_ThreadPool->Queue([=]() {
            m_Renderer->BeginCommandBuffer(m_SkyboxCommandBuffers[m_ImageIndex], VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, inheritanceInfo);
            m_Renderer->SetViewPort(m_SkyboxCommandBuffers[m_ImageIndex]);
            m_Renderer->SetScissor(m_SkyboxCommandBuffers[m_ImageIndex]);

            std::vector<Rbk::Data> skyboxData = *m_EntityManager->GetSkyboxMesh()->GetData();
           
            glm::mat4 skybowView = glm::mat4(glm::mat3(lookAt));
            for (uint32_t i = 0; i < m_EntityManager->GetSkyboxMesh()->m_UniformBuffers.size(); i++) {
                skyboxData[0].m_Ubos[i].view = skybowView;
                m_Renderer->UpdateUniformBuffer(
                    m_EntityManager->GetSkyboxMesh()->m_UniformBuffers[i],
                    { skyboxData[0].m_Ubos[i] },
                    1
                );
            }
            m_Renderer->BindPipeline(m_SkyboxCommandBuffers[m_ImageIndex], m_EntityManager->GetSkyboxMesh()->m_GraphicsPipeline);
            vkCmdPushConstants(m_SkyboxCommandBuffers[m_ImageIndex], m_EntityManager->GetSkyboxMesh()->m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants), &pushConstants);
            m_Renderer->Draw(m_SkyboxCommandBuffers[m_ImageIndex], m_EntityManager->GetSkyboxMesh().get(), skyboxData[0], m_ImageIndex, false);
            m_Renderer->EndCommandBuffer(m_SkyboxCommandBuffers[m_ImageIndex]);
        });

        //draw the crosshair
        m_ThreadPool->Queue([=]() {
            m_Renderer->BeginCommandBuffer(m_CrosshairCommandBuffers[m_ImageIndex], VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, inheritanceInfo);
            m_Renderer->SetViewPort(m_CrosshairCommandBuffers[m_ImageIndex]);
            m_Renderer->SetScissor(m_CrosshairCommandBuffers[m_ImageIndex]);

            std::vector<Rbk::Data> crosshairData = *m_Crosshair->GetData();
            
            cPC cConst;
            cConst.textureID = VulkanAdapter::s_Crosshair;

            for (uint32_t i = 0; i < m_Crosshair->m_UniformBuffers.size(); i++) {
                crosshairData[0].m_Ubos[i].view = lookAt;
                m_Renderer->UpdateUniformBuffer(m_Crosshair->m_UniformBuffers[i], { crosshairData[0].m_Ubos[i] }, 1);
            }

            m_Renderer->BindPipeline(m_CrosshairCommandBuffers[m_ImageIndex], m_Crosshair->m_GraphicsPipeline);
            vkCmdPushConstants(m_CrosshairCommandBuffers[m_ImageIndex], m_Crosshair->m_PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(cPC), &cConst);
            m_Renderer->Draw(m_CrosshairCommandBuffers[m_ImageIndex], m_Crosshair.get(), crosshairData[0], m_ImageIndex);
            m_Renderer->EndCommandBuffer(m_CrosshairCommandBuffers[m_ImageIndex]);
        });

        /*m_Renderer->EndRendering(m_CommandBuffers[m_ImageIndex]);

       VkImageMemoryBarrier endRenderBeginBarrier = m_Renderer->SetupImageMemoryBarrier(
            m_SwapChainImages[m_ImageIndex], 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        );
        m_Renderer->AddPipelineBarrier(
            m_CommandBuffers[m_ImageIndex], endRenderBeginBarrier, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT
        );*/

        while (!m_ThreadPool->Busy()) {
            //Rbk::Log::GetLogger()->debug("still busy");
        };

        m_ThreadPool->Stop();

        std::vector<VkCommandBuffer>secondaryCmdBuffer;
        secondaryCmdBuffer.emplace_back(m_EntitiesCommandBuffers[m_ImageIndex]);
        secondaryCmdBuffer.emplace_back(m_SkyboxCommandBuffers[m_ImageIndex]);
        secondaryCmdBuffer.emplace_back(m_CrosshairCommandBuffers[m_ImageIndex]);

        vkCmdExecuteCommands(m_CommandBuffers[m_ImageIndex], secondaryCmdBuffer.size(), secondaryCmdBuffer.data());

        m_Renderer->EndRenderPass(m_CommandBuffers[m_ImageIndex]);
        m_Renderer->EndCommandBuffer(m_CommandBuffers[m_ImageIndex]);

        m_Renderer->QueueSubmit(m_ImageIndex, m_CommandBuffers[m_ImageIndex], m_Semaphores);
        uint32_t currentFrame = m_Renderer->QueuePresent(m_ImageIndex, m_SwapChain, m_Semaphores);

        if (currentFrame == VK_ERROR_OUT_OF_DATE_KHR || currentFrame == VK_SUBOPTIMAL_KHR) {
            RecreateSwapChain();
        }
    }

    void VulkanAdapter::Destroy()
    {
        m_Renderer->WaitIdle();

        //@todo refactor all the destroy system...
        m_Renderer->DestroySwapchain(m_Renderer->GetDevice(), m_SwapChain, m_SwapChainFramebuffers, m_SwapChainImageViews);
        m_Renderer->DestroySemaphores(m_Semaphores);
    
        std::shared_ptr<Mesh> skyboxMesh = m_EntityManager->GetSkyboxMesh();

        for (auto buffer : skyboxMesh->m_UniformBuffers) {
            m_Renderer->DestroyBuffer(buffer.first);
            m_Renderer->DestroyDeviceMemory(buffer.second);
        }
        
        for (Data data : *skyboxMesh->GetData()) {
            m_Renderer->DestroyBuffer(data.m_VertexBuffer.first);
            m_Renderer->DestroyDeviceMemory(data.m_VertexBuffer.second);

            m_Renderer->DestroyBuffer(data.m_IndicesBuffer.first);
            m_Renderer->DestroyDeviceMemory(data.m_IndicesBuffer.second);
        }
        m_Renderer->DestroyPipeline(skyboxMesh->m_GraphicsPipeline);
        vkDestroyPipelineLayout(m_Renderer->GetDevice(), skyboxMesh->m_PipelineLayout, nullptr);

        for (auto buffer : m_Crosshair->m_UniformBuffers) {
            m_Renderer->DestroyBuffer(buffer.first);
            m_Renderer->DestroyDeviceMemory(buffer.second);
        }

        for (Data data : *m_Crosshair->GetData()) {
            m_Renderer->DestroyBuffer(data.m_VertexBuffer.first);
            m_Renderer->DestroyDeviceMemory(data.m_VertexBuffer.second);

            m_Renderer->DestroyBuffer(data.m_IndicesBuffer.first);
            m_Renderer->DestroyDeviceMemory(data.m_IndicesBuffer.second);
        }
        m_Renderer->DestroyPipeline(m_Crosshair->m_GraphicsPipeline);
        vkDestroyPipelineLayout(m_Renderer->GetDevice(), m_Crosshair->m_PipelineLayout, nullptr);


        std::vector<std::shared_ptr<Entity>> entities = *m_EntityManager->GetEntities();

        for (std::shared_ptr<Entity> entity : entities) {

            std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);

            for (auto buffer : mesh->m_UniformBuffers) {
                m_Renderer->DestroyBuffer(buffer.first);
                m_Renderer->DestroyDeviceMemory(buffer.second);
            }

            for (Data data : *mesh->GetData()) {

                m_Renderer->DestroyBuffer(data.m_VertexBuffer.first);
                m_Renderer->DestroyDeviceMemory(data.m_VertexBuffer.second);

                m_Renderer->DestroyBuffer(data.m_IndicesBuffer.first);
                m_Renderer->DestroyDeviceMemory(data.m_IndicesBuffer.second);
            }
            m_Renderer->DestroyPipeline(mesh->m_GraphicsPipeline);
            vkDestroyPipelineLayout(m_Renderer->GetDevice(), mesh->m_PipelineLayout, nullptr);

        }

        for (auto item : m_TextureManager->GetTextures()) {
            vkDestroySampler(m_Renderer->GetDevice(), item.second.sampler, nullptr);

            vkDestroyImage(m_Renderer->GetDevice(), item.second.image, nullptr);
            m_Renderer->DestroyDeviceMemory(item.second.imageMemory);
            vkDestroyImageView(m_Renderer->GetDevice(), item.second.imageView, nullptr);
        }

        for (auto item: m_DepthImageViews) {
            vkDestroyImageView(m_Renderer->GetDevice(), item, nullptr);
        }

        for (auto item : m_ColorImageViews) {
            vkDestroyImageView(m_Renderer->GetDevice(), item, nullptr);
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

        for (VkDescriptorSetLayout descriptorSetLayout : m_DescriptorSetLayouts) {
            vkDestroyDescriptorSetLayout(m_Renderer->GetDevice(), descriptorSetLayout, nullptr);
        }

        for (VkDescriptorPool descriptorPool : m_DescriptorPools) {
            vkDestroyDescriptorPool(m_Renderer->GetDevice(), descriptorPool, nullptr);
        }

        m_Renderer->DestroyRenderPass(m_RenderPass, m_CommandPool, m_CommandBuffers);
        m_Renderer->DestroyRenderPass(m_RenderPass, m_EntitiesCommandPool, m_EntitiesCommandBuffers);
        m_Renderer->DestroyRenderPass(m_RenderPass, m_SkyboxCommandPool, m_SkyboxCommandBuffers);
        m_Renderer->DestroyRenderPass(m_RenderPass, m_CrosshairCommandPool, m_CrosshairCommandBuffers);
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
        }

        return renderPass;
    }

    VImGuiInfo VulkanAdapter::GetVImGuiInfo()
    {
        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 100;
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
