#include "Renderer.hpp"

#include "VulkanAPI.hpp"

#include "Poulpe/GUI/Window.hpp"

#include "Poulpe/Renderer/Vulkan/VulkanAPI.hpp"

#include <cfenv>
#include <exception>
#include <future>
#include <memory>
#include <volk.h>

namespace Poulpe
{
  //@todo should not be globally accessible
  std::atomic<float> Renderer::s_AmbiantLight{ 1.0f };
  std::atomic<float> Renderer::s_FogDensity{ 0.0f };
  std::atomic<float> Renderer::s_FogColor[3]{ 25 / 255.0f, 25 / 255.0f, 25 / 255.0f };
  std::atomic<int> Renderer::s_Crosshair{ 0 };
  std::atomic<int> Renderer::s_PolygoneMode{ VK_POLYGON_MODE_FILL };
    
  Renderer::Renderer(
    Window* const window,
    IEntityManager* const entityManager,
    ComponentManager* const componentManager,
    ILightManager* const lightManager,
    ITextureManager* const textureManager)
      : m_Window(window),
        m_EntityManager(entityManager),
        m_ComponentManager(componentManager),
        m_LightManager(lightManager),
        m_TextureManager(textureManager)
  {
      m_API = std::make_unique<VulkanAPI>(window);
  }

