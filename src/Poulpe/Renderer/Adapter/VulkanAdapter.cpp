#include <future>
#include <memory>
#include <cfenv>
#include <volk.h>
#include "VulkanAdapter.hpp"
#include "Poulpe/Renderer/Vulkan/VulkanRenderer.hpp"
#include "Poulpe/GUI/Window.hpp"
#include <exception>

namespace Poulpe
{
    //@todo should not be globally accessible
    std::atomic<float> VulkanAdapter::s_AmbiantLight{ 0.5f };
    std::atomic<float> VulkanAdapter::s_FogDensity{ 0.0f };
    std::atomic<float> VulkanAdapter::s_FogColor[3]{ 25 / 255.0f, 25 / 255.0f, 25 / 255.0f };
    std::atomic<int> VulkanAdapter::s_Crosshair{ 0 };
    std::atomic<int> VulkanAdapter::s_PolygoneMode{ VK_POLYGON_MODE_FILL };
    
    VulkanAdapter::VulkanAdapter(std::shared_ptr<Window> window) :
        m_Window(window)
    {
        m_Renderer = std::make_shared<VulkanRenderer>(window);
    }

    void VulkanAdapter::Init()
    {
        m_RayPick = glm::vec3(0.0f);
        m_LightsPos.emplace_back(glm::vec3(0.5f, 4.5f, -3.00f));
        SetPerspective();
        m_RenderPass = m_Renderer->CreateRenderPass(m_Renderer->GetMsaaSamples());
        m_SwapChain = m_Renderer->CreateSwapChain(m_SwapChainImages);

        //init swap chain, depth image views, primary command buffers and semaphores
        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        m_DepthImages.resize(m_SwapChainImages.size());
        m_DepthImageViews.resize(m_SwapChainImages.size());

        m_CommandPoolSplash = m_Renderer->CreateCommandPool();
        m_CommandBuffersSplash = m_Renderer->AllocateCommandBuffers(m_CommandPoolSplash, static_cast<uint32_t>(m_SwapChainImageViews.size()));
        m_CommandPoolEntities = m_Renderer->CreateCommandPool();
        m_CommandBuffersEntities = m_Renderer->AllocateCommandBuffers(m_CommandPoolEntities, static_cast<uint32_t>(m_SwapChainImageViews.size()));
        m_CommandPoolBbox = m_Renderer->CreateCommandPool();
        m_CommandBuffersBbox = m_Renderer->AllocateCommandBuffers(m_CommandPoolBbox, static_cast<uint32_t>(m_SwapChainImageViews.size()));
        m_CommandPoolSkybox = m_Renderer->CreateCommandPool();
        m_CommandBuffersSkybox = m_Renderer->AllocateCommandBuffers(m_CommandPoolSkybox, static_cast<uint32_t>(m_SwapChainImageViews.size()));
        m_CommandPoolHud = m_Renderer->CreateCommandPool();
        m_CommandBuffersHud = m_Renderer->AllocateCommandBuffers(m_CommandPoolHud, static_cast<uint32_t>(m_SwapChainImageViews.size()));

        for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
            m_SwapChainImageViews[i] = m_Renderer->CreateImageView(m_SwapChainImages[i], m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);

            VkImage depthImage;
            m_Renderer->CreateImage(m_Renderer->GetSwapChainExtent().width, m_Renderer->GetSwapChainExtent().height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage);
            VkImageView depthImageView = m_Renderer->CreateImageView(depthImage, VK_FORMAT_D32_SFLOAT, 1, VK_IMAGE_ASPECT_DEPTH_BIT);

            m_DepthImages[i] = depthImage;
            m_DepthImageViews[i] = depthImageView;
        }

        for (int i = 0; i < m_Renderer->GetQueueCount(); i++) {
            m_Semaphores.emplace_back(m_Renderer->CreateSyncObjects(m_SwapChainImages));
        }

        AcquireNextImage();

        m_imguiSampler = m_Renderer->CreateTextureSampler(1);
        m_Renderer->CreateImage(m_Renderer->GetSwapChainExtent().width, m_Renderer->GetSwapChainExtent().height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_imguiImage);
       
        m_imguiDepthSampler = m_Renderer->CreateTextureSampler(1);
        m_Renderer->CreateImage(m_Renderer->GetSwapChainExtent().width, m_Renderer->GetSwapChainExtent().height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_imguiDepthImage);

        m_CopyCommandPool = m_Renderer->CreateCommandPool();
        m_CopyCmd = m_Renderer->AllocateCommandBuffers(m_CopyCommandPool, 1)[0];

