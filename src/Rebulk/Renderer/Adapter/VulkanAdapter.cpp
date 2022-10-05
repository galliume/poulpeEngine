#include "rebulkpch.h"
#include <future>
#include <memory>
#include <volk.h>
#include "VulkanAdapter.h"
#include "Rebulk/GUI/Window.h"
#include "Rebulk/Component/Mesh.h"
#include "Rebulk/Core/VisitorStrategy/VulkanInitEntity.h"
#include "Rebulk/Core/VisitorStrategy/VulkanSkybox.h"
#include "Rebulk/Core/VisitorStrategy/VulkanHUD.h"
#include "Rebulk/Component/Entity.h"

namespace Rbk
{
    float VulkanAdapter::s_AmbiantLight = 0.1f;
    float VulkanAdapter::s_FogDensity = 0.0f;
    float VulkanAdapter::s_FogColor[3] = { 25 / 255.0f, 25 / 255.0f, 25 / 255.0f };
    int VulkanAdapter::s_Crosshair = 0;
    int VulkanAdapter::s_PolygoneMode = VK_POLYGON_MODE_FILL;

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

        //init swap chain, depth image views, primary command buffers and semaphores
        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        m_DepthImages.resize(m_SwapChainImages.size());
        m_DepthImageViews.resize(m_SwapChainImages.size());
        m_CommandBuffers = m_Renderer->AllocateCommandBuffers(m_CommandPool, static_cast<uint32_t>(m_SwapChainImageViews.size()));

        VkDeviceMemory depthImageMemory;

        for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
            m_SwapChainImageViews[i] = m_Renderer->CreateImageView(m_SwapChainImages[i], m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);

            VkImage depthImage;
            m_Renderer->CreateImage(m_Renderer->GetSwapChainExtent().width, m_Renderer->GetSwapChainExtent().height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
            VkImageView depthImageView = m_Renderer->CreateImageView(depthImage, VK_FORMAT_D32_SFLOAT, 1, VK_IMAGE_ASPECT_DEPTH_BIT);

            m_DepthImages[i] = depthImage;
            m_DepthImageViews[i] = depthImageView;
        }

        m_Semaphores = m_Renderer->CreateSyncObjects(m_SwapChainImages);
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
        m_Renderer->DestroySwapchain(m_Renderer->GetDevice(), old, {}, m_SwapChainImageViews);
        m_Renderer->DestroySemaphores(m_Semaphores);
        m_Renderer->ResetCurrentFrameIndex();
        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        m_DepthImages.resize(m_SwapChainImages.size());
        m_DepthImageViews.resize(m_SwapChainImages.size());

        for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {

            m_SwapChainImageViews[i] = m_Renderer->CreateImageView(m_SwapChainImages[i], m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);

            VkImage depthImage;
            VkDeviceMemory depthImageMemory;
            m_Renderer->CreateImage(m_Renderer->GetSwapChainExtent().width, m_Renderer->GetSwapChainExtent().height, 1, VK_SAMPLE_COUNT_1_BIT, m_Renderer->FindDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
            VkImageView depthImageView = m_Renderer->CreateImageView(depthImage, m_Renderer->FindDepthFormat(), 1, VK_IMAGE_ASPECT_DEPTH_BIT);

            m_DepthImages[i] = depthImage;
            m_DepthImageViews[i] = depthImageView;
        }

        m_Semaphores = m_Renderer->CreateSyncObjects(m_SwapChainImages);
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
        std::vector<std::shared_ptr<Entity>>* entities = m_EntityManager->GetEntities();

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

        VkVertexInputBindingDescription bDesc = Vertex::GetBindingDescription();

        std::shared_ptr<VulkanInitEntity> vulkanisator = std::make_shared<VulkanInitEntity>(shared_from_this(), descriptorPool);
        for (std::shared_ptr<Entity>& entity : *entities) {
            entity->Accept(vulkanisator);
        }

        VkDescriptorPool skyDescriptorPool = m_Renderer->CreateDescriptorPool(poolSizes, 1000);
        m_DescriptorPools.emplace_back(skyDescriptorPool);
        std::shared_ptr<VulkanSkybox> skyboxVulkanisator = std::make_shared<VulkanSkybox>(shared_from_this(), skyDescriptorPool);
        std::shared_ptr<Mesh> skyboxMesh = std::make_shared<Mesh>();
        skyboxMesh->Accept(skyboxVulkanisator);
        m_EntityManager->SetSkyboxMesh(skyboxMesh);

        VkDescriptorPool HUDDescriptorPool = m_Renderer->CreateDescriptorPool(poolSizes, 1000);
        m_DescriptorPools.emplace_back(HUDDescriptorPool);
        std::shared_ptr<VulkanHUD> HUDVulkanisator = std::make_shared<VulkanHUD>(shared_from_this(), HUDDescriptorPool);
        m_HUD = std::make_shared<Mesh2D>();
        m_HUD->Accept(HUDVulkanisator);
    }

