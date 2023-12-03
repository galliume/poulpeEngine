#include "VulkanAdapter.hpp"

#include "Poulpe/GUI/Window.hpp"
#include "Poulpe/Renderer/Vulkan/VulkanRenderer.hpp"

#include <cfenv>
#include <exception>
#include <future>
#include <memory>
#include <volk.h>

namespace Poulpe
{
    //@todo should not be globally accessible
    std::atomic<float> VulkanAdapter::s_AmbiantLight{ 1.0f };
    std::atomic<float> VulkanAdapter::s_FogDensity{ 0.0f };
    std::atomic<float> VulkanAdapter::s_FogColor[3]{ 25 / 255.0f, 25 / 255.0f, 25 / 255.0f };
    std::atomic<int> VulkanAdapter::s_Crosshair{ 0 };
    std::atomic<int> VulkanAdapter::s_PolygoneMode{ VK_POLYGON_MODE_FILL };
    
    VulkanAdapter::VulkanAdapter(Window* window, EntityManager* entityManager, ComponentManager* componentManager)
        : m_Window(window), m_EntityManager(entityManager), m_ComponentManager(componentManager)
    {
        m_Renderer = std::make_unique<VulkanRenderer>(window);
    }

    void VulkanAdapter::init()
    {
        m_RayPick = glm::vec3(0.0f);
        m_LightsPos.emplace_back(glm::vec3(0.5f, 0.5f, 0.5f));
        setPerspective();
        m_RenderPass = std::unique_ptr<VkRenderPass>(m_Renderer->createRenderPass(m_Renderer->getMsaaSamples()));
        
//#ifndef  PLP_DEBUG_BUILD
//        m_SwapChain = m_Renderer->CreateSwapChain(m_SwapChainImages);
//#else
        m_SwapChainImages.resize(3);
        m_SwapChainSamplers.resize(3);
        m_SwapChainDepthSamplers.resize(3);

        for (size_t i = 0; i < m_SwapChainImages.size(); ++i) {
          VkImage image;
          //VkImage depthImage;

          m_Renderer->createImage(m_Renderer->getSwapChainExtent().width, m_Renderer->getSwapChainExtent().height, 1,
              VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT 
              | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image);

          m_SwapChainImages[i] = image;

          m_SwapChainSamplers[i] = m_Renderer->createTextureSampler(1);
          m_SwapChainDepthSamplers[i] = m_Renderer->createTextureSampler(1);;
        }
//#endif
        //init swap chain, depth image views, primary command buffers and semaphores
        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        m_DepthImages.resize(m_SwapChainImages.size());
        m_DepthImageViews.resize(m_SwapChainImages.size());

        m_CommandPoolEntities = m_Renderer->createCommandPool();

        m_CommandBuffersEntities = m_Renderer->allocateCommandBuffers(m_CommandPoolEntities,
            static_cast<uint32_t>(m_SwapChainImageViews.size()));

        m_CommandPoolBbox = m_Renderer->createCommandPool();
        m_CommandBuffersBbox = m_Renderer->allocateCommandBuffers(m_CommandPoolBbox,
            static_cast<uint32_t>(m_SwapChainImageViews.size()));

        m_CommandPoolSkybox = m_Renderer->createCommandPool();
        m_CommandBuffersSkybox = m_Renderer->allocateCommandBuffers(m_CommandPoolSkybox,
            static_cast<uint32_t>(m_SwapChainImageViews.size()));

        m_CommandPoolHud = m_Renderer->createCommandPool();
        m_CommandBuffersHud = m_Renderer->allocateCommandBuffers(m_CommandPoolHud,
            static_cast<uint32_t>(m_SwapChainImageViews.size()));

        for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
            m_SwapChainImageViews[i] = m_Renderer->createImageView(m_SwapChainImages[i],
                m_Renderer->getSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);

            VkImage depthImage;
            m_Renderer->createImage(m_Renderer->getSwapChainExtent().width, m_Renderer->getSwapChainExtent().height, 1,
                VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT
                | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage);

            VkImageView depthImageView = m_Renderer->createImageView(depthImage, VK_FORMAT_D32_SFLOAT, 1,
                VK_IMAGE_ASPECT_DEPTH_BIT);

            m_DepthImages[i] = depthImage;
            m_DepthImageViews[i] = depthImageView;
        }

