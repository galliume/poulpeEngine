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
    
    VulkanAdapter::VulkanAdapter(
      Window* window,
      EntityManager* entityManager,
      LightManager* lightManager)
        : m_Window(window),
          m_EntityManager(entityManager),
          m_LightManager(lightManager)
    {
        m_Renderer = std::make_unique<VulkanRenderer>(window);
    }

    void VulkanAdapter::init()
    {
        m_RayPick = glm::vec3(0.0f);
        setPerspective();

        m_SwapChain = m_Renderer->createSwapChain(m_SwapChainImages);

        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        m_SwapChainSamplers.resize(m_SwapChainImages.size());
        
        m_SwapChainDepthImages.resize(m_SwapChainImages.size());
        m_SwapChainDepthImageViews.resize(m_SwapChainImages.size());
        m_SwapChainDepthSamplers.resize(m_SwapChainImages.size());

        for (size_t i = 0; i < m_SwapChainImages.size(); ++i) {
          
          VkImage image;

          m_Renderer->createImage(m_Renderer->getSwapChainExtent().width, m_Renderer->getSwapChainExtent().height, 1,
              VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT 
              | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image);

          //m_SwapChainImages[i] = image;
          m_SwapChainImageViews[i] = m_Renderer->createImageView(m_SwapChainImages[i],
            m_Renderer->getSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
          m_SwapChainSamplers[i] = m_Renderer->createTextureSampler(1);

          VkImage depthImage;

          m_Renderer->createImage(m_Renderer->getSwapChainExtent().width, m_Renderer->getSwapChainExtent().height, 1,
            VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT
            | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage);

          VkImageView depthImageView = m_Renderer->createImageView(depthImage, VK_FORMAT_D32_SFLOAT, 1,
            VK_IMAGE_ASPECT_DEPTH_BIT);

          m_SwapChainDepthImages[i] = depthImage;
          m_SwapChainDepthImageViews[i] = depthImageView;
          m_SwapChainDepthSamplers[i] = m_Renderer->createTextureSampler(1);
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
        m_CommandBuffersHUD = m_Renderer->allocateCommandBuffers(m_CommandPoolHud,
            static_cast<uint32_t>(m_SwapChainImageViews.size()));

        m_CommandPoolShadowMap = m_Renderer->createCommandPool();
        m_CommandBuffersShadowMap = m_Renderer->allocateCommandBuffers(m_CommandPoolShadowMap,
          static_cast<uint32_t>(m_SwapChainImageViews.size()));

        VkSemaphoreTypeCreateInfo semaType;
        semaType.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        semaType.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
        semaType.initialValue = 0;
        semaType.pNext = NULL;

        VkSemaphoreCreateInfo sema{};
        sema.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        sema.pNext = &semaType;

        VkResult result{};

        m_EntitiesSemaRenderFinished.resize(m_MAX_FRAMES_IN_FLIGHT);
        m_SkyboxSemaRenderFinished.resize(m_MAX_FRAMES_IN_FLIGHT);
        m_HUDSemaRenderFinished.resize(m_MAX_FRAMES_IN_FLIGHT);
        m_ShadowMapSemaRenderFinished.resize(m_MAX_FRAMES_IN_FLIGHT);

        m_ImageAvailable.resize(m_MAX_FRAMES_IN_FLIGHT);

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        m_InFlightFences.resize(m_MAX_FRAMES_IN_FLIGHT);
        m_ImagesInFlight.resize(m_MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

        m_CmdSkyboxStatus.resize(m_MAX_FRAMES_IN_FLIGHT);
        m_CmdHUDStatus.resize(m_MAX_FRAMES_IN_FLIGHT);
        m_CmdEntitiesStatus.resize(m_MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < m_MAX_FRAMES_IN_FLIGHT; ++i) {

             result = vkCreateSemaphore(rdr()->getDevice(), &sema, nullptr, &m_EntitiesSemaRenderFinished[i]);
            if (VK_SUCCESS != result) PLP_ERROR("can't create m_EntitiesSemaRenderFinished semaphore");

            result = vkCreateSemaphore(rdr()->getDevice(), &sema, nullptr, &m_SkyboxSemaRenderFinished[i]);
            if (VK_SUCCESS != result) PLP_ERROR("can't create m_SkyboxSemaRenderFinished semaphore");

            result = vkCreateSemaphore(rdr()->getDevice(), &sema, nullptr, &m_HUDSemaRenderFinished[i]);
            if (VK_SUCCESS != result) PLP_ERROR("can't create m_HUDSemaRenderFinished semaphore");

            result = vkCreateSemaphore(rdr()->getDevice(), &sema, nullptr, &m_ShadowMapSemaRenderFinished[i]);
            if (VK_SUCCESS != result) PLP_ERROR("can't create m_ShadowMapSemaRenderFinished semaphore");

            result = vkCreateSemaphore(rdr()->getDevice(), &sema, nullptr, &m_ImageAvailable[i]);
            if (VK_SUCCESS != result) PLP_ERROR("can't create m_ImageAvailable semaphore");

            result = vkCreateFence(rdr()->getDevice(), &fenceInfo, nullptr, &m_ImagesInFlight[i]);
            if (VK_SUCCESS != result) PLP_ERROR("can't create m_PreviousFrame fence");

            result = vkCreateFence(rdr()->getDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]);
            if (VK_SUCCESS != result) PLP_ERROR("can't create m_InFlightFences fence");

            auto cmdSky = std::make_unique<DrawCommand>();
            cmdSky->buffer = &m_CommandBuffersSkybox[i],
            cmdSky->stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            cmdSky->semaphore = &m_SkyboxSemaRenderFinished[i],
            cmdSky->done.store(false);

            m_CmdSkyboxStatus[i] = std::move(cmdSky);

            auto cmdHUD = std::make_unique<DrawCommand>();
            cmdHUD->buffer = &m_CommandBuffersHUD[i],
            cmdHUD->stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            cmdHUD->semaphore = &m_HUDSemaRenderFinished[i],
            cmdHUD->done.store(false);

            m_CmdHUDStatus[i] = std::move(cmdHUD);

            auto cmdEntities = std::make_unique<DrawCommand>();
            cmdEntities->buffer = &m_CommandBuffersEntities[i],
            cmdEntities->stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            cmdEntities->semaphore = &m_EntitiesSemaRenderFinished[i],
            cmdEntities->done.store(false);

            m_CmdEntitiesStatus[i] = std::move(cmdEntities);
        }

        for (size_t i = 0; i < m_MAX_FRAMES_IN_FLIGHT; ++i) {
          VkImage image{};
          m_Renderer->createDepthMapImage(image);
          m_DepthMapImages.emplace_back(image);
          m_DepthMapImageViews.emplace_back(m_Renderer->createDepthMapImageView(image));
          m_DepthMapSamplers.emplace_back(m_Renderer->createDepthMapSampler());
        }
    }

    void VulkanAdapter::recreateSwapChain()
    {
 
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

    void VulkanAdapter::drawShadowMap(std::vector<std::unique_ptr<Entity>>* entities,  Light light)
    {
      std::string const pipelineName{ "shadowMap" };
      auto pipeline = getPipeline(pipelineName);

        //m_Renderer->beginCommandBuffer(m_CommandBuffersShadowMap[m_CurrentFrame]);
        m_Renderer->startMarker(m_CommandBuffersEntities[m_CurrentFrame], "shadow_map_" + pipelineName, 0.1, 0.2, 0.3);

        rdr()->transitionImageLayout(m_CommandBuffersEntities[m_CurrentFrame], m_DepthMapImages[m_CurrentFrame],
          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

        VkClearColorValue colorClear = {};
        colorClear.float32[0] = 1.0f;
        colorClear.float32[1] = 1.0f;
        colorClear.float32[2] = 1.0f;
        colorClear.float32[3] = 1.0f;
      
        VkClearDepthStencilValue depthStencil = { 1.f, 0 };
      
        VkRenderingAttachmentInfo depthAttachment{ };
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = m_DepthMapImageViews[m_CurrentFrame];
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.clearValue.depthStencil = depthStencil;
        depthAttachment.clearValue.color = colorClear;

        VkRenderingInfo  renderingInfo{ };
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea.extent.width = rdr()->getSwapChainExtent().width;
        renderingInfo.renderArea.extent.height = rdr()->getSwapChainExtent().height;
        renderingInfo.layerCount = 1;
        renderingInfo.pDepthAttachment = & depthAttachment;
        renderingInfo.colorAttachmentCount = 0;
        renderingInfo.flags = VK_SUBPASS_CONTENTS_INLINE;

        vkCmdBeginRenderingKHR(m_CommandBuffersEntities[m_CurrentFrame], & renderingInfo);

        m_Renderer->setViewPort(m_CommandBuffersEntities[m_CurrentFrame]);
        m_Renderer->setScissor(m_CommandBuffersEntities[m_CurrentFrame]);
        
        float depthBiasConstant = -1.25f;
        float depthBiasSlope = 5.f;

        vkCmdSetDepthBias(m_CommandBuffersEntities[m_CurrentFrame], depthBiasConstant, 0.0f, depthBiasSlope);

        for (auto& entity : *entities) {
            Mesh* mesh = entity->getMesh();

            if (!mesh) continue;

            m_Renderer->bindPipeline(m_CommandBuffersEntities[m_CurrentFrame], pipeline->pipeline);
                
            for (size_t i = 0; i < mesh->getData()->m_Ubos.size(); ++i) {
                mesh->getData()->m_Ubos[i].projection = light.lightSpaceMatrix;
            }

            int min{ 0 };
            int max{ 0 };

            for (size_t i = 0; i < mesh->getUniformBuffers()->size(); ++i) {
                max = mesh->getData()->m_UbosOffset.at(i);
                auto ubos = std::vector<UniformBufferObject>(mesh->getData()->m_Ubos.begin() + min, mesh->getData()->m_Ubos.begin() + max);
                rdr()->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &ubos);

                min = max;
            }

            constants pushConstants{};
            pushConstants.textureIDBB = glm::vec3(mesh->getData()->m_TextureIndex, 0.0, 0.0);
        
            pushConstants.view = light.view;
            pushConstants.viewPos = getCamera()->getPos();
            pushConstants.mapsUsed = mesh->getData()->mapsUsed;

            vkCmdPushConstants(m_CommandBuffersEntities[m_CurrentFrame], pipeline->pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants), & pushConstants);

            try {
                if (m_RenderingStopped) return;
                m_Renderer->draw(m_CommandBuffersEntities[m_CurrentFrame], *mesh->getDescSet(),
                *pipeline, mesh->getData(), mesh->getData()->m_Ubos.size(), mesh->isIndexed());
            }
            catch (std::exception& e) {
                PLP_DEBUG("Draw error: {}", e.what());
            }
        }

        m_Renderer->endMarker(m_CommandBuffersEntities[m_CurrentFrame]);
        m_Renderer->endRendering(m_CommandBuffersEntities[m_CurrentFrame]);

        rdr()->transitionImageLayout(m_CommandBuffersEntities[m_CurrentFrame], m_DepthMapImages[m_CurrentFrame],
          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

 //       m_Renderer->endCommandBuffer(m_CommandBuffersShadowMap[m_CurrentFrame]);

        //CommandToSubmit cmd{
        //    m_CommandBuffersShadowMap[m_CurrentFrame],
        //    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        //    m_ShadowMapSemaRenderFinished[m_CurrentFrame],
        //    & m_CmdShadowMapStatus
        //};

        //{
        //  std::lock_guard<std::mutex> guard(m_MutexQueueSubmit);
        //  m_CmdsToSubmit.emplace_back(cmd);
        //}
    }

    void VulkanAdapter::drawEntities()
    {
        if (0 < m_EntityManager->getEntities()->size()) {

          m_Renderer->beginCommandBuffer(m_CommandBuffersEntities[m_CurrentFrame], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

          drawShadowMap(m_EntityManager->getEntities(), m_LightManager->getAmbientLight());

          rdr()->transitionImageLayout(m_CommandBuffersEntities[m_CurrentFrame], m_SwapChainImages[m_CurrentFrame],
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

          rdr()->transitionImageLayout(m_CommandBuffersEntities[m_CurrentFrame], m_SwapChainDepthImages[m_CurrentFrame],
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

          m_Renderer->beginRendering(
            m_CommandBuffersEntities[m_CurrentFrame],
            m_SwapChainImageViews[m_CurrentFrame],
            m_SwapChainDepthImageViews[m_CurrentFrame],
            VK_ATTACHMENT_LOAD_OP_LOAD,
            VK_ATTACHMENT_STORE_OP_STORE);

          m_Renderer->setViewPort(m_CommandBuffersEntities[m_CurrentFrame]);
          m_Renderer->setScissor(m_CommandBuffersEntities[m_CurrentFrame]);

            m_Renderer->startMarker(m_CommandBuffersEntities[m_CurrentFrame], "entities_drawing", 0.2, 0.2, 0.9);

            for (auto & entity : *m_EntityManager->getEntities()) {
                Mesh* mesh = entity->getMesh();

                if (!mesh) continue;
                auto pipeline = getPipeline(mesh->getShaderName());


                // if (m_HasClicked && mesh->IsHit(m_RayPick)) {
                //    PLP_DEBUG("HIT ! {}", mesh->GetName());
                // }
                //m_HasClicked = false;

                m_Renderer->bindPipeline(m_CommandBuffersEntities[m_CurrentFrame], pipeline->pipeline);

                for (size_t i = 0; i < mesh->getData()->m_Ubos.size(); ++i) {
                    mesh->getData()->m_Ubos[i].projection = getPerspective();
                }

                int min{ 0 };
                int max{ 0 };

                for (size_t i = 0; i < mesh->getUniformBuffers()->size(); ++i) {
                  max = mesh->getData()->m_UbosOffset.at(i);
                  auto ubos = std::vector<UniformBufferObject>(mesh->getData()->m_Ubos.begin() + min, mesh->getData()->m_Ubos.begin() + max);
                  rdr()->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &ubos);

                  min = max;

                }
                if (mesh->hasPushConstants() && nullptr != mesh->applyPushConstants)
                    mesh->applyPushConstants(m_CommandBuffersEntities[m_CurrentFrame], pipeline->pipelineLayout, this, mesh);

                try {
                    if (m_RenderingStopped) return;
                    m_Renderer->draw(m_CommandBuffersEntities[m_CurrentFrame], *mesh->getDescSet(),
                        *pipeline, mesh->getData(), mesh->getData()->m_Ubos.size(), mesh->isIndexed());
                }
                catch (std::exception & e) {
                    PLP_DEBUG("Draw error: {}", e.what());
                }
            }

            m_Renderer->endMarker(m_CommandBuffersEntities[m_CurrentFrame]);
            endRendering(m_CommandBuffersEntities[m_CurrentFrame]);
            m_CmdsToSubmit.emplace_back(m_CmdEntitiesStatus[m_CurrentFrame].get());
            m_CmdEntitiesStatus[m_CurrentFrame]->done.store(true);
        }
    }

     void VulkanAdapter::drawSkybox()
     {
        if (auto skybox = m_EntityManager->getSkybox()) {

            beginRendering(m_CommandBuffersSkybox[m_CurrentFrame], VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, false);
            m_Renderer->startMarker(m_CommandBuffersSkybox[m_CurrentFrame], "skybox_drawing", 0.3, 0.2, 0.1);

            Mesh::Data* skyboxData = skybox->getMesh()->getData();
            auto pipeline = getPipeline(skybox->getMesh()->getShaderName());

            m_Renderer->bindPipeline(m_CommandBuffersSkybox[m_CurrentFrame], pipeline->pipeline);

            if (skybox->getMesh()->hasPushConstants() && nullptr != skybox->getMesh()->applyPushConstants)
                skybox->getMesh()->applyPushConstants(m_CommandBuffersSkybox[m_CurrentFrame], pipeline->pipelineLayout, this,
                    skybox->getMesh());

            if (m_RenderingStopped) return;
            m_Renderer->draw(m_CommandBuffersSkybox[m_CurrentFrame], *skybox->getMesh()->getDescSet(), *pipeline,
                skyboxData, skyboxData->m_Ubos.size(), false);

            m_Renderer->endMarker(m_CommandBuffersSkybox[m_CurrentFrame]);
            endRendering(m_CommandBuffersSkybox[m_CurrentFrame]);

            m_CmdsToSubmit.emplace_back(m_CmdSkyboxStatus[m_CurrentFrame].get());
            m_CmdSkyboxStatus[m_CurrentFrame]->done.store(true);
        }
    }

    void VulkanAdapter::drawHUD()
    {
        beginRendering(m_CommandBuffersHUD[m_CurrentFrame]);
        m_Renderer->startMarker(m_CommandBuffersHUD[m_CurrentFrame], "hud_drawing", 0.3, 0.2, 0.1);

        for (auto const & entity : * m_EntityManager->getHUD()) {

           auto* hudPart = entity->getMesh();

            if (!hudPart || !entity->isVisible()) continue;
            
            auto pipeline = getPipeline(hudPart->getShaderName());

            m_Renderer->bindPipeline(m_CommandBuffersHUD[m_CurrentFrame], pipeline->pipeline);

            if (hudPart->hasPushConstants() && nullptr != hudPart->applyPushConstants) {
                hudPart->applyPushConstants(m_CommandBuffersHUD[m_CurrentFrame], pipeline->pipelineLayout, this,
                    hudPart);
            }

            if (m_RenderingStopped) return;
            m_Renderer->draw(m_CommandBuffersHUD[m_CurrentFrame], *hudPart->getDescSet(), *pipeline,
                hudPart->getData(), hudPart->getData()->m_Ubos.size());
        }

        m_Renderer->endMarker(m_CommandBuffersHUD[m_CurrentFrame]);
        endRendering(m_CommandBuffersHUD[m_CurrentFrame]);

        m_CmdsToSubmit.emplace_back(m_CmdHUDStatus[m_CurrentFrame].get());
        m_CmdHUDStatus[m_CurrentFrame]->done.store(true);
    }

    void VulkanAdapter::drawBbox()
    {
 /*       auto & entities = *m_EntityManager->getEntities();

        if (entities.size() > 0)
        {
            beginRendering(m_CommandBuffersBbox[m_CurrentFrame]);
            m_Renderer->startMarker(m_CommandBuffersBbox[m_CurrentFrame], "bbox_drawing", 0.3, 0.2, 0.1);

            for (auto & entity : entities) {
                Mesh* mesh = entity->getMesh();

                if (!mesh || !mesh->hasBbox()) continue;
                auto && bbox = mesh->getBBox()->mesh;

                if (!bbox) continue;

                m_Renderer->bindPipeline(m_CommandBuffersBbox[m_CurrentFrame], bbox->getGraphicsPipeline());

                if (m_HasClicked && entity->isHit(m_RayPick)) {
                    PLP_DEBUG("HIT ! {}", mesh->getName());
                }
                m_HasClicked = false;

                int index = m_CurrentFrame;
                for (uint32_t i = 0; i < bbox->getUniformBuffers()->size(); i++) {
                    index += i * 3;

                    if (bbox->hasPushConstants() && nullptr != bbox->applyPushConstants)
                        bbox->applyPushConstants(m_CommandBuffersBbox[m_CurrentFrame], bbox->getPipelineLayout(), this,
                            bbox.get());

                    if (m_RenderingStopped) return;

                    m_Renderer->draw(m_CommandBuffersBbox[m_CurrentFrame], bbox->getDescriptorSets().at(index), bbox.get(),
                        bbox->getData(), mesh->getData()->m_Ubos.size());

                    index = m_CurrentFrame;
                }
            }

            m_Renderer->endMarker(m_CommandBuffersBbox[m_CurrentFrame]);
            endRendering(m_CommandBuffersBbox[m_CurrentFrame]);
        }*/
    }

    void VulkanAdapter::renderScene()
    {
        //uint32_t imageIndex{};

        vkWaitForFences(rdr()->getDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
        VkResult result = vkAcquireNextImageKHR(rdr()->getDevice(), m_SwapChain, UINT64_MAX, m_ImageAvailable[m_CurrentFrame], VK_NULL_HANDLE, &m_ImageIndex);
        vkResetFences(rdr()->getDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

        //@todo clean
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            PLP_ERROR("Error on vkAcquireNextImageKHR {}", result);
        }
        //vkResetCommandBuffer(m_CommandBuffersEntities[m_CurrentFrame], 0);
        //vkResetCommandBuffer(m_CommandBuffersHUD[m_CurrentFrame], 0);
        //vkResetCommandBuffer(m_CommandBuffersShadowMap[m_CurrentFrame], 0);
        //vkResetCommandBuffer(m_CommandBuffersSkybox[m_CurrentFrame], 0);

        //std::string_view threadQueueName{ "render" };

         //PLP_DEBUG("m_Nodraw: {}", m_Nodraw.load());
         //PLP_DEBUG("m_CmdSkyboxStatus: {}", m_CmdSkyboxStatus[m_CurrentFrame]->done.load());
         //PLP_DEBUG("m_CmdEntitiesStatus: {}", m_CmdEntitiesStatus[m_CurrentFrame]->done.load());
         //PLP_DEBUG("m_CmdHUDStatus: {}", m_CmdHUDStatus[m_CurrentFrame]->done.load());

          if (!m_CmdSkyboxStatus[m_CurrentFrame]->done.load() || m_Nodraw.load()) {
            m_CmdSkyboxStatus[m_CurrentFrame]->done.store(false);
            drawSkybox();
            //Poulpe::Locator::getThreadPool()->submit(threadQueueName, [this]() { drawSkybox(); });
          }
          if (!m_CmdEntitiesStatus[m_CurrentFrame]->done.load() || m_Nodraw.load()) {
            m_CmdEntitiesStatus[m_CurrentFrame]->done.store(false);
            drawEntities();
            //Poulpe::Locator::getThreadPool()->submit(threadQueueName, [this]() { drawEntities(); });
          }
          if (!m_CmdHUDStatus[m_CurrentFrame]->done.load() || m_Nodraw.load()) {
            m_CmdHUDStatus[m_CurrentFrame]->done.store(false);
            drawHUD();
            //Poulpe::Locator::getThreadPool()->submit(threadQueueName, [this]() { drawHUD(); });
          }

          submit();
        //@todo strip for release?
        //if (getDrawBbox()) {
        //    Poulpe::Locator::getThreadPool()->submit(threadQueueName, [this] {
        //        drawBbox();
        //    });
        //}
    }

    void VulkanAdapter::draw()
    {
        //if (m_RenderingStopped) {
        //    {
        //      m_RenderingStopped = false;
        //      clearRendererScreen();
        //    }
        //    return;
        //}
        //std::string_view threadQueueName{ "render" };

        //Poulpe::Locator::getThreadPool()->Submit("submit", [=, this]() { Submit({ cmds.at(0), cmds.at(1)}); });
        //Poulpe::Locator::getThreadPool()->Submit("submit", [=, this]() { Submit({ cmds.at(2) }); });
        //submit();
    }

    void VulkanAdapter::clearRendererScreen()
    {
        {
            vkResetCommandBuffer(m_CommandBuffersEntities[m_CurrentFrame], 0);
            vkResetCommandBuffer(m_CommandBuffersHUD[m_CurrentFrame], 0);
            vkResetCommandBuffer(m_CommandBuffersSkybox[m_CurrentFrame], 0);

            beginRendering(m_CommandBuffersEntities[m_CurrentFrame]);
            //do nothing !
            endRendering(m_CommandBuffersEntities[m_CurrentFrame]);

            beginRendering(m_CommandBuffersHUD[m_CurrentFrame]);
            //do nothing !
            endRendering(m_CommandBuffersHUD[m_CurrentFrame]);

            beginRendering(m_CommandBuffersSkybox[m_CurrentFrame], VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
            //do nothing !
            endRendering(m_CommandBuffersSkybox[m_CurrentFrame]);

            std::vector<VkCommandBuffer> cmds{};
            cmds.emplace_back(m_CommandBuffersSkybox[m_CurrentFrame]);
            cmds.emplace_back(m_CommandBuffersEntities[m_CurrentFrame]);
            cmds.emplace_back(m_CommandBuffersHUD[m_CurrentFrame]);

            if (getDrawBbox()) {
                vkResetCommandBuffer(m_CommandBuffersBbox[m_CurrentFrame], 0);

                beginRendering(m_CommandBuffersBbox[m_CurrentFrame]);
                //do nothing !
                endRendering(m_CommandBuffersBbox[m_CurrentFrame]);
                cmds.emplace_back(m_CommandBuffersBbox[m_CurrentFrame]);
            }

            submit();
        }
    }

    void VulkanAdapter::destroy()
    {
       /* m_Renderer->destroySwapchain(m_Renderer->getDevice(), m_SwapChain, {}, m_SwapChainImageViews);

        m_Renderer->destroyFences();

        for (auto item: m_SwapChainDepthImages) {
            vkDestroyImage(m_Renderer->getDevice(), item, nullptr);
        }
        for (auto item : m_SwapChainDepthImageViews) {
            vkDestroyImageView(m_Renderer->getDevice(), item, nullptr);
        }
        for (auto& buffer : m_UniformBuffers.first) {
            m_Renderer->destroyBuffer(buffer);
        }*/
        for (VkDescriptorSetLayout descriptorSetLayout : m_DescriptorSetLayouts) {
            vkDestroyDescriptorSetLayout(m_Renderer->getDevice(), descriptorSetLayout, nullptr);
        }
        for (VkDescriptorPool descriptorPool : m_DescriptorPools) {
            vkDestroyDescriptorPool(m_Renderer->getDevice(), descriptorPool, nullptr);
        }
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

    //@todo do to much to refacto
    void VulkanAdapter::beginRendering(VkCommandBuffer commandBuffer, VkAttachmentLoadOp loadOp,
        VkAttachmentStoreOp storeOp, bool continuousCmdBuffer)
    {
        if (!continuousCmdBuffer) m_Renderer->beginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        rdr()->transitionImageLayout(commandBuffer, m_SwapChainImages[m_CurrentFrame],
          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

        rdr()->transitionImageLayout(commandBuffer, m_SwapChainDepthImages[m_CurrentFrame],
          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

        m_Renderer->beginRendering(
          commandBuffer,
          m_SwapChainImageViews[m_CurrentFrame],
          m_SwapChainDepthImageViews[m_CurrentFrame],
          loadOp,
          storeOp);

        m_Renderer->setViewPort(commandBuffer);
        m_Renderer->setScissor(commandBuffer);
    }

    void VulkanAdapter::endRendering(VkCommandBuffer commandBuffer)
    {
        m_Renderer->endRendering(commandBuffer);

        rdr()->transitionImageLayout(commandBuffer, m_SwapChainImages[m_CurrentFrame],
          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT);

       rdr()->transitionImageLayout(commandBuffer, m_SwapChainDepthImages[m_CurrentFrame],
          VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_DEPTH_BIT);

        m_Renderer->endCommandBuffer(commandBuffer);
    }

    void VulkanAdapter::submit()
    {
        if (m_CmdsToSubmit.empty()) {
          m_Nodraw.store(true);
          return;
        }
       
        m_Nodraw.store(false);
      
        {
        std::lock_guard<std::mutex> guard(m_MutexQueueSubmit);

        std::vector<VkCommandBuffer> cmdBuffers{};
        std::vector<VkPipelineStageFlags> waitStages{ };
        std::vector<VkSemaphore> waitSemaphores{ m_ImageAvailable[m_CurrentFrame] };
        std::vector<VkSemaphore> signalSemaphores{};

        for (auto cmdToSubmit : m_CmdsToSubmit) {
          cmdBuffers.emplace_back(*cmdToSubmit->buffer);
          signalSemaphores.emplace_back(*cmdToSubmit->semaphore);
          waitStages.emplace_back(cmdToSubmit->stageFlags);
        }

        //PLP_WARN("submit: {} commands to index: {}", m_CmdsToSubmit.size(), m_CurrentFrame);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pCommandBuffers = cmdBuffers.data();
        submitInfo.commandBufferCount = cmdBuffers.size();
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pWaitDstStageMask = waitStages.data();;
        submitInfo.signalSemaphoreCount = signalSemaphores.size();
        submitInfo.pSignalSemaphores = signalSemaphores.data();
        submitInfo.waitSemaphoreCount = waitSemaphores.size();
        submitInfo.pWaitSemaphores = waitSemaphores.data();

        auto queue = rdr()->getGraphicsQueues().at(0);

        VkResult submitResult = vkQueueSubmit(queue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);

        for (auto & cmdToSubmit : m_CmdsToSubmit) {
            cmdToSubmit->done.store(false);
        }

        m_CmdsToSubmit.clear();

        if (submitResult != VK_SUCCESS) {
          PLP_ERROR("Error on queue submit: {}", submitResult);
          throw std::runtime_error("Error on queueSubmit");
        }

        std::vector<VkSwapchainKHR> swapChains{ m_SwapChain };

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = signalSemaphores.size();
        presentInfo.pWaitSemaphores = signalSemaphores.data();
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains.data();
        presentInfo.pImageIndices = &m_ImageIndex;
        presentInfo.pResults = nullptr;

        VkResult presentResult = vkQueuePresentKHR(queue, &presentInfo);
        vkDeviceWaitIdle(rdr()->getDevice());
        if (presentResult != VK_SUCCESS) {
          PLP_ERROR("Error on queue present: {}", presentResult);
        }

        m_CurrentFrame = (m_CurrentFrame + 1) % static_cast<uint32_t>(m_MAX_FRAMES_IN_FLIGHT);
        //PLP_WARN("current frame: {}", m_ImageIndex);
        onFinishRender();
      }
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

    void VulkanAdapter::addPipeline(std::string const & shaderName, VulkanPipeline pipeline)
    {
        m_Pipelines[shaderName] = std::move(pipeline);
    }
}
