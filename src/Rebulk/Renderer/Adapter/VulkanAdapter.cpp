#include "rebulkpch.h"
#include <future>
#include <memory>
#include <volk.h>
#include "VulkanAdapter.h"
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"
#include "Rebulk/GUI/Window.h"

namespace Rbk
{
    float VulkanAdapter::s_AmbiantLight = 0.1f;
    float VulkanAdapter::s_FogDensity = 0.0f;
    float VulkanAdapter::s_FogColor[3] = { 25 / 255.0f, 25 / 255.0f, 25 / 255.0f };
    int VulkanAdapter::s_Crosshair = 0;
    int VulkanAdapter::s_PolygoneMode = VK_POLYGON_MODE_FILL;
    
    VulkanAdapter::VulkanAdapter(std::shared_ptr<Window> window) :
        m_Window(window)
    {
        m_Renderer = std::make_shared<VulkanRenderer>(window);
    }

    VulkanAdapter::~VulkanAdapter()
    {

    }

    void VulkanAdapter::Init()
    {
        m_RayPick = glm::vec3(0.0f);
        m_LightsPos.emplace_back(glm::vec3(0.5f, 4.5f, -3.00f));
        SetPerspective();
        m_RenderPass = m_Renderer->CreateRenderPass(m_Renderer->GetMsaaSamples());
        m_SwapChain = m_Renderer->CreateSwapChain(m_SwapChainImages);
        m_CommandPool = m_Renderer->CreateCommandPool();

        //init swap chain, depth image views, primary command buffers and semaphores
        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        m_DepthImages.resize(m_SwapChainImages.size());
        m_DepthImageViews.resize(m_SwapChainImages.size());
        m_CommandBuffers = m_Renderer->AllocateCommandBuffers(m_CommandPool, static_cast<uint32_t>(m_SwapChainImageViews.size()));

        VkDeviceMemory depthImageMemory;

        for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
            m_SwapChainImageViews[i] = m_Renderer->CreateImageView(m_SwapChainImages[i], m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);

            VkImage depthImage;
            m_Renderer->CreateImage(m_Renderer->GetSwapChainExtent().width, m_Renderer->GetSwapChainExtent().height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage);
            VkImageView depthImageView = m_Renderer->CreateImageView(depthImage, VK_FORMAT_D32_SFLOAT, 1, VK_IMAGE_ASPECT_DEPTH_BIT);

            m_DepthImages[i] = depthImage;
            m_DepthImageViews[i] = depthImageView;
        }

