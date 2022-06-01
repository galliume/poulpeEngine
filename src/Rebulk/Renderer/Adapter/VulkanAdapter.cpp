#include "rebulkpch.h"
#include "VulkanAdapter.h"
#include "Rebulk/GUI/Window.h"
#include <volk.h>

namespace Rbk
{
    float VulkanAdapter::s_AmbiantLight = 1.0;
    float VulkanAdapter::s_FogDensity = 0.0;
    float VulkanAdapter::s_FogColor[3] = { 25 / 255.0f, 25 / 255.0f, 25 / 255.0f };

    struct constants {
        glm::vec3 cameraPos;
        float ambiantLight;
        float fogDensity;
        glm::vec3 fogColor;
    };

    VulkanAdapter::VulkanAdapter(std::shared_ptr<Window> window) :
        m_Window(window),
        m_Renderer(std::make_shared<VulkanRenderer>(window))
    {
    }

    VulkanAdapter::~VulkanAdapter()
    {
        std::cout << "VulkanAdapter deleted." << std::endl;
    }

    void VulkanAdapter::Init()
    {
        SetPerspective();
        m_RenderPass = m_Renderer->CreateRenderPass(m_Renderer->GetMsaaSamples());
        m_SwapChain = m_Renderer->CreateSwapChain(m_SwapChainImages);
        m_CommandPool = m_Renderer->CreateCommandPool();
        VulkanShaders m_Shaders;
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

    void VulkanAdapter::AddMeshManager(std::shared_ptr<MeshManager> meshManager)
    {
        m_MeshManager = meshManager;
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

    void VulkanAdapter::ShouldRecreateSwapChain()
    {
        if (Rbk::Window::m_FramebufferResized == true) {

            while (m_Window.get()->IsMinimized()) {
                m_Window.get()->Wait();
            }

            RecreateSwapChain();

            Rbk::Window::m_FramebufferResized = false;
        }
    }

    void VulkanAdapter::PrepareWorld()
    {
        bool wireFrame = false;

        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        VkVertexInputBindingDescription bDesc = Vertex::GetBindingDescription();

        for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
            m_SwapChainImageViews[i] = m_Renderer->CreateImageView(m_SwapChainImages[i], m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
        }

        /*std::vector<VkImageView> depthImageViews;
        std::vector<VkImageView> colorImageViews;*/

        for (auto&& [textName, tex] : m_TextureManager->GetTextures()) {
            m_DepthImageViews.emplace_back(tex.depthImageView);
            m_ColorImageViews.emplace_back(tex.colorImageView);
        }

        m_Semaphores = m_Renderer->CreateSyncObjects(m_SwapChainImages);

        std::vector<std::shared_ptr<Mesh>>* worldMeshes = m_MeshManager->GetWorldMeshes();
        std::map<std::string, std::array<uint32_t, 2>> worldMeshesLoaded = m_MeshManager->GetWoldMeshesLoaded();

        uint32_t maxUniformBufferRange = 0;
        uint32_t uniformBufferChunkSize = 0;
        uint32_t uniformBuffersCount = 0;

        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = 1000;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = 1000;

        VkDescriptorPool descriptorPool = m_Renderer->CreateDescriptorPool(poolSizes, 1000);

        std::vector<VkPushConstantRange> pushConstants = {};
        VkPushConstantRange vkPushconstants;
        vkPushconstants.offset = 0;
        vkPushconstants.size = sizeof(constants);
        vkPushconstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

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

        pushConstants.emplace_back(vkPushconstants);

        for (std::shared_ptr<Mesh> mesh : *worldMeshes) {


            mesh.get()->cameraPos = m_Camera->GetPos();
            uint32_t totalInstances = worldMeshesLoaded[mesh.get()->name][0];

            maxUniformBufferRange = m_Renderer->GetDeviceProperties().limits.maxUniformBufferRange;
            uniformBufferChunkSize = maxUniformBufferRange / sizeof(UniformBufferObject);
            uniformBuffersCount = std::ceil(totalInstances / (float)uniformBufferChunkSize);

            for (uint32_t i = 0; i < uniformBuffersCount; i++) {
                std::pair<VkBuffer, VkDeviceMemory> uniformBuffer = m_Renderer->CreateUniformBuffers(uniformBufferChunkSize);
                mesh.get()->uniformBuffers.emplace_back(uniformBuffer);
            }

            mesh.get()->vertexBuffer = m_Renderer->CreateVertexBuffer(m_CommandPool, mesh.get()->vertices);
            mesh.get()->indicesBuffer = m_Renderer->CreateIndexBuffer(m_CommandPool, mesh.get()->indices);

            for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {

                VkDescriptorSet descriptorSet = m_Renderer->CreateDescriptorSets(descriptorPool, { desriptorSetLayout }, 1);

                Texture tex = m_TextureManager->GetTextures()[mesh.get()->texture];

                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = tex.imageView;
                imageInfo.sampler = tex.sampler;
             
                m_Renderer->UpdateDescriptorSets(mesh.get()->uniformBuffers, descriptorSet, imageInfo);
                mesh.get()->descriptorSets.emplace_back(descriptorSet);
            }


            mesh.get()->pipelineLayout = m_Renderer->CreatePipelineLayout(mesh.get()->descriptorSets, { desriptorSetLayout }, pushConstants);

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

            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
            vertexInputInfo.pVertexBindingDescriptions = &bDesc;
            vertexInputInfo.pVertexAttributeDescriptions = Vertex::GetAttributeDescriptions().data();

            mesh.get()->graphicsPipeline = m_Renderer->CreateGraphicsPipeline(
                m_RenderPass,
                mesh.get()->pipelineLayout,
                mesh.get()->pipelineCache,
                shadersStageInfos,
                vertexInputInfo,
                VK_CULL_MODE_BACK_BIT,
                true, true, true, wireFrame
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

        std::shared_ptr<Mesh> skyboxMesh = std::make_shared<Mesh>();
   
        skyboxMesh.get()->texture = "skybox";
        skyboxMesh.get()->vertices = skyVertices;
        skyboxMesh.get()->vertexBuffer = m_Renderer->CreateVertexBuffer(m_CommandPool, skyVertices);
     
        std::pair<VkBuffer, VkDeviceMemory> uniformBuffer = m_Renderer->CreateUniformBuffers(1);
        skyboxMesh.get()->uniformBuffers.emplace_back(uniformBuffer);

        glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
        UniformBufferObject skyUbo;
        skyUbo.model = glm::mat4(0.0f);
        //skyUbo.model = glm::translate(skyUbo.model, pos);
        skyUbo.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        skyUbo.proj = m_Perspective;
        skyUbo.proj[1][1] *= -1;

        skyboxMesh.get()->ubos.emplace_back(skyUbo);

        Texture tex = m_TextureManager->GetSkyboxTexture();
        //Texture tex = m_TextureManager->GetTextures()["skybox_tex"];
        m_DepthImageViews.emplace_back(tex.depthImageView);
        m_ColorImageViews.emplace_back(tex.colorImageView);

        std::array<VkDescriptorPoolSize, 2> skyPoolSizes{};
        skyPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        skyPoolSizes[0].descriptorCount = 10;
        skyPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        skyPoolSizes[1].descriptorCount = 10;

        VkDescriptorPool skyDescriptorPool = m_Renderer->CreateDescriptorPool(poolSizes, 10);

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

        for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
            m_Renderer->UpdateDescriptorSets(skyboxMesh.get()->uniformBuffers, skyDescriptorSet, skyDescriptorImageInfo);
            skyboxMesh.get()->descriptorSets.emplace_back(skyDescriptorSet);
        }

        skyboxMesh.get()->pipelineLayout = m_Renderer->CreatePipelineLayout(skyboxMesh.get()->descriptorSets, { skyDesriptorSetLayout }, pushConstants);

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

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
        vertexInputInfo.pVertexBindingDescriptions = &bDesc;
        vertexInputInfo.pVertexAttributeDescriptions = Vertex::GetAttributeDescriptions().data();

        skyboxMesh.get()->graphicsPipeline = m_Renderer->CreateGraphicsPipeline(
            m_RenderPass,
            skyboxMesh.get()->pipelineLayout,
            skyboxMesh.get()->pipelineCache,
            shadersStageInfos,
            vertexInputInfo,
            VK_CULL_MODE_NONE
        );

        m_MeshManager->SetSkyboxMesh(skyboxMesh);

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

        m_Crosshair = std::make_shared<Mesh2D>();
        m_Crosshair->texture = "crosshair";
        m_Crosshair->name = "crosshair";
        m_Crosshair->indices = indices;
        m_Crosshair.get()->vertexBuffer = m_Renderer->CreateVertex2DBuffer(m_CommandPool, vertices);
        m_Crosshair.get()->indicesBuffer = m_Renderer->CreateIndexBuffer(m_CommandPool, indices);
        std::pair<VkBuffer, VkDeviceMemory> crossHairuniformBuffer = m_Renderer->CreateUniformBuffers(1);
        m_Crosshair.get()->uniformBuffers.emplace_back(crossHairuniformBuffer);

        UniformBufferObject ubo;
        ubo.view = glm::mat4(0.0f);
        m_Crosshair.get()->ubos.emplace_back(ubo);

        std::array<VkDescriptorPoolSize, 2> cpoolSizes{};
        cpoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        cpoolSizes[0].descriptorCount = 10;
        cpoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cpoolSizes[1].descriptorCount = 10;

        VkDescriptorPool cdescriptorPool = m_Renderer->CreateDescriptorPool(cpoolSizes, 10);

        VkDescriptorSetLayoutBinding cuboLayoutBinding{};
        cuboLayoutBinding.binding = 0;
        cuboLayoutBinding.descriptorCount = 1;
        cuboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        cuboLayoutBinding.pImmutableSamplers = nullptr;
        cuboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding csamplerLayoutBinding{};
        csamplerLayoutBinding.binding = 1;
        csamplerLayoutBinding.descriptorCount = 1;
        csamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        csamplerLayoutBinding.pImmutableSamplers = nullptr;
        csamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkDescriptorSetLayoutBinding> cbindings = { cuboLayoutBinding, csamplerLayoutBinding };

        VkDescriptorSetLayout cdesriptorSetLayout = m_Renderer->CreateDescriptorSetLayout(
            cbindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
        );

        Texture ctex = m_TextureManager->GetTextures()["crosshair"];

        for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {

            VkDescriptorSet cdescriptorSet = m_Renderer->CreateDescriptorSets(cdescriptorPool, { cdesriptorSetLayout }, 1);

            VkDescriptorImageInfo cimageInfo{};
            cimageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            cimageInfo.imageView = ctex.imageView;
            cimageInfo.sampler = ctex.sampler;

            m_Renderer->UpdateDescriptorSets(m_Crosshair.get()->uniformBuffers, cdescriptorSet, cimageInfo);
            m_Crosshair.get()->descriptorSets.emplace_back(cdescriptorSet);
        }

        m_Crosshair.get()->pipelineLayout = m_Renderer->CreatePipelineLayout(m_Crosshair.get()->descriptorSets, { cdesriptorSetLayout });

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
        VkPipelineVertexInputStateCreateInfo vertexInputInfo2D{};
        vertexInputInfo2D.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo2D.vertexBindingDescriptionCount = 1;
        vertexInputInfo2D.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex2D::GetAttributeDescriptions().size());
        vertexInputInfo2D.pVertexBindingDescriptions = &bDesc2D;
        vertexInputInfo2D.pVertexAttributeDescriptions = Vertex2D::GetAttributeDescriptions().data();

        m_Crosshair.get()->graphicsPipeline = m_Renderer->CreateGraphicsPipeline(
            m_RenderPass,
            m_Crosshair.get()->pipelineLayout,
            m_Crosshair.get()->pipelineCache,
            cshadersStageInfos,
            vertexInputInfo2D,
            VK_CULL_MODE_FRONT_BIT
        );

        //command buffer
        m_SwapChainFramebuffers = m_Renderer->CreateFramebuffers(m_RenderPass, m_SwapChainImageViews, m_DepthImageViews, m_ColorImageViews);
        m_CommandBuffers = m_Renderer->AllocateCommandBuffers(m_CommandPool, (uint32_t)m_SwapChainFramebuffers.size());

        m_lastLookAt = glm::mat4(1.0f);
    }

    void VulkanAdapter::SetPerspective()
    {        
        m_Perspective = glm::perspective(
            glm::radians(60.0f), 
            m_Renderer.get()->GetSwapChainExtent().width / (float)m_Renderer.get()->GetSwapChainExtent().height, 
            0.1f, 
            10.f
        );
    }

    void VulkanAdapter::UpdateWorldPositions()
    {
        glm::mat4 lookAt = m_Camera->LookAt();

        if (lookAt == m_lastLookAt) return;

        glm::mat4 proj = m_Perspective;
        proj[1][1] *= -1;
        glm::vec4 cameraPos = m_Camera->GetPos();

        for (uint32_t i = 0; i < m_Crosshair.get()->uniformBuffers.size(); i++) {
            m_Crosshair.get()->ubos[i].view = lookAt;
            m_Renderer->UpdateUniformBuffer(m_Crosshair.get()->uniformBuffers[i], { m_Crosshair.get()->ubos[i] }, 1);
        }

        std::vector<std::shared_ptr<Mesh>>* worldMeshes = m_MeshManager->GetWorldMeshes();
        for (std::shared_ptr<Mesh> mesh : *worldMeshes) {
            for (uint32_t i = 0; i < mesh.get()->ubos.size(); i++) {
                mesh.get()->ubos[i].view = lookAt;
                mesh.get()->cameraPos = cameraPos;
                mesh.get()->ubos[i].proj = proj;
            }
            for (uint32_t i = 0; i < mesh.get()->uniformBuffers.size(); i++) {
                m_Renderer->UpdateUniformBuffer(
                    mesh.get()->uniformBuffers[i],
                    mesh.get()->ubos,
                    mesh.get()->ubos.size()
                );
            }
        }

        glm::mat4 skybowView = glm::mat4(glm::mat3(lookAt));
        for (uint32_t i = 0; i < m_MeshManager->GetSkyboxMesh()->uniformBuffers.size(); i++) {
            m_MeshManager->GetSkyboxMesh()->ubos[i].view = skybowView;
            m_Renderer->UpdateUniformBuffer(
                m_MeshManager->GetSkyboxMesh()->uniformBuffers[i],
                { m_MeshManager->GetSkyboxMesh()->ubos[i]},
                1
            );
        }

        m_lastLookAt = lookAt;
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
        m_Renderer->SetViewPort(m_CommandBuffers[m_ImageIndex]);
        m_Renderer->SetScissor(m_CommandBuffers[m_ImageIndex]);

        constants data;
        data.cameraPos = m_Camera->GetPos();
        data.ambiantLight = Rbk::VulkanAdapter::s_AmbiantLight;
        data.fogDensity = Rbk::VulkanAdapter::s_FogDensity;
        data.fogColor = glm::vec3({ Rbk::VulkanAdapter::s_FogColor[0], Rbk::VulkanAdapter::s_FogColor[1], Rbk::VulkanAdapter::s_FogColor[2] });

        //draw the world !
        for (std::shared_ptr<Mesh> mesh : *m_MeshManager->GetWorldMeshes()) {
            m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], mesh.get()->graphicsPipeline);
            vkCmdPushConstants(m_CommandBuffers[m_ImageIndex], mesh->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants), &data);
            m_Renderer->Draw(m_CommandBuffers[m_ImageIndex], mesh.get(), m_ImageIndex);
        }