        for (size_t i = 0; i < m_Renderer->getQueueCount(); i++) {
            m_Semaphores.emplace_back(m_Renderer->createSyncObjects(m_SwapChainImages));
        }

        acquireNextImage();
    }

    void VulkanAdapter::recreateSwapChain()
    {
        m_Renderer->initDetails();
        VkSwapchainKHR old = m_SwapChain;

        m_Renderer->destroySwapchain(m_Renderer->getDevice(), old, {}, m_SwapChainImageViews);

        for (auto sema : m_Semaphores) {
            m_Renderer->destroySemaphores(sema);
        }
        m_Renderer->destroyFences();
        m_Renderer->resetCurrentFrameIndex();

//#ifndef  PLP_DEBUG_BUILD
//        m_SwapChain = m_Renderer->CreateSwapChain(m_SwapChainImages, old);
//#else
        m_SwapChainImages.resize(3);
        m_SwapChainSamplers.resize(3);
        m_SwapChainDepthSamplers.resize(3);

        for (size_t i = 0; i < m_SwapChainImages.size(); ++i) {
          VkImage image;
          //VkImage depthImage;

          m_Renderer->createImage(m_Renderer->getSwapChainExtent().width, m_Renderer->getSwapChainExtent().height, 1,
              VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT
              | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image);

          m_SwapChainImages[i] = image;

          m_SwapChainSamplers[i] = m_Renderer->createTextureSampler(1);
          m_SwapChainDepthSamplers[i] = m_Renderer->createTextureSampler(1);;
        }
//#endif

        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        m_DepthImages.resize(m_SwapChainImages.size());
        m_DepthImageViews.resize(m_SwapChainImages.size());

        for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
            m_SwapChainImageViews[i] = m_Renderer->createImageView(m_SwapChainImages[i],
                m_Renderer->getSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);

            VkImage depthImage;
            m_Renderer->createImage(m_Renderer->getSwapChainExtent().width, m_Renderer->getSwapChainExtent().height, 1,
                VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT
                | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage);

            VkImageView depthImageView = m_Renderer->createImageView(depthImage, VK_FORMAT_D32_SFLOAT, 1,
                VK_IMAGE_ASPECT_DEPTH_BIT);

            m_DepthImages[i] = depthImage;
            m_DepthImageViews[i] = depthImageView;
        }

        m_Semaphores.clear();

        for (size_t i = 0; i < m_Renderer->getQueueCount(); i++) {
            m_Semaphores.emplace_back(m_Renderer->createSyncObjects(m_SwapChainImages));
        }

        m_CommandPoolEntities = m_Renderer->createCommandPool();
        m_CommandBuffersEntities = m_Renderer->allocateCommandBuffers(m_CommandPoolEntities,
            static_cast<uint32_t>(m_SwapChainImageViews.size()));

        m_CommandPoolBbox = m_Renderer->createCommandPool();
        m_CommandBuffersBbox = m_Renderer->allocateCommandBuffers(m_CommandPoolBbox,
            static_cast<uint32_t>(m_SwapChainImageViews.size()));

        m_CommandPoolSkybox = m_Renderer->createCommandPool();
        m_CommandBuffersSkybox = m_Renderer->allocateCommandBuffers(m_CommandPoolSkybox,
            static_cast<uint32_t>(m_SwapChainImageViews.size()));

        m_CommandPoolHud = m_Renderer->createCommandPool();
        m_CommandBuffersHud = m_Renderer->allocateCommandBuffers(m_CommandPoolHud,
            static_cast<uint32_t>(m_SwapChainImageViews.size()));

        acquireNextImage();
    }

    void VulkanAdapter::shouldRecreateSwapChain()
    {
        if (Poulpe::Window::m_FramebufferResized == true) {

            while (m_Window->isMinimized()) {
                m_Window->wait();
            }
            recreateSwapChain();

            Poulpe::Window::m_FramebufferResized = false;
        }
    }

    void VulkanAdapter::setPerspective()
    {        
        m_Perspective = glm::perspective(glm::radians(45.0f),
            static_cast<float>(m_Renderer->getSwapChainExtent().width) / static_cast<float>(m_Renderer->getSwapChainExtent().height),
            0.1f, 100.f);
        m_Perspective[1][1] *= -1;
    }

    void VulkanAdapter::setDeltatime(float deltaTime)
    {
        m_Deltatime = deltaTime;
    }

    void VulkanAdapter::drawEntities()
    {
        if (0 < m_EntityManager->getEntities()->size()) {
            beginRendering(m_CommandBuffersEntities[m_ImageIndex]);
            m_Renderer->startMarker(m_CommandBuffersEntities[m_ImageIndex], "entities_drawing", 0.3, 0.2, 0.1);


            for (auto & entity : *m_EntityManager->getEntities()) {
                Mesh* mesh = entity->getMesh();

                if (!mesh) continue;

                m_Renderer->bindPipeline(m_CommandBuffersEntities[m_ImageIndex], mesh->getGraphicsPipeline());

                // if (m_HasClicked && mesh->IsHit(m_RayPick)) {
                //    PLP_DEBUG("HIT ! {}", mesh->GetName());
                // }
                //m_HasClicked = false;

                int index = m_ImageIndex;
                for (uint32_t i = 0; i < mesh->getUniformBuffers()->size(); i++) {
                    index += i * 3;

                    if (mesh->hasPushConstants() && nullptr != mesh->applyPushConstants)
                        mesh->applyPushConstants(m_CommandBuffersEntities[m_ImageIndex], mesh->getPipelineLayout(),
                            this, mesh);

                    try {
                        if (m_RenderingStopped) return;
                        m_Renderer->draw(m_CommandBuffersEntities[m_ImageIndex], mesh->getDescriptorSets().at(index),
                            mesh, mesh->getData(), mesh->getData()->m_Ubos.size(), mesh->isIndexed());
                    }
                    catch (std::exception & e) {
                        PLP_DEBUG("Draw error: {}", e.what());
                    }
                    index = m_ImageIndex;
                }
            }

            m_Renderer->endMarker(m_CommandBuffersEntities[m_ImageIndex]);
            endRendering(m_CommandBuffersEntities[m_ImageIndex]);
            {
                std::lock_guard<std::mutex> guard(m_MutexCmdSubmit);
                m_CmdToSubmit[1] = m_CommandBuffersEntities[m_ImageIndex];
                m_renderStatus = m_renderStatus << 1;

                if ((getDrawBbox() && m_renderStatus == 16) || (!getDrawBbox() && m_renderStatus == 8)) {
                    m_RenderCond.notify_one();
                }
            }
        }
    }

     void VulkanAdapter::drawSkybox()
     {
        if (auto skybox = m_EntityManager->getSkybox()) {

            beginRendering(m_CommandBuffersSkybox[m_ImageIndex], VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
            m_Renderer->startMarker(m_CommandBuffersSkybox[m_ImageIndex], "skybox_drawing", 0.3, 0.2, 0.1);

            Mesh::Data* skyboxData = skybox->getMesh()->getData();

            m_Renderer->bindPipeline(m_CommandBuffersSkybox[m_ImageIndex], skybox->getMesh()->getGraphicsPipeline());

            for (uint32_t i = 0; i < skybox->getMesh()->getUniformBuffers()->size(); i++) {

                if (skybox->getMesh()->hasPushConstants() && nullptr != skybox->getMesh()->applyPushConstants)
                    skybox->getMesh()->applyPushConstants(m_CommandBuffersSkybox[m_ImageIndex], skybox->getMesh()->getPipelineLayout(), this,
                        skybox->getMesh());

                if (m_RenderingStopped) return;
                m_Renderer->draw(m_CommandBuffersSkybox[m_ImageIndex], skybox->getMesh()->getDescriptorSets().at(i), skybox->getMesh(),
                    skyboxData, skyboxData->m_Ubos.size(), false);
            }

            m_Renderer->endMarker(m_CommandBuffersSkybox[m_ImageIndex]);
            endRendering(m_CommandBuffersSkybox[m_ImageIndex]);

            {
                std::lock_guard<std::mutex> guard(m_MutexCmdSubmit);
                m_CmdToSubmit[0] = m_CommandBuffersSkybox[m_ImageIndex];
                m_renderStatus = m_renderStatus << 1;

                if ((getDrawBbox() && m_renderStatus == 16) || (!getDrawBbox() && m_renderStatus == 8)) {
                    m_RenderCond.notify_one();
                }
            }
        }
    }

    void VulkanAdapter::drawHUD()
    {
        beginRendering(m_CommandBuffersHud[m_ImageIndex]);
        m_Renderer->startMarker(m_CommandBuffersHud[m_ImageIndex], "hud_drawing", 0.3, 0.2, 0.1);

        for (auto const & entity : * m_EntityManager->getHUD()) {

            auto* hudPart = entity->getMesh();

            if (!hudPart || !entity->isVisible()) continue;
            m_Renderer->bindPipeline(m_CommandBuffersHud[m_ImageIndex], hudPart->getGraphicsPipeline());

            if (hudPart->hasPushConstants() && nullptr != hudPart->applyPushConstants) {
                hudPart->applyPushConstants(m_CommandBuffersHud[m_ImageIndex], hudPart->getPipelineLayout(), this,
                    hudPart);
            }

            for (uint32_t i = 0; i < hudPart->getUniformBuffers()->size(); i++) {
                if (m_RenderingStopped) return;
                m_Renderer->draw(m_CommandBuffersHud[m_ImageIndex], hudPart->getDescriptorSets().at(i), hudPart,
                    hudPart->getData(), hudPart->getData()->m_Ubos.size());
            }
        }

        m_Renderer->endMarker(m_CommandBuffersHud[m_ImageIndex]);
        endRendering(m_CommandBuffersHud[m_ImageIndex]);

        {
            std::lock_guard<std::mutex> guard(m_MutexCmdSubmit);
            m_CmdToSubmit[2] = m_CommandBuffersHud[m_ImageIndex];
            m_renderStatus = m_renderStatus << 1;

            if ((getDrawBbox() && m_renderStatus == 16) || (!getDrawBbox() && m_renderStatus == 8)) {
                m_RenderCond.notify_one();
            }
        }
    }

    void VulkanAdapter::drawBbox()
    {
        auto & entities = *m_EntityManager->getEntities();

        if (entities.size() > 0)
        {
            beginRendering(m_CommandBuffersBbox[m_ImageIndex]);
            m_Renderer->startMarker(m_CommandBuffersBbox[m_ImageIndex], "bbox_drawing", 0.3, 0.2, 0.1);

            for (auto & entity : entities) {
                Mesh* mesh = entity->getMesh();

                if (!mesh || !mesh->hasBbox()) continue;
                auto && bbox = mesh->getBBox()->mesh;

                if (!bbox) continue;

                m_Renderer->bindPipeline(m_CommandBuffersBbox[m_ImageIndex], bbox->getGraphicsPipeline());

                if (m_HasClicked && entity->isHit(m_RayPick)) {
                    PLP_DEBUG("HIT ! {}", mesh->getName());
                }
                m_HasClicked = false;

                int index = m_ImageIndex;
                for (uint32_t i = 0; i < bbox->getUniformBuffers()->size(); i++) {
                    index += i * 3;

                    if (bbox->hasPushConstants() && nullptr != bbox->applyPushConstants)
                        bbox->applyPushConstants(m_CommandBuffersBbox[m_ImageIndex], bbox->getPipelineLayout(), this,
                            bbox.get());

                    if (m_RenderingStopped) return;

                    m_Renderer->draw(m_CommandBuffersBbox[m_ImageIndex], bbox->getDescriptorSets().at(index), bbox.get(),
                        bbox->getData(), mesh->getData()->m_Ubos.size());

                    index = m_ImageIndex;
                }
            }

            m_Renderer->endMarker(m_CommandBuffersBbox[m_ImageIndex]);
            endRendering(m_CommandBuffersBbox[m_ImageIndex]);

            {
                std::lock_guard<std::mutex> guard(m_MutexCmdSubmit);
                m_CmdToSubmit[3] = m_CommandBuffersBbox[m_ImageIndex];

                m_renderStatus = m_renderStatus << 1;
                if ((getDrawBbox() && m_renderStatus == 16) || (!getDrawBbox() && m_renderStatus == 8)) {
                    m_RenderCond.notify_one();
                }
            }
        }
    }

    void VulkanAdapter::renderScene()
    {
        {
            std::unique_lock<std::mutex> render(m_MutexRenderScene);

            m_CmdToSubmit.resize(4);
            m_renderStatus = 1;

            std::string_view threadQueueName{ "render" };

            Poulpe::Locator::getThreadPool()->submit(threadQueueName, [this]() { drawSkybox(); });
            Poulpe::Locator::getThreadPool()->submit(threadQueueName, [this]() { drawEntities(); });
            Poulpe::Locator::getThreadPool()->submit(threadQueueName, [this]() { drawHUD(); });

            //@todo strip for release?
            if (getDrawBbox()) {
                Poulpe::Locator::getThreadPool()->submit(threadQueueName, [this] {
                    drawBbox();
                });
            }
        }
    }

    void VulkanAdapter::addCmdToSubmit(VkCommandBuffer cmd)
    {
        m_moreCmdToSubmit.emplace_back(cmd);
    }

    void VulkanAdapter::draw()
    {
        if (m_RenderingStopped) {
            {
              m_CmdToSubmit.clear();
              m_RenderingStopped = false;

              clearRendererScreen();
            }
            return;
        }

        {
            std::unique_lock<std::mutex> render(m_MutexRenderScene);

            m_RenderCond.wait(render, [this] {
                return  (getDrawBbox()) ? m_renderStatus == 16 : m_renderStatus == 8;
            });
            {
                std::unique_lock<std::mutex> render(m_MutexCmdSubmit);
                std::vector<VkCommandBuffer> cmds{};

                std::copy_if(m_CmdToSubmit.begin(), m_CmdToSubmit.end(), std::back_inserter(cmds),
                    [](VkCommandBuffer vkBuffer) { return vkBuffer != VK_NULL_HANDLE; });

                if (m_moreCmdToSubmit.size() > 0) {
                   std::copy(m_moreCmdToSubmit.begin(), m_moreCmdToSubmit.end(), std::back_inserter(cmds));
                }

                //Poulpe::Locator::getThreadPool()->Submit("submit", [=, this]() { Submit({ cmds.at(0), cmds.at(1)}); });
                //Poulpe::Locator::getThreadPool()->Submit("submit", [=, this]() { Submit({ cmds.at(2) }); });
                submit(cmds);
                present();

                m_CmdToSubmit.clear();
                m_moreCmdToSubmit.clear();
                cmds.clear();

                //@todo wtf ?
                /*uint32_t currentFrame = m_Renderer->GetNextFrameIndex();
                if (std::cmp_equal(currentFrame, VK_ERROR_OUT_OF_DATE_KHR) || std::cmp_equal(currentFrame, VK_SUBOPTIMAL_KHR)) {
                    RecreateSwapChain();
                }*/
            }
            acquireNextImage();
        }
    }

    void VulkanAdapter::clearRendererScreen()
    {
        {
            vkResetCommandBuffer(m_CommandBuffersEntities[m_ImageIndex], 0);
            vkResetCommandBuffer(m_CommandBuffersHud[m_ImageIndex], 0);
            vkResetCommandBuffer(m_CommandBuffersSkybox[m_ImageIndex], 0);

            beginRendering(m_CommandBuffersEntities[m_ImageIndex]);
            //do nothing !
            endRendering(m_CommandBuffersEntities[m_ImageIndex]);

            beginRendering(m_CommandBuffersHud[m_ImageIndex]);
            //do nothing !
            endRendering(m_CommandBuffersHud[m_ImageIndex]);

            beginRendering(m_CommandBuffersSkybox[m_ImageIndex], VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
            //do nothing !
            endRendering(m_CommandBuffersSkybox[m_ImageIndex]);

            std::vector<VkCommandBuffer> cmds{};
            cmds.emplace_back(m_CommandBuffersSkybox[m_ImageIndex]);
            cmds.emplace_back(m_CommandBuffersEntities[m_ImageIndex]);
            cmds.emplace_back(m_CommandBuffersHud[m_ImageIndex]);

            if (getDrawBbox()) {
                vkResetCommandBuffer(m_CommandBuffersBbox[m_ImageIndex], 0);

                beginRendering(m_CommandBuffersBbox[m_ImageIndex]);
                //do nothing !
                endRendering(m_CommandBuffersBbox[m_ImageIndex]);
                cmds.emplace_back(m_CommandBuffersBbox[m_ImageIndex]);
            }

            submit(cmds);
            present();

            //@todo wtf ?
            //uint32_t currentFrame = m_Renderer->GetNextFrameIndex();
            //if (std::cmp_equal(currentFrame, VK_ERROR_OUT_OF_DATE_KHR) || std::cmp_equal(currentFrame, VK_SUBOPTIMAL_KHR)) {
            //    RecreateSwapChain();
            //}

            acquireNextImage();
            onFinishRender();
            m_renderStatus = 1;
        }
    }

    void VulkanAdapter::destroy()
    {
        m_Renderer->destroySwapchain(m_Renderer->getDevice(), m_SwapChain, {}, m_SwapChainImageViews);

        for (auto sema : m_Semaphores) {
            m_Renderer->destroySemaphores(sema);
        }
        m_Renderer->destroyFences();

        for (auto item: m_DepthImages) {
            vkDestroyImage(m_Renderer->getDevice(), item, nullptr);
        }
        for (auto item : m_DepthImageViews) {
            vkDestroyImageView(m_Renderer->getDevice(), item, nullptr);
        }
        for (auto& buffer : m_UniformBuffers.first) {
            m_Renderer->destroyBuffer(buffer);
        }
        for (VkDescriptorSetLayout descriptorSetLayout : m_DescriptorSetLayouts) {
            vkDestroyDescriptorSetLayout(m_Renderer->getDevice(), descriptorSetLayout, nullptr);
        }
        for (VkDescriptorPool descriptorPool : m_DescriptorPools) {
            vkDestroyDescriptorPool(m_Renderer->getDevice(), descriptorPool, nullptr);
        }
        m_Renderer->destroyRenderPass(m_RenderPass.get(), m_CommandPoolEntities, m_CommandBuffersEntities);
        m_Renderer->destroyRenderPass(m_RenderPass.get(), m_CommandPoolBbox, m_CommandBuffersBbox);
        m_Renderer->destroyRenderPass(m_RenderPass.get(), m_CommandPoolSkybox, m_CommandBuffersSkybox);
        m_Renderer->destroyRenderPass(m_RenderPass.get(), m_CommandPoolHud, m_CommandBuffersHud);
    }

    void VulkanAdapter::immediateSubmit(std::function<void(VkCommandBuffer cmd)> && function,
        [[maybe_unused]] int queueIndex)
    {
        auto commandPool = m_Renderer->createCommandPool();
        VkCommandBuffer cmd = m_Renderer->allocateCommandBuffers(commandPool)[0];
        m_Renderer->beginCommandBuffer(cmd);
        function(cmd);
        m_Renderer->endCommandBuffer(cmd);
        m_Renderer->queueSubmit(cmd);
        vkDestroyCommandPool(m_Renderer->getDevice(), commandPool, nullptr);
    }

    void VulkanAdapter::beginRendering(VkCommandBuffer commandBuffer, VkAttachmentLoadOp loadOp,
        VkAttachmentStoreOp storeOp)
    {
        m_Renderer->beginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        VkImageMemoryBarrier swapChainImageRenderBeginBarrier = m_Renderer->setupImageMemoryBarrier(
            m_SwapChainImages[m_ImageIndex], 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        m_Renderer->addPipelineBarriers(commandBuffer, { swapChainImageRenderBeginBarrier },
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_DEPENDENCY_BY_REGION_BIT);

        VkImageMemoryBarrier depthImageRenderBeginBarrier = m_Renderer->setupImageMemoryBarrier(
            m_DepthImages[m_ImageIndex], 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, VK_IMAGE_ASPECT_DEPTH_BIT);

        m_Renderer->addPipelineBarriers(commandBuffer, { depthImageRenderBeginBarrier },
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_DEPENDENCY_BY_REGION_BIT);

        m_Renderer->beginRendering(commandBuffer, m_SwapChainImageViews[m_ImageIndex], m_DepthImageViews[m_ImageIndex],
            loadOp, storeOp);

        m_Renderer->setViewPort(commandBuffer);
        m_Renderer->setScissor(commandBuffer);
    }

    void VulkanAdapter::endRendering(VkCommandBuffer commandBuffer)
    {
        m_Renderer->endRendering(commandBuffer);

//#ifndef PLP_DEBUG_BUILD
//        auto newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//#else
        auto newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//#endif

        VkImageMemoryBarrier swapChainImageEndRenderBeginBarrier = m_Renderer->setupImageMemoryBarrier(
            m_SwapChainImages[m_ImageIndex], VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, newLayout );

        m_Renderer->addPipelineBarriers(commandBuffer, { swapChainImageEndRenderBeginBarrier },
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0);

        VkImageMemoryBarrier depthImageEndRenderBeginBarrier = m_Renderer->setupImageMemoryBarrier(
            m_DepthImages[m_ImageIndex], VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, newLayout, 1, VK_IMAGE_ASPECT_DEPTH_BIT);

        m_Renderer->addPipelineBarriers(commandBuffer, { depthImageEndRenderBeginBarrier },
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0);

        m_Renderer->endCommandBuffer(commandBuffer);
    }

    void VulkanAdapter::submit(std::vector<VkCommandBuffer> commandBuffers, int queueIndex)
    {
        VkResult submitResult = m_Renderer->queueSubmit(m_ImageIndex, commandBuffers, queueIndex);

        onFinishRender();

        if (submitResult != VK_SUCCESS) {
            PLP_WARN("Error on queue submit: {}", submitResult);
            recreateSwapChain();
            return;
        }
    }

    void VulkanAdapter::present([[maybe_unused]] int queueIndex)
    {
//#ifndef PLP_DEBUG_BUILD
//        std::vector<VkSemaphore>& renderFinishedSemaphores = m_Semaphores.at(queueIndex).second;
//        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[m_ImageIndex] };
//
//  
//        VkSemaphoreTypeCreateInfo sema;
//        sema.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
//        sema.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
//        sema.pNext = NULL;
//
//        VkSemaphoreWaitInfo waitInfo;
//        waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
//        waitInfo.pNext = NULL;
//        waitInfo.flags = 0;
//        waitInfo.semaphoreCount = 1;
//        waitInfo.pSemaphores = { &m_Semaphores.at(queueIndex).second[m_ImageIndex]};
//
//        vkWaitSemaphores(m_Renderer->GetDevice(), &waitInfo, UINT64_MAX);
//
//        VkResult presentResult = m_Renderer->QueuePresent(m_ImageIndex, m_SwapChain, m_Semaphores.at(queueIndex), queueIndex);
//
//        if (presentResult != VK_SUCCESS) {
//            PLP_WARN("Error on queue present: {}", presentResult);
//            RecreateSwapChain();
//        }
//#endif
    }

    void VulkanAdapter::acquireNextImage()
    {
//#ifndef PLP_DEBUG_BUILD
//      m_ImageIndex = m_Renderer->AcquireNextImageKHR(m_SwapChain, m_Semaphores.at(0));
//#else
      m_ImageIndex = m_Renderer->getNextFrameIndex();
//#endif
    }

    void VulkanAdapter::setRayPick(float x, float y, float z, [[maybe_unused]] int width, [[maybe_unused]] int height)
    {
        glm::vec3 rayNds = glm::vec3(x, y, z);
        glm::vec4 rayClip = glm::vec4(rayNds.x, rayNds.y, -1.0, 1.0);
        glm::vec4 rayEye = glm::inverse(getPerspective()) * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);

        glm::vec4 tmp = (glm::inverse(getCamera()->getView()) * rayEye);
        glm::vec3 rayWor = glm::vec3(tmp.x, tmp.y, tmp.z);
        m_RayPick = glm::normalize(rayWor);

        m_HasClicked = true;
    }

    void VulkanAdapter::clear()
    {
        m_EntityManager->clear();
    }

    void VulkanAdapter::onFinishRender()
    {
        Event event{ "OnFinishRender" };
        for (const auto& observer : m_Observers) {
            observer->notify(event);
        }
    }

    void VulkanAdapter::attachObserver(IObserver* observer)
    {
        m_Observers.push_back(observer);
    }

    VkRenderPass VulkanAdapter::createImGuiRenderPass(VkFormat format)
    {
        VkRenderPass renderPass{};

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

        VkResult result = vkCreateRenderPass(m_Renderer->getDevice(), & info, nullptr, & renderPass);

        if (result != VK_SUCCESS) {
            PLP_FATAL("failed to create imgui render pass : {}", result);
        }
        return renderPass;
    }

    ImGuiInfo VulkanAdapter::getImGuiInfo()
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

        VkDescriptorPool imguiPool = m_Renderer->createDescriptorPool(poolSizes, 1000,
            VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

        ImGui_ImplVulkan_InitInfo info = {};

        info.Instance = m_Renderer->getInstance();
        info.PhysicalDevice = m_Renderer->getPhysicalDevice();
        info.Device = m_Renderer->getDevice();
        info.QueueFamily = m_Renderer->getQueueFamily();
        info.Queue = m_Renderer->getGraphicsQueues()[1];
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

        auto commandPool = m_Renderer->createCommandPool();

        const SwapChainSupportDetails swapChainDetails = m_Renderer->querySwapChainSupport(m_Renderer->getPhysicalDevice());
        VkSurfaceFormatKHR imguiFormat{};
        
        for (const auto& availableFormat : swapChainDetails.formats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM
                && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                imguiFormat = availableFormat;
                break;
            }
        }

        auto rdrPass = createImGuiRenderPass(imguiFormat.format);

        ImGuiInfo imGuiInfo;
        imGuiInfo.info = info;
        imGuiInfo.rdrPass = std::move(rdrPass);
        imGuiInfo.cmdBuffer = m_Renderer->allocateCommandBuffers(commandPool)[0];
        //imGuiInfo.pipeline = m_Pipelines[0].graphicsPipeline;
        imGuiInfo.width = m_Renderer->getSwapChainExtent().width;
        imGuiInfo.height = m_Renderer->getSwapChainExtent().height;

        return imGuiInfo;
    }

    void VulkanAdapter::showGrid(bool show)
    {
        for (auto & hudPart : *m_EntityManager->getHUD()) {
            if ("grid" == hudPart->getName()) {
                hudPart->setVisible(show);
            }
        }
    }
}