        m_Semaphores = m_Renderer->CreateSyncObjects(m_SwapChainImages);
    }

    void VulkanAdapter::WaitIdle()
    {
        m_Renderer->WaitIdle();
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
            m_Renderer->CreateImage(m_Renderer->GetSwapChainExtent().width, m_Renderer->GetSwapChainExtent().height, 1, VK_SAMPLE_COUNT_1_BIT, m_Renderer->FindDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage);
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
        BeginRendering();

        constants pushConstants;
        pushConstants.cameraPos = m_Camera->GetPos();
        pushConstants.ambiantLight = Rbk::VulkanAdapter::s_AmbiantLight;
        pushConstants.fogDensity = Rbk::VulkanAdapter::s_FogDensity;
        pushConstants.fogColor = glm::vec3({ Rbk::VulkanAdapter::s_FogColor[0], Rbk::VulkanAdapter::s_FogColor[1], Rbk::VulkanAdapter::s_FogColor[2] });
        pushConstants.lightPos = m_LightsPos.at(0);
        pushConstants.rayPick = m_RayPick;

        glm::mat4 lookAt = m_Camera->LookAt();
        glm::mat4 proj = m_Perspective;
        proj[1][1] *= -1;
        glm::vec4 cameraPos = m_Camera->GetPos();

        //entities !
        for (std::shared_ptr<Entity> entity : *m_Entities) {
            std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);

            if (!mesh) continue;

            m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], mesh->m_GraphicsPipeline);

            if (mesh->IsDirty()) {

                auto min = 0;
                auto max = 0;

                for (Data& data : *mesh->GetData()) {

                    for (uint32_t i = 0; i < data.m_Ubos.size(); i++) {
                        data.m_Ubos[i].view = lookAt;
                        //mesh->cameraPos = cameraPos * mesh->ubos[i].view * mesh->ubos[i].model * mesh->ubos[i].proj;
                        data.m_Ubos[i].proj = proj;
                    }
                }

                for (uint32_t i = 0; i < mesh->m_UniformBuffers.size(); i++) {
                    max = mesh->GetData()->at(0).m_UbosOffset.at(i);
                    auto ubos = std::vector<UniformBufferObject>(mesh->GetData()->at(0).m_Ubos.begin() + min, mesh->GetData()->at(0).m_Ubos.begin() + max);

                    m_Renderer->UpdateUniformBuffer(
                        mesh->m_UniformBuffers[i],
                        ubos,
                        ubos.size()
                    );

                    min = max;
                }

                //mesh->SetIsDirty(false);
            }

            //if (m_HasClicked && mesh->IsHit(m_RayPick)) {
            //    Rbk::Log::GetLogger()->warn("HIT ! {}", mesh->GetName());
            //    m_HasClicked = false;
            //}

            for (Data& data : *mesh->GetData()) {
                int index = m_ImageIndex;
                for (uint32_t i = 0; i < mesh->m_UniformBuffers.size(); i++) {
                    index += i * 3;
                    pushConstants.textureID = data.m_TextureIndex;
                    vkCmdPushConstants(m_CommandBuffers[m_ImageIndex], mesh->m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants), &pushConstants);
                    m_Renderer->Draw(m_CommandBuffers[m_ImageIndex], mesh->GetDescriptorSets().at(index), mesh.get(), data, mesh->m_UniformBuffers.at(i).size, m_ImageIndex);
                    index = m_ImageIndex;
                }
            }
        }

        //bbox !
        for (std::shared_ptr<Entity> bbox : *m_BoundingBox) {
            std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(bbox);

            if (!mesh) continue;
            m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], mesh->m_GraphicsPipeline);

            auto min = 0;
            auto max = 0;

            for (Data& data : *mesh->GetData()) {

                for (uint32_t i = 0; i < data.m_Ubos.size(); i++) {
                    data.m_Ubos[i].view = lookAt;
                    data.m_Ubos[i].proj = proj;
                }
            }

            for (uint32_t i = 0; i < mesh->m_UniformBuffers.size(); i++) {
                max = mesh->GetData()->at(0).m_UbosOffset.at(i);
                auto ubos = std::vector<UniformBufferObject>(mesh->GetData()->at(0).m_Ubos.begin() + min, mesh->GetData()->at(0).m_Ubos.begin() + max);

                m_Renderer->UpdateUniformBuffer(
                    mesh->m_UniformBuffers[i],
                    ubos,
                    ubos.size()
                );

                min = max;
            }

            if (m_HasClicked && mesh->IsHit(m_RayPick)) {
                Rbk::Log::GetLogger()->warn("HIT ! {}", mesh->GetName());
                m_HasClicked = false;
            }

            for (Data& data : *mesh->GetData()) {
                int index = m_ImageIndex;
                for (uint32_t i = 0; i < mesh->m_UniformBuffers.size(); i++) {
                    index += i * 3;
                    m_Renderer->Draw(m_CommandBuffers[m_ImageIndex], mesh->GetDescriptorSets().at(index), mesh.get(), data, mesh->m_UniformBuffers.at(i).size, m_ImageIndex);
                    index = m_ImageIndex;
                }
            }
        }

        //skybox !
        if (m_SkyboxMesh) {
            std::vector<Rbk::Data> skyboxData = *m_SkyboxMesh->GetData();

            if (!skyboxData.empty()) {
                glm::mat4 skybowView = glm::mat4(glm::mat3(lookAt));
                m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], m_SkyboxMesh->m_GraphicsPipeline);

                for (uint32_t i = 0; i < m_SkyboxMesh->m_UniformBuffers.size(); i++) {
                    skyboxData[0].m_Ubos[i].view = skybowView;
                    m_Renderer->UpdateUniformBuffer(
                        m_SkyboxMesh->m_UniformBuffers[i],
                        { skyboxData[0].m_Ubos[i] },
                        1
                    );
                    vkCmdPushConstants(m_CommandBuffers[m_ImageIndex], m_SkyboxMesh->m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants), &pushConstants);
                    m_Renderer->Draw(m_CommandBuffers[m_ImageIndex], m_SkyboxMesh->GetDescriptorSets().at(i), m_SkyboxMesh.get(), skyboxData[0], skyboxData[0].m_Ubos.size(), m_ImageIndex, false);
                }
            }
        }

        //HUD!
        for (std::shared_ptr<Mesh> hudPart : m_HUD) {

            if (!hudPart || !hudPart->IsVisible()) continue;
            m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], hudPart->m_GraphicsPipeline);

            if (hudPart->HasPushConstants() && nullptr != hudPart->ApplyPushConstants)
                hudPart->ApplyPushConstants(m_CommandBuffers[m_ImageIndex], hudPart->m_PipelineLayout);

            for (Data& data : *hudPart->GetData()) {

                for (uint32_t i = 0; i < hudPart->m_UniformBuffers.size(); i++) {
                    data.m_Ubos[i].view = lookAt;
                    data.m_Ubos[i].proj = proj;
                }

                for (uint32_t i = 0; i < hudPart->m_UniformBuffers.size(); i++) {
                    m_Renderer->UpdateUniformBuffer(
                        hudPart->m_UniformBuffers[i],
                        data.m_Ubos,
                        data.m_Ubos.size()
                    );
                    m_Renderer->Draw(m_CommandBuffers[m_ImageIndex], hudPart->GetDescriptorSets().at(i), hudPart.get(), data, data.m_Ubos.size(), m_ImageIndex);

                }
            }
        }

        EndRendering();
    }

    void VulkanAdapter::DrawSplashScreen()
    {
        BeginRendering();

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

        for (std::shared_ptr<Mesh> mesh : m_Splash) {

            if (!mesh || !mesh->IsVisible()) continue;
            m_Renderer->BindPipeline(m_CommandBuffers[m_ImageIndex], mesh->m_GraphicsPipeline);

            for (Data& data : *mesh->GetData()) {

                for (uint32_t i = 0; i < mesh->m_UniformBuffers.size(); i++) {
                    data.m_Ubos[i].view = lookAt;
                    data.m_Ubos[i].proj = proj;
                }

                for (uint32_t i = 0; i < mesh->m_UniformBuffers.size(); i++) {
                    m_Renderer->UpdateUniformBuffer(
                        mesh->m_UniformBuffers[i],
                        data.m_Ubos,
                        data.m_Ubos.size()
                    );

                    if (mesh->HasPushConstants() && nullptr != mesh->ApplyPushConstants)
                        mesh->ApplyPushConstants(m_CommandBuffers[m_ImageIndex], mesh->m_PipelineLayout);

                    m_Renderer->Draw(m_CommandBuffers[m_ImageIndex], mesh->GetDescriptorSets().at(i), mesh.get(), data, data.m_Ubos.size(), m_ImageIndex);
                }
            }
        }

        EndRendering();
    }

    void VulkanAdapter::Destroy()
    {
        m_Renderer->DestroySwapchain(m_Renderer->GetDevice(), m_SwapChain, {}, m_SwapChainImageViews);
        m_Renderer->DestroySemaphores(m_Semaphores);
    
        for (auto item: m_DepthImages) {
            vkDestroyImage(m_Renderer->GetDevice(), item, nullptr);
        }

        for (auto item : m_DepthImageViews) {
            vkDestroyImageView(m_Renderer->GetDevice(), item, nullptr);
        }

        for (auto& buffer : m_UniformBuffers.first) {
            m_Renderer->DestroyBuffer(buffer);
        }

        for (VkDescriptorSetLayout descriptorSetLayout : m_DescriptorSetLayouts) {
            vkDestroyDescriptorSetLayout(m_Renderer->GetDevice(), descriptorSetLayout, nullptr);
        }

        for (VkDescriptorPool descriptorPool : m_DescriptorPools) {
            vkDestroyDescriptorPool(m_Renderer->GetDevice(), descriptorPool, nullptr);
        }

        m_Renderer->DestroyRenderPass(m_RenderPass, m_CommandPool, m_CommandBuffers);
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
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
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

    ImGuiInfo VulkanAdapter::GetImGuiInfo()
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

        ImGuiInfo imGuiInfo;
        imGuiInfo.info = info;
        imGuiInfo.rdrPass = CreateImGuiRenderPass();
        imGuiInfo.cmdBuffer = m_Renderer->AllocateCommandBuffers(m_CommandPool)[0];
        //imGuiInfo.pipeline = m_Pipelines[0].graphicsPipeline;

        return imGuiInfo;
    }

    void VulkanAdapter::ShowGrid(bool show)
    {
        for (auto hudPart : m_HUD) {
            if ("grid" == hudPart->GetName()) {
                hudPart->SetVisible(show);
            }
        }
    }

    void VulkanAdapter::BeginRendering()
    {
        m_MutexRendering.lock();

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
    }

    void VulkanAdapter::EndRendering()
    {
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

        m_MutexRendering.unlock();
    }

    void VulkanAdapter::SetRayPick(float x, float y, float z, int width, int height)
    {
        glm::vec3 rayNds = glm::vec3(x, y, z);
        glm::vec4 rayClip = glm::vec4(rayNds.x, rayNds.y, -1.0, 1.0);
        glm::vec4 rayEye = glm::inverse(GetPerspective()) * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);

        glm::vec4 tmp = (glm::inverse(GetCamera()->GetView()) * rayEye);
        glm::vec3 rayWor = glm::vec3(tmp.x, tmp.y, tmp.z);
        m_RayPick = glm::normalize(rayWor);

        m_HasClicked = true;
    }

    void VulkanAdapter::Clear()
    {
        m_Entities->clear();
        m_BoundingBox->clear();
        m_SkyboxMesh = nullptr;
        m_HUD.clear();
        m_Splash.clear();
    }
}