        m_imguiImageView = m_Renderer->CreateImageView(m_imguiImage, VK_FORMAT_B8G8R8A8_UNORM, 1, VK_IMAGE_ASPECT_COLOR_BIT);
        m_imguiDepthImageView = m_Renderer->CreateImageView(m_imguiDepthImage, VK_FORMAT_D32_SFLOAT, 1, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void VulkanAdapter::RecreateSwapChain()
    {
        m_Renderer->InitDetails();
        VkSwapchainKHR old = m_SwapChain;
        m_SwapChain = m_Renderer->CreateSwapChain(m_SwapChainImages, old);
        m_Renderer->DestroySwapchain(m_Renderer->GetDevice(), old, {}, m_SwapChainImageViews);
        for (auto sema : m_Semaphores) {
            m_Renderer->DestroySemaphores(sema);
        }
        m_Renderer->DestroyFences();
        m_Renderer->ResetCurrentFrameIndex();
        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        m_DepthImages.resize(m_SwapChainImages.size());
        m_DepthImageViews.resize(m_SwapChainImages.size());

        for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {

            m_SwapChainImageViews[i] = m_Renderer->CreateImageView(m_SwapChainImages[i], m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);

            VkImage depthImage;
            m_Renderer->CreateImage(m_Renderer->GetSwapChainExtent().width, m_Renderer->GetSwapChainExtent().height, 1, VK_SAMPLE_COUNT_1_BIT, m_Renderer->FindDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage);
            VkImageView depthImageView = m_Renderer->CreateImageView(depthImage, m_Renderer->FindDepthFormat(), 1, VK_IMAGE_ASPECT_DEPTH_BIT);

            m_DepthImages[i] = depthImage;
            m_DepthImageViews[i] = depthImageView;
        }


        m_Semaphores.clear();

        for (int i = 0; i < m_Renderer->GetQueueCount(); i++) {
            m_Semaphores.emplace_back(m_Renderer->CreateSyncObjects(m_SwapChainImages));
        }

        m_CommandPoolSplash = m_Renderer->CreateCommandPool();
        m_CommandBuffersSplash = m_Renderer->AllocateCommandBuffers(m_CommandPoolSplash, static_cast<uint32_t>(m_SwapChainImageViews.size()));
        m_CommandPoolEntities = m_Renderer->CreateCommandPool();
        m_CommandBuffersEntities = m_Renderer->AllocateCommandBuffers(m_CommandPoolEntities, static_cast<uint32_t>(m_SwapChainImageViews.size()));
        m_CommandPoolBbox = m_Renderer->CreateCommandPool();
        m_CommandBuffersBbox = m_Renderer->AllocateCommandBuffers(m_CommandPoolBbox, static_cast<uint32_t>(m_SwapChainImageViews.size()));
        m_CommandPoolSkybox = m_Renderer->CreateCommandPool();
        m_CommandBuffersSkybox = m_Renderer->AllocateCommandBuffers(m_CommandPoolSkybox, static_cast<uint32_t>(m_SwapChainImageViews.size()));
        m_CommandPoolHud = m_Renderer->CreateCommandPool();
        m_CommandBuffersHud = m_Renderer->AllocateCommandBuffers(m_CommandPoolHud, static_cast<uint32_t>(m_SwapChainImageViews.size()));
    }

    void VulkanAdapter::ShouldRecreateSwapChain()
    {
        if (Poulpe::Window::m_FramebufferResized == true) {

            while (m_Window->IsMinimized()) {
                m_Window->Wait();
            }

            RecreateSwapChain();

            Poulpe::Window::m_FramebufferResized = false;
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
        m_Perspective[1][1] *= -1;
    }

    void VulkanAdapter::SetDeltatime(float deltaTime)
    {
        m_Deltatime = deltaTime;
    }

    void VulkanAdapter::DrawEntities()
    {
        if (0 < m_Entities->size()) {
            BeginRendering(m_CommandBuffersEntities[m_ImageIndex]);
            m_Renderer->StartMarker(m_CommandBuffersEntities[m_ImageIndex], "entities_drawing", 0.3, 0.2, 0.1);


            for (std::shared_ptr<Entity> entity : *m_Entities) {
                std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);

                if (!mesh) continue;

                m_Renderer->BindPipeline(m_CommandBuffersEntities[m_ImageIndex], mesh->m_GraphicsPipeline);


                // if (m_HasClicked && mesh->IsHit(m_RayPick)) {
                //    PLP_DEBUG("HIT ! {}", mesh->GetName());
                // }
                //m_HasClicked = false;

                int index = m_ImageIndex;
                for (uint32_t i = 0; i < mesh->m_UniformBuffers.size(); i++) {
                    index += i * 3;

                    if (mesh->HasPushConstants() && nullptr != mesh->ApplyPushConstants)
                        mesh->ApplyPushConstants(m_CommandBuffersEntities[m_ImageIndex], mesh->m_PipelineLayout, shared_from_this(), *mesh->GetData());

                    try {
                        m_Renderer->Draw(m_CommandBuffersEntities[m_ImageIndex], mesh->GetDescriptorSets().at(index), mesh.get(), *mesh->GetData(), mesh->GetData()->m_Ubos.size(), m_ImageIndex);
                    }
                    catch (std::exception& e) {
                        PLP_DEBUG("Draw error: {}", e.what());
                    }
                    index = m_ImageIndex;
                }
            }

            m_Renderer->EndMarker(m_CommandBuffersEntities[m_ImageIndex]);
            EndRendering(m_CommandBuffersEntities[m_ImageIndex]);

            {
                std::lock_guard<std::mutex> guard(m_MutexCmdSubmit);
                m_CmdToSubmit[1] = m_CommandBuffersEntities[m_ImageIndex];
                m_renderStatus = m_renderStatus << 1;

                if ((GetDrawBbox() && m_renderStatus == 16) || (!GetDrawBbox() && m_renderStatus == 8)) {
                    m_RenderCond.notify_one();
                }
            }
        }
    }

     void VulkanAdapter::DrawSkybox()
     {
        if (m_SkyboxMesh) {

            BeginRendering(m_CommandBuffersSkybox[m_ImageIndex], VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
            m_Renderer->StartMarker(m_CommandBuffersSkybox[m_ImageIndex], "skybox_drawing", 0.3, 0.2, 0.1);

            Poulpe::Data skyboxData = *m_SkyboxMesh->GetData();

            m_Renderer->BindPipeline(m_CommandBuffersSkybox[m_ImageIndex], m_SkyboxMesh->m_GraphicsPipeline);

            for (uint32_t i = 0; i < m_SkyboxMesh->m_UniformBuffers.size(); i++) {

                if (m_SkyboxMesh->HasPushConstants() && nullptr != m_SkyboxMesh->ApplyPushConstants)
                    m_SkyboxMesh->ApplyPushConstants(m_CommandBuffersSkybox[m_ImageIndex], m_SkyboxMesh->m_PipelineLayout, shared_from_this(), skyboxData);

                m_Renderer->Draw(m_CommandBuffersSkybox[m_ImageIndex], m_SkyboxMesh->GetDescriptorSets().at(i), m_SkyboxMesh.get(), skyboxData, skyboxData.m_Ubos.size(), m_ImageIndex, false);
            }

            m_Renderer->EndMarker(m_CommandBuffersSkybox[m_ImageIndex]);
            EndRendering(m_CommandBuffersSkybox[m_ImageIndex]);

            {
                std::lock_guard<std::mutex> guard(m_MutexCmdSubmit);
                m_CmdToSubmit[0] = m_CommandBuffersSkybox[m_ImageIndex];
                m_renderStatus = m_renderStatus << 1;

                if ((GetDrawBbox() && m_renderStatus == 16) || (!GetDrawBbox() && m_renderStatus == 8)) {
                    m_RenderCond.notify_one();
                }
            }
        }
    }

    void VulkanAdapter::DrawHUD()
    {
        BeginRendering(m_CommandBuffersHud[m_ImageIndex]);
        m_Renderer->StartMarker(m_CommandBuffersHud[m_ImageIndex], "hud_drawing", 0.3, 0.2, 0.1);

        for (std::shared_ptr<Mesh> hudPart : m_HUD) {

            if (!hudPart || !hudPart->IsVisible()) continue;
            m_Renderer->BindPipeline(m_CommandBuffersHud[m_ImageIndex], hudPart->m_GraphicsPipeline);

            if (hudPart->HasPushConstants() && nullptr != hudPart->ApplyPushConstants)
                hudPart->ApplyPushConstants(m_CommandBuffersHud[m_ImageIndex], hudPart->m_PipelineLayout, shared_from_this(), *hudPart->GetData());

            for (uint32_t i = 0; i < hudPart->m_UniformBuffers.size(); i++) {
                m_Renderer->Draw(m_CommandBuffersHud[m_ImageIndex], hudPart->GetDescriptorSets().at(i), hudPart.get(), *hudPart->GetData(), hudPart->GetData()->m_Ubos.size(), m_ImageIndex);
            }
        }

        m_Renderer->EndMarker(m_CommandBuffersHud[m_ImageIndex]);
        EndRendering(m_CommandBuffersHud[m_ImageIndex]);

        {
            std::lock_guard<std::mutex> guard(m_MutexCmdSubmit);
            m_CmdToSubmit[2] = m_CommandBuffersHud[m_ImageIndex];
            m_renderStatus = m_renderStatus << 1;

            if ((GetDrawBbox() && m_renderStatus == 16) || (!GetDrawBbox() && m_renderStatus == 8)) {
                m_RenderCond.notify_one();
            }
        }
    }

    void VulkanAdapter::DrawBbox()
    {
        if (m_Entities->size() > 0)
        {
            BeginRendering(m_CommandBuffersBbox[m_ImageIndex]);
            m_Renderer->StartMarker(m_CommandBuffersBbox[m_ImageIndex], "bbox_drawing", 0.3, 0.2, 0.1);

            for (std::shared_ptr<Entity> entity : *m_Entities) {
                std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);

                if (!mesh || !mesh->HasBbox()) continue;
                auto&& bbox = std::dynamic_pointer_cast<Mesh>(mesh->GetBBox()->mesh);

                if (!bbox) continue;

                m_Renderer->BindPipeline(m_CommandBuffersBbox[m_ImageIndex], bbox->m_GraphicsPipeline);

                if (m_HasClicked && mesh->IsHit(m_RayPick)) {
                    PLP_DEBUG("HIT ! {}", mesh->GetName());
                }
                m_HasClicked = false;

                int index = m_ImageIndex;
                for (uint32_t i = 0; i < bbox->m_UniformBuffers.size(); i++) {
                    index += i * 3;

                    if (bbox->HasPushConstants() && nullptr != bbox->ApplyPushConstants)
                        bbox->ApplyPushConstants(m_CommandBuffersBbox[m_ImageIndex], bbox->m_PipelineLayout, shared_from_this(), *bbox->GetData());

                    m_Renderer->Draw(m_CommandBuffersBbox[m_ImageIndex], bbox->GetDescriptorSets().at(index), bbox.get(), *bbox->GetData(), mesh->GetData()->m_Ubos.size(), m_ImageIndex);
                    index = m_ImageIndex;
                }
            }

            m_Renderer->EndMarker(m_CommandBuffersBbox[m_ImageIndex]);
            EndRendering(m_CommandBuffersBbox[m_ImageIndex]);

            {
                std::lock_guard<std::mutex> guard(m_MutexCmdSubmit);
                m_CmdToSubmit[3] = m_CommandBuffersBbox[m_ImageIndex];

                m_renderStatus = m_renderStatus << 1;
                if ((GetDrawBbox() && m_renderStatus == 16) || (!GetDrawBbox() && m_renderStatus == 8)) {
                    m_RenderCond.notify_one();
                }
            }
        }
    }

    void VulkanAdapter::RenderScene()
    {
        {
            std::unique_lock<std::mutex> render(m_MutexRenderScene);
            m_CmdToSubmit.resize(4);
            m_renderStatus = 1;

            std::string_view threadQueueName{ "render" };

            Poulpe::Locator::getThreadPool()->Submit(threadQueueName, [=, this]() { DrawSkybox(); });
            Poulpe::Locator::getThreadPool()->Submit(threadQueueName, [=, this]() { DrawHUD(); });
            Poulpe::Locator::getThreadPool()->Submit(threadQueueName, [=, this]() { DrawEntities(); });

            //@todo strip for release?
            if (GetDrawBbox()) {
                Poulpe::Locator::getThreadPool()->Submit(threadQueueName, [=, this] {
                    DrawBbox();
                });
            }
        }
    }

    void VulkanAdapter::AddCmdToSubmit(VkCommandBuffer cmd)
    {
        m_moreCmdToSubmit.emplace_back(cmd);
    }

    void VulkanAdapter::Draw()
    {
        if (m_RenderingStopped) {
            VkCommandBufferResetFlags flags = VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT;

            vkResetCommandBuffer(m_CommandBuffersEntities[m_ImageIndex], flags);
            vkResetCommandBuffer(m_CommandBuffersHud[m_ImageIndex], flags);
            vkResetCommandBuffer(m_CommandBuffersSkybox[m_ImageIndex], flags);

            if (GetDrawBbox()) {
              vkResetCommandBuffer(m_CommandBuffersBbox[m_ImageIndex], flags);
            }

            m_RenderingStopped = false;

            return;
        }

        {
            std::unique_lock<std::mutex> render(m_MutexRenderScene);

            m_RenderCond.wait(render, [=, this] {
                return  (GetDrawBbox()) ? m_renderStatus == 16 : m_renderStatus == 8;
            });
            {
                std::unique_lock<std::mutex> render(m_MutexCmdSubmit);
                std::vector<VkCommandBuffer> cmds{};
                std::copy_if(
                    m_CmdToSubmit.begin(), m_CmdToSubmit.end(),
                    std::back_inserter(cmds),
                    [](VkCommandBuffer vkBuffer) {
                        return vkBuffer != VK_NULL_HANDLE;
                    }
                );

                if (m_moreCmdToSubmit.size() > 0) {
                   std::copy(m_moreCmdToSubmit.begin(), m_moreCmdToSubmit.end(), std::back_inserter(cmds)); 
                }

                Submit(cmds);
                Present();

                m_CmdToSubmit.clear();
                m_moreCmdToSubmit.clear();
                cmds.clear();

                //@todo wtf ?
                uint32_t currentFrame = m_Renderer->GetNextFrameIndex();
                if (currentFrame == VK_ERROR_OUT_OF_DATE_KHR || currentFrame == VK_SUBOPTIMAL_KHR) {
                    RecreateSwapChain();
                }
            }

            AcquireNextImage();
        }
    }

    void VulkanAdapter::DrawSplashScreen()
    {
        BeginRendering(m_CommandBuffersSplash[m_ImageIndex], VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_NONE_KHR);

        for (std::shared_ptr<Mesh> mesh : m_Splash) {

            if (!mesh || !mesh->IsVisible()) continue;
            m_Renderer->BindPipeline(m_CommandBuffersSplash[m_ImageIndex], mesh->m_GraphicsPipeline);

            for (uint32_t i = 0; i < mesh->m_UniformBuffers.size(); i++) {

                if (mesh->HasPushConstants() && nullptr != mesh->ApplyPushConstants)
                    mesh->ApplyPushConstants(m_CommandBuffersSplash[m_ImageIndex], mesh->m_PipelineLayout, shared_from_this(), *mesh->GetData());

                m_Renderer->Draw(m_CommandBuffersSplash[m_ImageIndex], mesh->GetDescriptorSets().at(i), mesh.get(), *mesh->GetData(), mesh->GetData()->m_Ubos.size(), m_ImageIndex);
            }
        }

        EndRendering(m_CommandBuffersSplash[m_ImageIndex]);

        Submit({ m_CommandBuffersSplash[m_ImageIndex] });
        Present();

        //@todo wtf ?
        uint32_t currentFrame = m_Renderer->GetNextFrameIndex();
        if (currentFrame == VK_ERROR_OUT_OF_DATE_KHR || currentFrame == VK_SUBOPTIMAL_KHR) {
            RecreateSwapChain();
        }
        AcquireNextImage();
    }

    void VulkanAdapter::ClearSplashScreen()
    {
        BeginRendering(m_CommandBuffersSplash[m_ImageIndex], VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_NONE_KHR);
        //do nothing !
        EndRendering(m_CommandBuffersSplash[m_ImageIndex]);

        Submit({ m_CommandBuffersSplash[m_ImageIndex] });
        Present();

        //@todo wtf ?
        uint32_t currentFrame = m_Renderer->GetNextFrameIndex();
        if (currentFrame == VK_ERROR_OUT_OF_DATE_KHR || currentFrame == VK_SUBOPTIMAL_KHR) {
            RecreateSwapChain();
        }

        AcquireNextImage();
    }

    void VulkanAdapter::ClearRendererScreen()
    {
        {
            std::unique_lock<std::mutex> render(m_MutexRenderScene);

            m_RenderCond.wait(render, [=, this] {
                return  (GetDrawBbox()) ? m_renderStatus == 16 : m_renderStatus == 8;
                });

            BeginRendering(m_CommandBuffersEntities[m_ImageIndex]);
            //do nothing !
            EndRendering(m_CommandBuffersEntities[m_ImageIndex]);

            BeginRendering(m_CommandBuffersHud[m_ImageIndex]);
            //do nothing !
            EndRendering(m_CommandBuffersHud[m_ImageIndex]);

            BeginRendering(m_CommandBuffersSkybox[m_ImageIndex], VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
            //do nothing !
            EndRendering(m_CommandBuffersSkybox[m_ImageIndex]);


            std::vector<VkCommandBuffer> cmds{};
            cmds.emplace_back(m_CommandBuffersSkybox[m_ImageIndex]);
            cmds.emplace_back(m_CommandBuffersEntities[m_ImageIndex]);
            cmds.emplace_back(m_CommandBuffersHud[m_ImageIndex]);

            if (GetDrawBbox()) {
                BeginRendering(m_CommandBuffersSkybox[m_ImageIndex]);
                //do nothing !
                EndRendering(m_CommandBuffersBbox[m_ImageIndex]);
                cmds.emplace_back(m_CommandBuffersBbox[m_ImageIndex]);
            }

            m_renderStatus = 1;

            Submit(cmds);
            Present();

            //@todo wtf ?
            uint32_t currentFrame = m_Renderer->GetNextFrameIndex();
            if (currentFrame == VK_ERROR_OUT_OF_DATE_KHR || currentFrame == VK_SUBOPTIMAL_KHR) {
                RecreateSwapChain();
            }

            AcquireNextImage();
        }
    }

    void VulkanAdapter::Destroy()
    {
        m_Renderer->DestroySwapchain(m_Renderer->GetDevice(), m_SwapChain, {}, m_SwapChainImageViews);

        for (auto sema : m_Semaphores) {
            m_Renderer->DestroySemaphores(sema);
        }

        m_Renderer->DestroyFences();

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

        m_Renderer->DestroyRenderPass(m_RenderPass, m_CommandPoolSplash, m_CommandBuffersSplash);
        m_Renderer->DestroyRenderPass(m_RenderPass, m_CommandPoolEntities, m_CommandBuffersEntities);
        m_Renderer->DestroyRenderPass(m_RenderPass, m_CommandPoolBbox, m_CommandBuffersBbox);
        m_Renderer->DestroyRenderPass(m_RenderPass, m_CommandPoolSkybox, m_CommandBuffersSkybox);
        m_Renderer->DestroyRenderPass(m_RenderPass, m_CommandPoolHud, m_CommandBuffersHud);
    }

    void VulkanAdapter::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function, int queueIndex)
    {
        auto commandPool = m_Renderer->CreateCommandPool();
        VkCommandBuffer cmd = m_Renderer->AllocateCommandBuffers(commandPool)[0];
        m_Renderer->BeginCommandBuffer(cmd);
        function(cmd);
        m_Renderer->EndCommandBuffer(cmd);
        m_Renderer->QueueSubmit(cmd);
        vkDestroyCommandPool(m_Renderer->GetDevice(), commandPool, nullptr);
    }

    void VulkanAdapter::BeginRendering(VkCommandBuffer commandBuffer, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
    {
        m_Renderer->BeginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        VkImageMemoryBarrier swapChainImageRenderBeginBarrier = m_Renderer->SetupImageMemoryBarrier(
            m_SwapChainImages[m_ImageIndex],
            0,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        );

        m_Renderer->AddPipelineBarriers(
            commandBuffer,
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
            commandBuffer,
            { depthImageRenderBeginBarrier },
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        );

        m_Renderer->BeginRendering(commandBuffer, m_SwapChainImageViews[m_ImageIndex], m_DepthImageViews[m_ImageIndex], loadOp, storeOp);

        m_Renderer->SetViewPort(commandBuffer);
        m_Renderer->SetScissor(commandBuffer);
    }

    void VulkanAdapter::EndRendering(VkCommandBuffer commandBuffer)
    {
        m_Renderer->EndRendering(commandBuffer);

        VkImageMemoryBarrier swapChainImageEndRenderBeginBarrier = m_Renderer->SetupImageMemoryBarrier(
            m_SwapChainImages[m_ImageIndex],
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            0,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        );
        m_Renderer->AddPipelineBarriers(
            commandBuffer,
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
            commandBuffer,
            { depthImageEndRenderBeginBarrier },
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0
        );

        m_Renderer->EndCommandBuffer(commandBuffer);
    }

    void VulkanAdapter::Submit(std::vector<VkCommandBuffer> commandBuffers, int queueIndex)
    {
        OnFinishRender();

        VkResult submitResult = m_Renderer->QueueSubmit(m_ImageIndex, commandBuffers, m_Semaphores.at(queueIndex), queueIndex);

        if (submitResult != VK_SUCCESS) {
            PLP_WARN("Error on queue submit: {}", submitResult);
            RecreateSwapChain();
            return;
        }
    }

    void VulkanAdapter::Present(int queueIndex)
    {
      //std::vector<VkSemaphore>& renderFinishedSemaphores = m_Semaphores.at(queueIndex).second;
      //VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[m_ImageIndex] };

  
      //VkSemaphoreTypeCreateInfo sema;
      //sema.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
      //sema.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
      //sema.pNext = NULL;

      //VkSemaphoreWaitInfo waitInfo;
      //waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
      //waitInfo.pNext = NULL;
      //waitInfo.flags = 0;
      //waitInfo.semaphoreCount = 1;
      //waitInfo.pSemaphores = { &m_Semaphores.at(queueIndex).second[m_ImageIndex]};

      //vkWaitSemaphores(m_Renderer->GetDevice(), &waitInfo, UINT64_MAX);

        VkResult presentResult = m_Renderer->QueuePresent(m_ImageIndex, m_SwapChain, m_Semaphores.at(queueIndex), queueIndex);

        if (presentResult != VK_SUCCESS) {
            PLP_WARN("Error on queue present: {}", presentResult);
            RecreateSwapChain();
        }
    }

    void VulkanAdapter::AcquireNextImage()
    {
      m_ImageIndex = m_Renderer->AcquireNextImageKHR(m_SwapChain, m_Semaphores.at(0));
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
        m_SkyboxMesh = nullptr;
        m_HUD.clear();
        m_Splash.clear();
    }

    void VulkanAdapter::AddEntities(std::vector<std::shared_ptr<Entity>>* entities)
    {
        m_Entities = entities;
    }

    void VulkanAdapter::OnFinishRender()
    {
        m_Renderer->BeginCommandBuffer(m_CopyCmd);

        VkImageCopy imageCopyRegion{};
        imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.srcSubresource.layerCount = 1;
        imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.dstSubresource.layerCount = 1;
        imageCopyRegion.extent.width = m_Renderer->GetSwapChainExtent().width;
        imageCopyRegion.extent.height = m_Renderer->GetSwapChainExtent().height;
        imageCopyRegion.extent.depth = 1;

        VkImageMemoryBarrier copyImageRenderBeginBarrier = m_Renderer->SetupImageMemoryBarrier(
          m_imguiImage,
          VK_ACCESS_TRANSFER_WRITE_BIT,
          VK_ACCESS_MEMORY_READ_BIT,
          VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );

        m_Renderer->AddPipelineBarriers(
          m_CopyCmd,
          { copyImageRenderBeginBarrier },
          VK_PIPELINE_STAGE_TRANSFER_BIT,
          VK_PIPELINE_STAGE_TRANSFER_BIT,
          0
        );

        VkImageMemoryBarrier copyImageSrcBeginBarrier = m_Renderer->SetupImageMemoryBarrier(
          m_SwapChainImages[m_ImageIndex],
          VK_ACCESS_MEMORY_READ_BIT,
          VK_ACCESS_TRANSFER_READ_BIT,
          VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
        );

        m_Renderer->AddPipelineBarriers(
          m_CopyCmd,
          { copyImageSrcBeginBarrier },
          VK_PIPELINE_STAGE_TRANSFER_BIT,
          VK_PIPELINE_STAGE_TRANSFER_BIT,
          VK_DEPENDENCY_BY_REGION_BIT
        );

        vkCmdCopyImage(
          m_CopyCmd,
          m_SwapChainImages[m_ImageIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
          m_imguiImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          1,
          &imageCopyRegion);

        VkImageMemoryBarrier copyImageRenderEndBarrier = m_Renderer->SetupImageMemoryBarrier(
          m_imguiImage,
          VK_ACCESS_TRANSFER_WRITE_BIT,
          VK_ACCESS_MEMORY_READ_BIT,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        m_Renderer->AddPipelineBarriers(
          m_CopyCmd,
          { copyImageRenderEndBarrier },
          VK_PIPELINE_STAGE_TRANSFER_BIT,
          VK_PIPELINE_STAGE_TRANSFER_BIT,
          0
        );

        VkImageMemoryBarrier srcImageRenderEndBarrier = m_Renderer->SetupImageMemoryBarrier(
          m_SwapChainImages[m_ImageIndex],
          VK_ACCESS_TRANSFER_READ_BIT,
          VK_ACCESS_MEMORY_READ_BIT,
          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
          VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        );

        m_Renderer->AddPipelineBarriers(
          m_CopyCmd,
          { srcImageRenderEndBarrier },
          VK_PIPELINE_STAGE_TRANSFER_BIT,
          VK_PIPELINE_STAGE_TRANSFER_BIT,
          0
        );

        m_Renderer->EndCommandBuffer(m_CopyCmd);
        m_Renderer->QueueSubmit(m_CopyCmd);

        //m_Renderer->BeginCommandBuffer(m_CopyCmd);

        //VkDeviceSize imageSize = m_Renderer->GetSwapChainExtent().width * m_Renderer->GetSwapChainExtent().height * 4;
        //VkMemoryPropertyFlags properties;
        //VkBuffer buffer;
        //VkDeviceMemory bufferMemory;
        //m_Renderer->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);

        //VkImageSubresourceLayers srcImgsubrcLayers;
        //srcImgsubrcLayers.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        //srcImgsubrcLayers.mipLevel = 0;
        //srcImgsubrcLayers.baseArrayLayer = 0;
        //srcImgsubrcLayers.layerCount = 1;

        //VkBufferImageCopy srcImgcopy;
        //srcImgcopy.bufferOffset = 0;
        //srcImgcopy.bufferRowLength = 0;
        //srcImgcopy.bufferImageHeight = 0;
        //srcImgcopy.imageSubresource = srcImgsubrcLayers;
        //srcImgcopy.imageOffset = { 0, 0, 0 };
        //srcImgcopy.imageExtent = {
        //  m_Renderer->GetSwapChainExtent().width,
        //  m_Renderer->GetSwapChainExtent().height,
        //  1
        //};

        //VkImageCopy depthImageCopyRegion{};
        //depthImageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //depthImageCopyRegion.srcSubresource.layerCount = 1;
        //depthImageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //depthImageCopyRegion.dstSubresource.layerCount = 1;
        //depthImageCopyRegion.extent.width = m_Renderer->GetSwapChainExtent().width;
        //depthImageCopyRegion.extent.height = m_Renderer->GetSwapChainExtent().height;
        //depthImageCopyRegion.extent.depth = 1;
        //
        //VkImageMemoryBarrier copyDepthImageRenderBeginBarrier = m_Renderer->SetupImageMemoryBarrier(
        //  m_imguiDepthImage,
        //  VK_ACCESS_TRANSFER_WRITE_BIT,
        //  VK_ACCESS_MEMORY_READ_BIT,
        //  VK_IMAGE_LAYOUT_UNDEFINED,
        //  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        //);

        //m_Renderer->AddPipelineBarriers(
        //  m_CopyCmd,
        //  { copyDepthImageRenderBeginBarrier },
        //  VK_PIPELINE_STAGE_TRANSFER_BIT,
        //  VK_PIPELINE_STAGE_TRANSFER_BIT,
        //  0
        //);

        //VkImageMemoryBarrier copyDepthImageSrcBeginBarrier = m_Renderer->SetupImageMemoryBarrier(
        //  m_DepthImages[m_ImageIndex],
        //  VK_ACCESS_MEMORY_READ_BIT,
        //  VK_ACCESS_TRANSFER_READ_BIT,
        //  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        //  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
        //);

        //m_Renderer->AddPipelineBarriers(
        //  m_CopyCmd,
        //  { copyDepthImageSrcBeginBarrier },
        //  VK_PIPELINE_STAGE_TRANSFER_BIT,
        //  VK_PIPELINE_STAGE_TRANSFER_BIT,
        //  VK_DEPENDENCY_BY_REGION_BIT
        //);

        //vkCmdCopyImageToBuffer(m_CopyCmd, m_DepthImages[m_ImageIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &srcImgcopy);
        //vkCmdCopyBufferToImage(m_CopyCmd, buffer, m_imguiDepthImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &srcImgcopy);

        //VkImageMemoryBarrier copyDepthImageRenderEndBarrier = m_Renderer->SetupImageMemoryBarrier(
        //  m_imguiDepthImage,
        //  VK_ACCESS_TRANSFER_WRITE_BIT,
        //  VK_ACCESS_MEMORY_READ_BIT,
        //  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        //  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        //);

        //m_Renderer->AddPipelineBarriers(
        //  m_CopyCmd,
        //  { copyDepthImageRenderEndBarrier },
        //  VK_PIPELINE_STAGE_TRANSFER_BIT,
        //  VK_PIPELINE_STAGE_TRANSFER_BIT,
        //  0
        //);

        //VkImageMemoryBarrier srcDepthImageRenderEndBarrier = m_Renderer->SetupImageMemoryBarrier(
        //  m_DepthImages[m_ImageIndex],
        //  VK_ACCESS_TRANSFER_READ_BIT,
        //  VK_ACCESS_MEMORY_READ_BIT,
        //  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        //  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        //);

        //m_Renderer->AddPipelineBarriers(
        //  m_CopyCmd,
        //  { srcDepthImageRenderEndBarrier },
        //  VK_PIPELINE_STAGE_TRANSFER_BIT,
        //  VK_PIPELINE_STAGE_TRANSFER_BIT,
        //  0
        //);


        //m_Renderer->EndCommandBuffer(m_CopyCmd);
        //m_Renderer->QueueSubmit(m_CopyCmd);

        //vkDestroyBuffer(m_Renderer->GetDevice(), buffer, nullptr);
        //vkFreeMemory(m_Renderer->GetDevice(), bufferMemory, nullptr);

        Event event{ "OnFinishRender" };
        for (const auto& observer : m_Observers) {
            observer->Notify(event);
        }
    }

    void VulkanAdapter::AttachObserver(IObserver* observer)
    {
        m_Observers.push_back(observer);
    }

    VkRenderPass VulkanAdapter::CreateImGuiRenderPass(VkFormat format)
    {
        VkRenderPass renderPass;

        VkAttachmentDescription attachment = {};
        attachment.format = format;
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
            PLP_FATAL("failed to create imgui render pass : {}", result);
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

        VkDescriptorPool imguiPool = m_Renderer->CreateDescriptorPool(poolSizes, 1000, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

        ImGui_ImplVulkan_InitInfo info = {};

        info.Instance = m_Renderer->GetInstance();
        info.PhysicalDevice = m_Renderer->GetPhysicalDevice();
        info.Device = m_Renderer->GetDevice();
        info.QueueFamily = m_Renderer->GetQueueFamily();
        info.Queue = m_Renderer->GetGraphicsQueues()[0];
        info.PipelineCache = nullptr;//to implement VkPipelineCache
        info.DescriptorPool = std::move(imguiPool);
        info.Subpass = 0;
        info.MinImageCount = 3;
        info.ImageCount = 3;
        info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        info.Allocator = nullptr;
        info.CheckVkResultFn = [](VkResult err) {
            if (0 == err) return;
            PLP_FATAL("ImGui error {}", err);
        };

        auto commandPool = m_Renderer->CreateCommandPool();

        VkRenderPass renderPass;

        const SwapChainSupportDetails swapChainDetails = m_Renderer->QuerySwapChainSupport(m_Renderer->GetPhysicalDevice());
        VkSurfaceFormatKHR imguiFormat{};
        
        for (const auto& availableFormat : swapChainDetails.formats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                imguiFormat = availableFormat;
                break;
            }
        }

        auto rdrPass = CreateImGuiRenderPass(imguiFormat.format);

        ImGuiInfo imGuiInfo;
        imGuiInfo.info = info;
        imGuiInfo.rdrPass = std::move(rdrPass);
        imGuiInfo.cmdBuffer = m_Renderer->AllocateCommandBuffers(commandPool)[0];
        //imGuiInfo.pipeline = m_Pipelines[0].graphicsPipeline;
        imGuiInfo.width = m_Renderer->GetSwapChainExtent().width;
        imGuiInfo.height = m_Renderer->GetSwapChainExtent().height;

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
    //void VulkanAdapter::RenderForImGui(VkCommandBuffer cmdBuffer, VkFramebuffer swapChainFramebuffer)
    //{
    //    auto renderPass = m_Renderer->CreateRenderPass(VK_SAMPLE_COUNT_1_BIT);

    //    VkRenderPassBeginInfo renderPassInfo{};
    //    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    //    renderPassInfo.renderPass = *renderPass.get();
    //    renderPassInfo.framebuffer = swapChainFramebuffer;
    //    renderPassInfo.renderArea.offset = { 0, 0 };
    //    renderPassInfo.renderArea.extent = m_Renderer->GetSwapChainExtent();

    //    std::array<VkClearValue, 2> clearValues{};
    //    clearValues[0].color = { {0.f, 1.f, 0.f, 1.0f} };
    //    clearValues[1].depthStencil = { 1.0f, 0 };

    //    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    //    renderPassInfo.pClearValues = clearValues.data();

    //    vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    //    VkViewport viewport{};
    //    viewport.x = 0.0f;
    //    viewport.y = 0.0f;
    //    viewport.width = static_cast<float>(m_Renderer->GetSwapChainExtent().width);
    //    viewport.height = static_cast<float>(m_Renderer->GetSwapChainExtent().height);
    //    viewport.minDepth = 0.0f;
    //    viewport.maxDepth = 1.0f;

    //    VkRect2D scissor{};
    //    scissor.offset = { 0, 0 };
    //    scissor.extent = m_Renderer->GetSwapChainExtent();

    //    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    //    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

    //    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.textured, 0, 1, &descriptorSets.mirror, 0, nullptr);
    //    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.mirror);
    //    m_Renderer->Draw(cmdBuffer, mesh->GetDescriptorSets().at(index), mesh.get(), *mesh->GetData(), mesh->GetData()->m_Ubos.size(), m_ImageIndex);
   
    //    vkCmdEndRenderPass(cmdBuffer);
    //}
}