  void Renderer::init()
  {
      m_RayPick = glm::vec3(0.0f);
      setPerspective();

      m_SwapChain = m_API->createSwapChain(m_Images);
      
      m_ImageViews.resize(m_Images.size());
      m_Samplers.resize(m_Images.size());       
      m_DepthImages.resize(m_Images.size());
      m_DepthImageViews.resize(m_Images.size());
      m_DepthSamplers.resize(m_Images.size());

      m_ImageViewsBis.resize(m_Images.size());
      m_ImagesBis.resize(m_Images.size());
      m_SamplersBis.resize(m_Images.size());       
      m_DepthImagesBis.resize(m_Images.size());
      m_DepthImageViewsBis.resize(m_Images.size());
      m_DepthSamplersBis.resize(m_Images.size());

      m_ImageViewsTer.resize(m_Images.size());
      m_ImagesTer.resize(m_Images.size());
      m_SamplersTer.resize(m_Images.size());       
      m_DepthImagesTer.resize(m_Images.size());
      m_DepthImageViewsTer.resize(m_Images.size());
      m_DepthSamplersTer.resize(m_Images.size());

      for (size_t i{ 0 }; i < m_Images.size(); ++i) {
          
        VkImage image;

        m_API->createImage(m_API->getSwapChainExtent().width, m_API->getSwapChainExtent().height, 1,
            VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT 
            | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image);

        m_ImageViews[i] = m_API->createImageView(m_Images[i],
          m_API->getSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
        m_Samplers[i] = m_API->createTextureSampler(1);

        VkImage depthImage;

        m_API->createImage(m_API->getSwapChainExtent().width, m_API->getSwapChainExtent().height, 1,
          VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT
          | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage);

        VkImageView depthImageView = m_API->createImageView(depthImage, VK_FORMAT_D32_SFLOAT, 1,
          VK_IMAGE_ASPECT_DEPTH_BIT);

        m_DepthImages[i] = depthImage;
        m_DepthImageViews[i] = depthImageView;
        m_DepthSamplers[i] = m_API->createTextureSampler(1);
      }
      for (size_t i{ 0 }; i < m_Images.size(); ++i) {
        VkImageView depthImageView = m_API->createImageView(m_DepthImages[i], VK_FORMAT_D32_SFLOAT, 1,
          VK_IMAGE_ASPECT_DEPTH_BIT);

        m_DepthImageViewsBis[i] = depthImageView;
        m_DepthSamplersBis[i] = m_API->createTextureSampler(1);
      }      
      for (size_t i{ 0 }; i < m_Images.size(); ++i) {
        VkImageView depthImageView = m_API->createImageView(m_DepthImages[i], VK_FORMAT_D32_SFLOAT, 1,
          VK_IMAGE_ASPECT_DEPTH_BIT);
        m_DepthImageViewsTer[i] = depthImageView;
        m_DepthSamplersTer[i] = m_API->createTextureSampler(1);
      }
      m_CommandPoolEntities = m_API->createCommandPool();
      m_CommandPoolEntitiesBis = m_API->createCommandPool();
      m_CommandPoolEntitiesTer = m_API->createCommandPool();

      m_CommandBuffersEntities = m_API->allocateCommandBuffers(m_CommandPoolEntities,
        static_cast<uint32_t>(m_ImageViews.size()));

      m_CommandBuffersEntitiesBis = m_API->allocateCommandBuffers(m_CommandPoolEntitiesBis,
        static_cast<uint32_t>(m_ImageViews.size()));

      m_CommandBuffersEntitiesTer = m_API->allocateCommandBuffers(m_CommandPoolEntitiesTer,
        static_cast<uint32_t>(m_ImageViews.size()));

      m_CommandPoolShadowMap = m_API->createCommandPool();
      m_CommandBuffersShadowMap = m_API->allocateCommandBuffers(m_CommandPoolShadowMap,
        static_cast<uint32_t>(m_ImageViews.size()));

      for (size_t i { 0 }; i < m_Images.size(); ++i) {
        VkImage image{};
        m_API->createDepthMapImage(image);
        m_DepthMapImages.emplace_back(image);
        m_DepthMapImageViews.emplace_back(m_API->createDepthMapImageView(image));
        m_DepthMapSamplers.emplace_back(m_API->createDepthMapSampler());

        // VkImage imageBis{};
        // m_API->createDepthMapImage(imageBis);
        // m_DepthMapImagesBis.emplace_back(imageBis);
        // m_DepthMapImageViewsBis.emplace_back(m_API->createDepthMapImageView(imageBis));
        // m_DepthMapSamplersBis.emplace_back(m_API->createDepthMapSampler());

        // VkImage imageTer{};
        // m_API->createDepthMapImage(imageTer);
        // m_DepthMapImagesTer.emplace_back(imageTer);
        // m_DepthMapImageViewsTer.emplace_back(m_API->createDepthMapImageView(imageTer));
        // m_DepthMapSamplersTer.emplace_back(m_API->createDepthMapSampler());
      }
      
      VkResult result{};

      VkSemaphoreTypeCreateInfo semaType;
      semaType.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
      semaType.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
      semaType.initialValue = 0;
      semaType.pNext = nullptr;

      VkSemaphoreCreateInfo sema{};
      sema.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
      sema.pNext = &semaType;
      
      m_InFlightFences.resize(m_MAX_FRAMES_IN_FLIGHT);
      m_ImagesInFlight.resize(m_MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

      VkFenceCreateInfo fenceInfo{};
      fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
      m_EntitiesSemaRenderFinished.resize(m_MAX_RENDER_THREAD);
      m_ShadowMapSemaRenderFinished.resize(m_MAX_RENDER_THREAD);
      m_ImageAvailable.resize(m_MAX_RENDER_THREAD);

       for (size_t i { 0 }; i < m_MAX_RENDER_THREAD; ++i) {

          result = vkCreateSemaphore(m_API->getDevice(), &sema, nullptr, &m_EntitiesSemaRenderFinished[i]);
          if (VK_SUCCESS != result) PLP_ERROR("can't create m_EntitiesSemaRenderFinished semaphore");

          result = vkCreateSemaphore(m_API->getDevice(), &sema, nullptr, &m_ShadowMapSemaRenderFinished[i]);
          if (VK_SUCCESS != result) PLP_ERROR("can't create m_ShadowMapSemaRenderFinished semaphore");

          result = vkCreateSemaphore(m_API->getDevice(), &sema, nullptr, &m_ImageAvailable[i]);
          if (VK_SUCCESS != result) PLP_ERROR("can't create m_ImageAvailable semaphore");

          result = vkCreateFence(m_API->getDevice(), &fenceInfo, nullptr, &m_ImagesInFlight[i]);
          if (VK_SUCCESS != result) PLP_ERROR("can't create m_PreviousFrame fence");

          result = vkCreateFence(m_API->getDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]);
          if (VK_SUCCESS != result) PLP_ERROR("can't create m_InFlightFences fence");
      }
  }

  void Renderer::recreateSwapChain()
  {
 
  }

  void Renderer::shouldRecreateSwapChain()
  {
      if (Window::m_FramebufferResized == true) {

          while (m_Window->isMinimized()) {
              m_Window->wait();
          }
          recreateSwapChain();

          Window::m_FramebufferResized = false;
      }
  }

  void Renderer::setPerspective()
  {        
      m_Perspective = glm::perspective(glm::radians(45.0f),
          static_cast<float>(m_API->getSwapChainExtent().width) / static_cast<float>(m_API->getSwapChainExtent().height),
          0.1f, 100.f);
      m_Perspective[1][1] *= -1;
  }

  void Renderer::setDeltatime(float deltaTime)
  {
      m_Deltatime = deltaTime;
  }

  void Renderer::drawShadowMap(VkCommandBuffer &cmdBuffer)
  {
    std::string const pipelineName{ "shadowMap" };
    auto const& pipeline = getPipeline(pipelineName);
  
    //m_API->beginCommandBuffer(m_CommandBuffersEntities[m_CurrentFrame]);
    m_API->startMarker(cmdBuffer, "shadow_map_" + pipelineName, 0.1f, 0.2f, 0.3f);

    m_API->transitionImageLayout(cmdBuffer, m_DepthMapImages[0],
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkClearColorValue colorClear = {};
    colorClear.float32[0] = 1.0f;
    colorClear.float32[1] = 1.0f;
    colorClear.float32[2] = 1.0f;
    colorClear.float32[3] = 1.0f;
      
    VkClearDepthStencilValue depthStencil = { 1.f, 0 };

    VkRenderingAttachmentInfo depthAttachment{ };
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageView = m_DepthMapImageViews[0];
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.clearValue.depthStencil = depthStencil;
    depthAttachment.clearValue.color = colorClear;

    uint32_t const width{ 2048 };// m_API->getSwapChainExtent().width
    uint32_t const height{ 2048 };//  m_API->getSwapChainExtent().height

    VkRenderingInfo  renderingInfo{ };
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.extent.width = width;
    renderingInfo.renderArea.extent.height = height;
    renderingInfo.layerCount = 1;
    renderingInfo.pDepthAttachment = & depthAttachment;
    renderingInfo.colorAttachmentCount = 0;
    renderingInfo.flags = VK_SUBPASS_CONTENTS_INLINE;

    vkCmdBeginRenderingKHR(cmdBuffer, & renderingInfo);

    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    VkRect2D scissor = { { 0, 0 }, { width, height } };

    vkCmdSetScissor(cmdBuffer, 0, 1, & scissor);

    float const depthBiasConstant = 0.0f;
    float const depthBiasSlope = 0.0f;

    //vkCmdSetDepthClampEnableEXT(m_CommandBuffersEntities[m_CurrentFrame], VK_TRUE);
    //vkCmdSetDepthBias(m_CommandBuffersEntities[m_CurrentFrame], depthBiasConstant, 0.0f, depthBiasSlope);

    std::string last_shader{};
    bool need_pipeline_update{true};
    
    std::ranges::for_each(m_Entities, [&](auto const& entity) {
      auto meshComponent = m_ComponentManager->getComponent<MeshComponent>(entity->getID());
      if (meshComponent) {
        Mesh* mesh = meshComponent->template hasImpl<Mesh>();

        if (mesh->hasShadow() && !mesh->getUniformBuffers()->empty()) {

          auto const& current_shader = mesh->getShaderName();

          if (last_shader.empty()) {
            last_shader = current_shader;
          }
          if (last_shader != current_shader) {
            need_pipeline_update = true;
            last_shader = current_shader;
          }

          uint32_t min{ 0 };
          uint32_t max{ 0 };

          for (size_t i{ 0 }; i < mesh->getUniformBuffers()->size(); ++i) {
            max = mesh->getData()->m_UbosOffset.at(i);
            auto ubos = std::vector<UniformBufferObject>(mesh->getData()->m_Ubos.begin() + min, mesh->getData()->m_Ubos.begin() + max);
            min = max;
            if (ubos.empty()) continue;
            m_API->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &ubos);
          }

          constants pushConstants{};
          pushConstants.textureIDBB = glm::vec3(mesh->getData()->m_TextureIndex, 0.0, 0.0);
          pushConstants.view = getCamera()->lookAt();
          pushConstants.viewPos = getCamera()->getPos();

          vkCmdPushConstants(cmdBuffer, pipeline->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants),
            &pushConstants);

          vkCmdBindDescriptorSets(
            cmdBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->pipelineLayout,
            0, 1, mesh->getShadowMapDescSet(), 0, nullptr);

          if (need_pipeline_update) {
            m_API->bindPipeline(cmdBuffer, pipeline->pipeline);
            need_pipeline_update = false;
          }

          m_API->draw(
            cmdBuffer,
            *mesh->getShadowMapDescSet(),
            *pipeline,
            mesh->getData(),
            mesh->getData()->m_Ubos.size(),
            mesh->isIndexed());
        }
      }
    });

    m_API->endMarker(cmdBuffer);
    m_API->endRendering(cmdBuffer);

    m_API->transitionImageLayout(cmdBuffer, m_DepthMapImages[0],
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

    m_DepthMapDescSetUpdated = true;
  }

  void Renderer::draw(
    VkCommandBuffer& cmdBuffer, 
    VkImageView& colorView,
    VkImage& color,
    VkImageView& depthView,                      
    VkImage& depth,
    std::vector<Entity*> const& entities,
    std::latch& count_down,
    unsigned int thread_id,
    bool shadows)
  {
    if (0 < entities.size()) {
      m_API->beginCommandBuffer(cmdBuffer);

      if (shadows) {
        drawShadowMap(cmdBuffer);
      }

      m_API->transitionImageLayout(cmdBuffer, color,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

      m_API->transitionImageLayout(cmdBuffer, depth,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

      m_API->beginRendering(
        cmdBuffer,
        colorView,
        depthView,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE);

      m_API->setViewPort(cmdBuffer);
      m_API->setScissor(cmdBuffer);

      m_API->startMarker(cmdBuffer, "drawing", 0.2f, 0.2f, 0.9f);

      //std::vector<VkDrawIndexedIndirectCommand> drawCommands{};
      //drawCommands.reserve(m_Entities.size());

      //unsigned int firstInstance { 0 };
      //std::ranges::for_each(m_Entities, [&](auto const& entity) {
      //  auto meshComponent = m_ComponentManager->getComponent<MeshComponent>(entity->getID());
      //  if (meshComponent) {
      //    Mesh* mesh = meshComponent->hasImpl<Mesh>();

      //    drawCommands.emplace_back(
      //       mesh->getData()->m_Vertices.size(),
      //       1, 0, 0, firstInstance);
      //    firstInstance += 1;
      //  }
      //});

      //auto indirectBuffer = m_API->createIndirectCommandsBuffer(drawCommands);

      std::string last_shader{};
      bool need_pipeline_update{true};

      size_t num{ 0 };
      std::ranges::for_each(entities, [&](auto const& entity) {
        auto meshComponent = m_ComponentManager->getComponent<MeshComponent>(entity->getID());
        if (meshComponent) {

          Mesh* mesh = meshComponent->template hasImpl<Mesh>();
          if (!mesh->getUniformBuffers()->empty()) {

            auto const& current_shader {mesh->getShaderName()};
            auto pipeline = getPipeline(current_shader);

            if (last_shader.empty()) {
              last_shader = current_shader;
            }
            if (last_shader != current_shader) {
              need_pipeline_update = true;
              last_shader = current_shader;
            }
            
            for (size_t i{ 0 }; i < mesh->getData()->m_Ubos.size(); ++i) {
              mesh->getData()->m_Ubos[i].projection = getPerspective();
            }

            if (mesh->getData()->m_UbosOffset.size() > 0) {
              uint32_t min{ 0 };
              uint32_t max{ 0 };

              for (size_t i{ 0 }; i < mesh->getUniformBuffers()->size(); ++i) {
                max = mesh->getData()->m_UbosOffset.at(i);
                auto ubos = std::vector<UniformBufferObject>(mesh->getData()->m_Ubos.begin() + min, mesh->getData()->m_Ubos.begin() + max);
                min = max;
                if (ubos.empty()) continue;
                m_API->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &ubos);
              }
            }

            if (mesh->hasPushConstants()) {
              mesh->applyPushConstants(cmdBuffer, pipeline->pipelineLayout, this, mesh);
            }

            vkCmdBindDescriptorSets(cmdBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline->pipelineLayout,
                                    0, 1, mesh->getDescSet(), 0, nullptr);

            if (need_pipeline_update) {
              m_API->bindPipeline(cmdBuffer, pipeline->pipeline);
              need_pipeline_update = false;
            }

            m_API->draw(cmdBuffer, *mesh->getDescSet(), *pipeline, mesh->getData(), mesh->isIndexed());
            /*vkCmdDrawIndexedIndirect(
              m_CommandBuffersEntities[m_CurrentFrame],
              indirectBuffer.buffer,
              0,
              1,
              sizeof(VkDrawIndexedIndirectCommand));*/
            
            num += 1;
          }
        }
      });
      if (entities.size() > 0) {
        m_CmdsToSubmit.emplace_back(
          &cmdBuffer, 
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          &m_EntitiesSemaRenderFinished[thread_id]
        );
      }
      m_API->endMarker(cmdBuffer);
      endRendering(cmdBuffer, color, depth);
      count_down.count_down();
    }
  }

  void Renderer::renderScene()
  {
    vkWaitForFences(m_API->getDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(m_API->getDevice(), 1, &m_InFlightFences[m_CurrentFrame]);
    VkResult result = vkAcquireNextImageKHR(m_API->getDevice(), m_SwapChain, UINT64_MAX, m_ImageAvailable[m_CurrentFrame], VK_NULL_HANDLE, &m_ImageIndex);

    //@todo clean
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        PLP_ERROR("Error on vkAcquireNextImageKHR {}", result);
    }
      
    {
      std::lock_guard guard(m_MutexEntitySubmit);
      std::latch count_down{2};

      if (m_EntityManager->getSkybox()) {
        std::jthread pack2([this, &count_down]() {            
          draw(m_CommandBuffersEntitiesBis[m_CurrentFrame],
              m_ImageViews[m_CurrentFrame],
              m_Images[m_CurrentFrame],          
              m_DepthImageViews[m_CurrentFrame], 
              m_DepthImages[m_CurrentFrame],
              {m_EntityManager->getSkybox()},
              count_down,
              0);
        });
        pack2.detach();
      } else {
        count_down.count_down();
      };
      
      
      if (m_Entities.size() > 0) {
        std::jthread pack1([this, &count_down]() {
            draw(
              m_CommandBuffersEntities[m_CurrentFrame],
              m_ImageViews[m_CurrentFrame],
              m_Images[m_CurrentFrame],          
              m_DepthImageViews[m_CurrentFrame], 
              m_DepthImages[m_CurrentFrame], 
              m_Entities,
              count_down,
              1,
              true
            );
          }
        );

        pack1.detach();
      } else {
        count_down.count_down();
      }

      count_down.wait();
        
      submit();
    }
  }

  void Renderer::clearRendererScreen()
  {
      // {
      //     vkResetCommandBuffer(m_CommandBuffersEntities[m_CurrentFrame], 0);
      //     vkResetCommandBuffer(m_CommandBuffersHUD[m_CurrentFrame], 0);
      //     vkResetCommandBuffer(m_CommandBuffersSkybox[m_CurrentFrame], 0);

      //     beginRendering(m_CommandBuffersEntities[m_CurrentFrame]);
      //     //do nothing !
      //     endRendering(m_CommandBuffersEntities[m_CurrentFrame]);

      //     beginRendering(m_CommandBuffersHUD[m_CurrentFrame]);
      //     //do nothing !
      //     endRendering(m_CommandBuffersHUD[m_CurrentFrame]);

      //     beginRendering(m_CommandBuffersSkybox[m_CurrentFrame], VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
      //     //do nothing !
      //     endRendering(m_CommandBuffersSkybox[m_CurrentFrame]);

      //     std::vector<VkCommandBuffer> cmds{};
      //     cmds.emplace_back(m_CommandBuffersSkybox[m_CurrentFrame]);
      //     cmds.emplace_back(m_CommandBuffersEntities[m_CurrentFrame]);
      //     cmds.emplace_back(m_CommandBuffersHUD[m_CurrentFrame]);

      //     if (getDrawBbox()) {
      //         vkResetCommandBuffer(m_CommandBuffersBbox[m_CurrentFrame], 0);

      //         beginRendering(m_CommandBuffersBbox[m_CurrentFrame]);
      //         //do nothing !
      //         endRendering(m_CommandBuffersBbox[m_CurrentFrame]);
      //         cmds.emplace_back(m_CommandBuffersBbox[m_CurrentFrame]);
      //     }

      //     submit();
      // }
  }

  void Renderer::destroy()
  {
      /* m_API->destroySwapchain(m_API->getDevice(), m_SwapChain, {}, m_SwapChainImageViews);

      m_API->destroyFences();

      for (auto item: m_SwapChainDepthImages) {
          vkDestroyImage(m_API->getDevice(), item, nullptr);
      }
      for (auto item : m_SwapChainDepthImageViews) {
          vkDestroyImageView(m_API->getDevice(), item, nullptr);
      }
      for (auto& buffer : m_UniformBuffers.first) {
          m_API->destroyBuffer(buffer);
      }*/
      for (VkDescriptorSetLayout descriptorSetLayout : m_DescriptorSetLayouts) {
          vkDestroyDescriptorSetLayout(m_API->getDevice(), descriptorSetLayout, nullptr);
      }
      for (VkDescriptorPool descriptorPool : m_DescriptorPools) {
          vkDestroyDescriptorPool(m_API->getDevice(), descriptorPool, nullptr);
      }
  }

  void Renderer::immediateSubmit(std::function<void(VkCommandBuffer cmd)> && function,
       int queueIndex)
  {
      auto commandPool = m_API->createCommandPool();
      VkCommandBuffer cmd = m_API->allocateCommandBuffers(commandPool)[0];
      m_API->beginCommandBuffer(cmd);
      function(cmd);
      m_API->endCommandBuffer(cmd);
      m_API->queueSubmit(cmd);
      vkDestroyCommandPool(m_API->getDevice(), commandPool, nullptr);
  }

  //@todo do to much to refacto
  void Renderer::beginRendering(
    VkCommandBuffer commandBuffer,
    VkImageView imageView,
    VkImage image,
    VkImageView depthImageView,
    VkImage depthImage,
    VkAttachmentLoadOp loadOp,
    VkAttachmentStoreOp storeOp,
    bool continuousCmdBuffer)
  {
      if (!continuousCmdBuffer) m_API->beginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

      m_API->transitionImageLayout(commandBuffer, image,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

      m_API->transitionImageLayout(commandBuffer, depthImage,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

      m_API->beginRendering(commandBuffer,
        imageView,
        depthImageView,
        loadOp,
        storeOp);

      m_API->setViewPort(commandBuffer);
      m_API->setScissor(commandBuffer);
  }

  void Renderer::endRendering(VkCommandBuffer commandBuffer, VkImage image, VkImage depthImage)
  {
      m_API->endRendering(commandBuffer);

      m_API->transitionImageLayout(commandBuffer, image,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT);

      m_API->transitionImageLayout(commandBuffer, depthImage,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_DEPTH_BIT);

      m_API->endCommandBuffer(commandBuffer);
  }

  void Renderer::submit()
  {
    if (m_CmdsToSubmit.empty()) {
      return;
    }
    
    std::vector<VkCommandBuffer*> cmdBuffers{};
    std::vector<VkPipelineStageFlags> waitStages{ };
    std::vector<VkSemaphore> waitSemaphores{ m_ImageAvailable[m_CurrentFrame] };
    std::vector<VkSemaphore*> signalSemaphores{};

    for (DrawCommand const& cmdToSubmit : m_CmdsToSubmit) {
      cmdBuffers.push_back(cmdToSubmit.buffer);
      signalSemaphores.push_back(cmdToSubmit.semaphore);
      waitStages.push_back(cmdToSubmit.stageFlags);
    }

    //PLP_WARN("submit: {} commands to index: {}", m_CmdsToSubmit.size(), m_CurrentFrame);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pCommandBuffers = *cmdBuffers.data();
    submitInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffers.size());
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
    submitInfo.pSignalSemaphores = *signalSemaphores.data();
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    submitInfo.pWaitSemaphores = waitSemaphores.data();

    auto queue = m_API->getGraphicsQueues().at(0);

    std::vector<VkSwapchainKHR> swapChains{ m_SwapChain };

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
    presentInfo.pWaitSemaphores = *signalSemaphores.data();
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains.data();
    presentInfo.pImageIndices = &m_CurrentFrame;

    presentInfo.pResults = nullptr;
    m_API->submit(queue, submitInfo, presentInfo,  m_InFlightFences[m_CurrentFrame]);

    m_CurrentFrame = (m_CurrentFrame + 1) % static_cast<uint32_t>(m_MAX_FRAMES_IN_FLIGHT);
    //PLP_WARN("current frame: {}", m_ImageIndex);
      
    m_CmdsToSubmit.clear();

    onFinishRender();
  }

  void Renderer::setRayPick(float x, float y, float z,  int width,  int height)
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

  void Renderer::clear()
  {
      m_EntityManager->clear();
  }

  void Renderer::onFinishRender()
  {
      Event event{ "OnFinishRender" };
      for (const auto& observer : m_Observers) {
          observer->notify(event);
      }
  }

  void Renderer::attachObserver(IObserver* observer)
  {
      m_Observers.push_back(observer);
  }

  /*VkRenderPass Renderer::createImGuiRenderPass(VkFormat format)
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

      VkResult result = vkCreateRenderPass(m_API->getDevice(), & info, nullptr, & renderPass);

      if (result != VK_SUCCESS) {
          PLP_FATAL("failed to create imgui render pass : {}", result);
      }
      return renderPass;
  }*/

  void Renderer::showGrid(bool show)
  {
      for (auto & hudPart : *m_EntityManager->getHUD()) {
          if ("grid" == hudPart->getName()) {
              hudPart->setVisible(show);
          }
      }
  }

  void Renderer::addPipeline(std::string const & shaderName, VulkanPipeline pipeline)
  {
      m_Pipelines[shaderName] = std::move(pipeline);
  }

  VkPipeline Renderer::createGraphicsPipeline(
  VkPipelineLayout pipelineLayout,
  std::string_view name,
  std::vector<VkPipelineShaderStageCreateInfo> shadersCreateInfos,
  VkPipelineVertexInputStateCreateInfo & vertexInputInfo,
  VkCullModeFlagBits cullMode,
  bool depthTestEnable,
  bool depthWriteEnable,
  bool stencilTestEnable,
  int polygoneMode, 
  bool hasColorAttachment,
  bool dynamicDepthBias) {
      return m_API->createGraphicsPipeline(
          pipelineLayout,
          name,
          shadersCreateInfos,
          vertexInputInfo,
          cullMode,
          depthTestEnable, depthWriteEnable, stencilTestEnable,
          polygoneMode,
          hasColorAttachment,
          dynamicDepthBias
      );
  }

  void Renderer::addEntities(std::vector<Entity*> entities)
  {
    {
      std::lock_guard guard(m_MutexEntitySubmit);
      copy(entities.begin(), entities.end(), back_inserter(m_Entities));
      m_DepthMapDescSetUpdated = false;
    }
  }

  void Renderer::addEntity(Entity* entity)
  {
    {
      std::lock_guard guard(m_MutexEntitySubmit);
      m_Entities.emplace_back(entity);
      m_DepthMapDescSetUpdated = false;
    }
  }

  void Renderer::updateData(std::string const& name, UniformBufferObject const& ubo, std::vector<Vertex> const& vertices)
  {
    {
      std::lock_guard guard(m_MutexEntitySubmit);

      auto const& entityIt = std::ranges::find_if(m_Entities, [&name](auto const& entity) {
        return entity->getName() == name;
      });

      if (entityIt != m_Entities.end()) {
        auto const& entity = *entityIt;
        auto meshComponent = m_ComponentManager->getComponent<MeshComponent>(entity->getID());
        if (meshComponent) {
          Mesh* mesh = meshComponent->hasImpl<Mesh>();
          //std::copy(vertices.begin(), vertices.end(), back_inserter(mesh->getData()->m_Vertices));
          mesh->addUbos({ std::move(ubo) });

          //auto basicRdrImpl = m_ComponentManager->getComponent<RenderComponent>(entity->getID());
          //auto deltaTime = std::chrono::duration<float, std::milli>(0);
          //basicRdrImpl->visit(deltaTime, mesh);
          return;
        }
      }
    }
  }
}