    void VulkanAdapter::SetPerspective()
    {        
        m_Perspective = glm::perspective(
            glm::radians(60.0f),
            static_cast<float>(m_Renderer->GetSwapChainExtent().width) / static_cast<float>(m_Renderer->GetSwapChainExtent().height),
            0.1f,
            100.f
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

        VkImageMemoryBarrier swapChainImageRenderBeginBarrier = m_Renderer->SetupImageMemoryBarrier(
            m_SwapChainImages[m_ImageIndex],
            0,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        );

        m_Renderer->AddPipelineBarriers(
            m_CommandBuffers[m_ImageIndex],
            { swapChainImageRenderBeginBarrier },
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        );

        VkImageMemoryBarrier depthImageRenderBeginBarrier = m_Renderer->SetupImageMemoryBarrier(
            m_DepthImages[m_ImageIndex],
            0,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            1,
            VK_IMAGE_ASPECT_DEPTH_BIT
        );

        m_Renderer->AddPipelineBarriers(
            m_CommandBuffers[m_ImageIndex],
            { depthImageRenderBeginBarrier },
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        );

        m_Renderer->BeginRendering(m_CommandBuffers[m_ImageIndex], m_SwapChainImageViews[m_ImageIndex], m_DepthImageViews[m_ImageIndex]);

        m_Renderer->SetViewPort(m_CommandBuffers[m_ImageIndex]);
        m_Renderer->SetScissor(m_CommandBuffers[m_ImageIndex]);

        constants pushConstants;
        pushConstants.cameraPos = m_Camera->GetPos();
        pushConstants.ambiantLight = Rbk::VulkanAdapter::s_AmbiantLight;
        pushConstants.fogDensity = Rbk::VulkanAdapter::s_FogDensity;
        pushConstants.fogColor = glm::vec3({ Rbk::VulkanAdapter::s_FogColor[0], Rbk::VulkanAdapter::s_FogColor[1], Rbk::VulkanAdapter::s_FogColor[2] });
        pushConstants.lightPos = m_LightsPos.at(0);

        glm::mat4 lookAt = m_Camera->LookAt();
        glm::mat4 proj = m_Perspective;
        proj[1][1] *= -1;
        glm::vec4 cameraPos = m_Camera->GetPos();

        //entities !
        std::vector<std::shared_ptr<Entity>> entities = *m_EntityManager->GetEntities();

        for (std::shared_ptr<Entity> entity : entities) {
            std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);

            if (mesh) {
                for (Data& data : *mesh->GetData()) {

                    for (uint32_t i = 0; i < data.m_Ubos.size(); i++) {
                        data.m_Ubos[i].view = lookAt;
                        //mesh->cameraPos = cameraPos * mesh->ubos[i].view * mesh->ubos[i].model * mesh->ubos[i].proj;
                        data.m_Ubos[i].proj = proj;
                    }
                    for (uint32_t i = 0; i < mesh->m_UniformBuffers.size(); i++) {
                        m_Renderer->UpdateUniformBuffer(
                            mesh->m_UniformBuffers[i],
                            data.m_Ubos,
                            data.m_Ubos.size()
                        );
                    }

                    pushConstants.textureID = data.m_TextureIndex;

                    m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], mesh->m_GraphicsPipeline);
                    vkCmdPushConstants(m_CommandBuffers[m_ImageIndex], mesh->m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants), &pushConstants);
                    m_Renderer->Draw(m_CommandBuffers[m_ImageIndex], mesh.get(), data, m_ImageIndex);
                }
            }
        }

        //skybox !
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
        m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], m_EntityManager->GetSkyboxMesh()->m_GraphicsPipeline);
        vkCmdPushConstants(m_CommandBuffers[m_ImageIndex], m_EntityManager->GetSkyboxMesh()->m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants), &pushConstants);
        m_Renderer->Draw(m_CommandBuffers[m_ImageIndex], m_EntityManager->GetSkyboxMesh().get(), skyboxData[0], m_ImageIndex, false);

        //HUD !
        std::vector<Rbk::Data> hud = *m_HUD->GetData();
            
        cPC cConst;
        cConst.textureID = VulkanAdapter::s_Crosshair;

        for (uint32_t i = 0; i < m_HUD->m_UniformBuffers.size(); i++) {
            hud[0].m_Ubos[i].view = lookAt;
            m_Renderer->UpdateUniformBuffer(m_HUD->m_UniformBuffers[i], { hud[0].m_Ubos[i] }, 1);
        }
        m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], m_HUD->m_GraphicsPipeline);
        vkCmdPushConstants(m_CommandBuffers[m_ImageIndex], m_HUD->m_PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(cPC), &cConst);
        m_Renderer->Draw(m_CommandBuffers[m_ImageIndex], m_HUD.get(), hud[0], m_ImageIndex);

        //end rendering !
        m_Renderer->EndRendering(m_CommandBuffers[m_ImageIndex]);
        
       VkImageMemoryBarrier swapChainImageEndRenderBeginBarrier = m_Renderer->SetupImageMemoryBarrier(
            m_SwapChainImages[m_ImageIndex],
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            0,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        );
       m_Renderer->AddPipelineBarriers(
           m_CommandBuffers[m_ImageIndex],
           { swapChainImageEndRenderBeginBarrier },
           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
           VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
           0
       );

        VkImageMemoryBarrier depthImageEndRenderBeginBarrier = m_Renderer->SetupImageMemoryBarrier(
            m_DepthImages[m_ImageIndex],
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            0,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            1,
            VK_IMAGE_ASPECT_DEPTH_BIT
        );
        m_Renderer->AddPipelineBarriers(
            m_CommandBuffers[m_ImageIndex],
            { depthImageEndRenderBeginBarrier },
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0
        );
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
        m_Renderer->DestroySwapchain(m_Renderer->GetDevice(), m_SwapChain, {}, m_SwapChainImageViews);
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

        for (auto buffer : m_HUD->m_UniformBuffers) {
            m_Renderer->DestroyBuffer(buffer.first);
            m_Renderer->DestroyDeviceMemory(buffer.second);
        }

        for (Data data : *m_HUD->GetData()) {
            m_Renderer->DestroyBuffer(data.m_VertexBuffer.first);
            m_Renderer->DestroyDeviceMemory(data.m_VertexBuffer.second);

            m_Renderer->DestroyBuffer(data.m_IndicesBuffer.first);
            m_Renderer->DestroyDeviceMemory(data.m_IndicesBuffer.second);
        }
        m_Renderer->DestroyPipeline(m_HUD->m_GraphicsPipeline);
        vkDestroyPipelineLayout(m_Renderer->GetDevice(), m_HUD->m_PipelineLayout, nullptr);


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
            vkDestroySampler(m_Renderer->GetDevice(), item.second.GetSampler(), nullptr);

            vkDestroyImage(m_Renderer->GetDevice(), item.second.GetImage(), nullptr);
            m_Renderer->DestroyDeviceMemory(item.second.GetImageMemory());
            vkDestroyImageView(m_Renderer->GetDevice(), item.second.GetImageView(), nullptr);
        }

        for (auto item: m_DepthImages) {
            vkDestroyImage(m_Renderer->GetDevice(), item, nullptr);
        }

        for (auto item : m_DepthImageViews) {
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
        std::vector<VkDescriptorPoolSize> poolSizes{};
        VkDescriptorPoolSize cp1;
        cp1.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        cp1.descriptorCount = 1000;
        VkDescriptorPoolSize cp2;
        cp2.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cp2.descriptorCount = 1000;
        poolSizes.emplace_back(cp1);
        poolSizes.emplace_back(cp2);

        VkDescriptorPool imguiPool = m_Renderer->CreateDescriptorPool(poolSizes, 1000);

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

    void VulkanAdapter::Refresh()
    {
        Prepare();
    }
}