         //draw the skybox !
        m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], m_MeshManager.get()->GetSkyboxMesh()->graphicsPipeline);
        m_Renderer->Draw(m_CommandBuffers[m_ImageIndex], m_MeshManager.get()->GetSkyboxMesh().get(), m_ImageIndex, false);

        //draw the crosshair
        m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], m_Crosshair.get()->graphicsPipeline);
        m_Renderer->Draw(m_CommandBuffers[m_ImageIndex], m_Crosshair.get(), m_ImageIndex);

        m_Renderer->EndRenderPass(m_CommandBuffers[m_ImageIndex]);
        m_Renderer->EndCommandBuffer(m_CommandBuffers[m_ImageIndex]);

        m_Renderer->QueueSubmit(m_ImageIndex, m_CommandBuffers[m_ImageIndex], m_Semaphores);
        uint32_t currentFrame = m_Renderer->QueuePresent(m_ImageIndex, m_SwapChain, m_Semaphores);

        if (currentFrame == VK_ERROR_OUT_OF_DATE_KHR || currentFrame == VK_SUBOPTIMAL_KHR) {
            RecreateSwapChain();
        }

        UpdateWorldPositions();
    }

    void VulkanAdapter::Destroy()
    {
        //@todo refactor all the destroy system...
        m_Renderer->DestroySwapchain(m_Renderer->GetDevice(), m_SwapChain, m_SwapChainFramebuffers, m_SwapChainImageViews);
        m_Renderer->DestroySemaphores(m_Semaphores);
    
        for (std::shared_ptr<Mesh> mesh : *m_MeshManager->GetWorldMeshes()) {
            for (auto buffer : mesh.get()->uniformBuffers) {
                m_Renderer->DestroyBuffer(buffer.first);
                m_Renderer->DestroyDeviceMemory(buffer.second);
            }

            m_Renderer->DestroyBuffer(mesh.get()->vertexBuffer.first);
            m_Renderer->DestroyDeviceMemory(mesh.get()->vertexBuffer.second);

            m_Renderer->DestroyBuffer(mesh.get()->indicesBuffer.first);
            m_Renderer->DestroyDeviceMemory(mesh.get()->indicesBuffer.second);
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
            Rbk::Log::GetLogger()->trace("created successfully imgui render pass");
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
