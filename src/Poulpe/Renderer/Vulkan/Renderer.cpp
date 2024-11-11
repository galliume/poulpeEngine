#include "Renderer.hpp"

#include "VulkanAPI.hpp"

#include "Poulpe/Component/MeshComponent.hpp"

#include "Poulpe/GUI/Window.hpp"

#include "Poulpe/Manager/ComponentManager.hpp"

#include "Poulpe/Renderer/Vulkan/VulkanAPI.hpp"

#include <algorithm>
#include <cfenv>
#include <exception>
#include <future>
#include <memory>
#include <volk.h>

namespace Poulpe
{
  Renderer::Renderer(
    Window* const window,
    EntityManager* const entity_manager,
    ComponentManager* const component_manager,
    LightManager* const light_manager,
    TextureManager* const texture_manager)
      : _window(window),
        _entity_manager(entity_manager),
        _component_manager(component_manager),
        _light_manager(light_manager),
        _texture_manager(texture_manager)
  {
      _vulkan = std::make_unique<VulkanAPI>(window);
  }

  void Renderer::init()
  {
    setPerspective();

    _swapchain = _vulkan->createSwapChain(_images);

    _image_views.resize(_images.size());
    _samplers.resize(_images.size());
    _depth_images.resize(_images.size());
    _depth_image_views.resize(_images.size());
    _depth_samplers.resize(_images.size());

    _imageviews2.resize(_images.size());
    _images2.resize(_images.size());
    _samplers2.resize(_images.size());
    _depth_images2.resize(_images.size());
    _depth_imageviews2.resize(_images.size());
    _depth_samplers2.resize(_images.size());

    _imageviews3.resize(_images.size());
    _images3.resize(_images.size());
    _samplers3.resize(_images.size());
    _depth_images3.resize(_images.size());
    _depth_imageviews3.resize(_images.size());
    _depth_samplers3.resize(_images.size());

    for (size_t i{ 0 }; i < _images.size(); ++i) {
      VkImage image;

      _vulkan->createImage(_vulkan->getSwapChainExtent().width, _vulkan->getSwapChainExtent().height, 1,
          VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT 
          | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image);

      _image_views[i] = _vulkan->createImageView(_images[i],
        _vulkan->getSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
      _samplers[i] = _vulkan->createTextureSampler(1);

      VkImage depth_image;

      _vulkan->createImage(_vulkan->getSwapChainExtent().width, _vulkan->getSwapChainExtent().height, 1,
        VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT
        | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_image);

      VkImageView depth_imageview = _vulkan->createImageView(depth_image, VK_FORMAT_D32_SFLOAT, 1,
        VK_IMAGE_ASPECT_DEPTH_BIT);

      _depth_images[i] = depth_image;
      _depth_image_views[i] = depth_imageview;
      _depth_samplers[i] = _vulkan->createTextureSampler(1);
    }
    for (size_t i{ 0 }; i < _images.size(); ++i) {
      VkImageView depth_imageview = _vulkan->createImageView(_depth_images[i], VK_FORMAT_D32_SFLOAT, 1,
        VK_IMAGE_ASPECT_DEPTH_BIT);

      _depth_imageviews2[i] = depth_imageview;
      _depth_samplers2[i] = _vulkan->createTextureSampler(1);
    }
    for (size_t i{ 0 }; i < _images.size(); ++i) {
      VkImageView depth_imageview = _vulkan->createImageView(_depth_images[i], VK_FORMAT_D32_SFLOAT, 1,
        VK_IMAGE_ASPECT_DEPTH_BIT);
      _depth_imageviews3[i] = depth_imageview;
      _depth_samplers3[i] = _vulkan->createTextureSampler(1);
    }
    _cmd_pool_entities = _vulkan->createCommandPool();
    _cmd_pool_entities2 = _vulkan->createCommandPool();
    _cmd_pool_entities3 = _vulkan->createCommandPool();

    _cmd_buffer_entities = _vulkan->allocateCommandBuffers(_cmd_pool_entities,
      static_cast<uint32_t>(_image_views.size()));

    _cmd_buffer_entities2 = _vulkan->allocateCommandBuffers(_cmd_pool_entities2,
      static_cast<uint32_t>(_image_views.size()));

    _cmd_buffer_entities3 = _vulkan->allocateCommandBuffers(_cmd_pool_entities3,
      static_cast<uint32_t>(_image_views.size()));

    _cmd_pool_shadowmap = _vulkan->createCommandPool();
    _cmd_buffer_shadowmap = _vulkan->allocateCommandBuffers(_cmd_pool_shadowmap,
      static_cast<uint32_t>(_image_views.size()));

    for (size_t i { 0 }; i < _images.size(); ++i) {
      VkImage image{};
      _vulkan->createDepthMapImage(image);
      _depthmap_images.emplace_back(image);
      _depthmap_imageviews.emplace_back(_vulkan->createDepthMapImageView(image));
      _depthmap_samplers.emplace_back(_vulkan->createDepthMapSampler());

      // VkImage imageBis{};
      // _API->createDepthMapImage(imageBis);
      // _DepthMapImagesBis.emplace_back(imageBis);
      // _DepthMapImageViewsBis.emplace_back(_API->createDepthMapImageView(imageBis));
      // _DepthMapSamplersBis.emplace_back(_API->createDepthMapSampler());

      // VkImage imageTer{};
      // _API->createDepthMapImage(imageTer);
      // _DepthMapImagesTer.emplace_back(imageTer);
      // _DepthMapImageViewsTer.emplace_back(_API->createDepthMapImageView(imageTer));
      // _DepthMapSamplersTer.emplace_back(_API->createDepthMapSampler());
    }

    VkResult result{};

    VkSemaphoreTypeCreateInfo sema_type_info;
    sema_type_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    sema_type_info.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    sema_type_info.initialValue = 0;
    sema_type_info.pNext = nullptr;

    VkSemaphoreCreateInfo sema_create_info{};
    sema_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    sema_create_info.pNext = &sema_type_info;
      
    _fences_in_flight.resize(_MAX_FRAMES_IN_FLIGHT);
    _images_in_flight.resize(_MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    _entities_sema_finished.resize(_MAX_RENDER_THREAD);
    _shadowmap_sema_finished.resize(_MAX_RENDER_THREAD);
    _image_available.resize(_MAX_RENDER_THREAD);

    for (size_t i { 0 }; i < _MAX_RENDER_THREAD; ++i) {

      result = vkCreateSemaphore(_vulkan->getDevice(), &sema_create_info, nullptr, &_entities_sema_finished[i]);
      if (VK_SUCCESS != result) PLP_ERROR("can't create _EntitiesSemaRenderFinished semaphore");

      result = vkCreateSemaphore(_vulkan->getDevice(), &sema_create_info, nullptr, &_shadowmap_sema_finished[i]);
      if (VK_SUCCESS != result) PLP_ERROR("can't create _ShadowMapSemaRenderFinished semaphore");

      result = vkCreateSemaphore(_vulkan->getDevice(), &sema_create_info, nullptr, &_image_available[i]);
      if (VK_SUCCESS != result) PLP_ERROR("can't create _ImageAvailable semaphore");

      result = vkCreateFence(_vulkan->getDevice(), &fence_info, nullptr, &_images_in_flight[i]);
      if (VK_SUCCESS != result) PLP_ERROR("can't create _PreviousFrame fence");

      result = vkCreateFence(_vulkan->getDevice(), &fence_info, nullptr, &_fences_in_flight[i]);
      if (VK_SUCCESS != result) PLP_ERROR("can't create _InFlightFences fence");
    }
  }

  void Renderer::setPerspective()
  {        
      _perspective = glm::perspective(glm::radians(45.0f),
          static_cast<float>(_vulkan->getSwapChainExtent().width) / static_cast<float>(_vulkan->getSwapChainExtent().height),
          0.1f, 100.f);
      _perspective[1][1] *= -1;
  }

  void Renderer::setDeltatime(float deltatime)
  {
      _deltatime = deltatime;
  }

  void Renderer::drawShadowMap(VkCommandBuffer &cmd_buffer)
  {
    std::string const pipeline_name{ "shadowMap" };
    auto const& pipeline = getPipeline(pipeline_name);
  
    //_API->beginCommandBuffer(_CommandBuffersEntities[_CurrentFrame]);
    _vulkan->startMarker(cmd_buffer, "shadow_map_" + pipeline_name, 0.1f, 0.2f, 0.3f);

    _vulkan->transitionImageLayout(cmd_buffer, _depthmap_images[0],
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkClearColorValue color_clear = {};
    color_clear.float32[0] = 1.0f;
    color_clear.float32[1] = 1.0f;
    color_clear.float32[2] = 1.0f;
    color_clear.float32[3] = 1.0f;
      
    VkClearDepthStencilValue depth_stencil = { 1.f, 0 };

    VkRenderingAttachmentInfo depth_attachment_info{ };
    depth_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depth_attachment_info.imageView = _depthmap_imageviews[0];
    depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment_info.clearValue.depthStencil = depth_stencil;
    depth_attachment_info.clearValue.color = color_clear;

    uint32_t const width{ 2048 };// _API->getSwapChainExtent().width
    uint32_t const height{ 2048 };//  _API->getSwapChainExtent().height

    VkRenderingInfo  rendering_info{ };
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.renderArea.extent.width = width;
    rendering_info.renderArea.extent.height = height;
    rendering_info.layerCount = 1;
    rendering_info.pDepthAttachment = & depth_attachment_info;
    rendering_info.colorAttachmentCount = 0;
    rendering_info.flags = VK_SUBPASS_CONTENTS_INLINE;

    vkCmdBeginRenderingKHR(cmd_buffer, & rendering_info);

    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
    VkRect2D scissor = { { 0, 0 }, { width, height } };

    vkCmdSetScissor(cmd_buffer, 0, 1, & scissor);

    //float const depthBiasConstant = 0.0f;
    //float const depthBiasSlope = 0.0f;

    //vkCmdSetDepthClampEnableEXT(_CommandBuffersEntities[_CurrentFrame], VK_TRUE);
    //vkCmdSetDepthBias(_CommandBuffersEntities[_CurrentFrame], depthBiasConstant, 0.0f, depthBiasSlope);

    std::string last_shader{};
    bool need_pipeline_update{true};
    
    std::ranges::for_each(_entities, [&](auto const& entity) {
      auto mesh_component = _component_manager->get<MeshComponent>(entity->getID());
      if (mesh_component) {
        Mesh* mesh = mesh_component->template has<Mesh>();

        if (mesh->hasShadow() && !mesh->getUniformBuffers()->empty()) {

          auto const& current_shader = mesh->getShaderName();

          if (last_shader.empty()) {
            last_shader = current_shader;
          }
          if (last_shader != current_shader) {
            need_pipeline_update = true;
            last_shader = current_shader;
          }

          constants push_constants{};
          push_constants.textureIDBB = glm::vec3(mesh->getData()->_texture_index, 0.0, 0.0);
          push_constants.view = getCamera()->lookAt();
          push_constants.viewPos = getCamera()->getPos();

          vkCmdPushConstants(
            cmd_buffer,
            pipeline->pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0, sizeof(constants),
            &push_constants);

          vkCmdBindDescriptorSets(
            cmd_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->pipelineLayout,
            0, 1, mesh->getShadowMapDescSet(), 0, nullptr);

          if (need_pipeline_update) {
            _vulkan->bindPipeline(cmd_buffer, pipeline->pipeline);
            need_pipeline_update = false;
          }

          _vulkan->draw(
            cmd_buffer,
            *mesh->getShadowMapDescSet(),
            *pipeline,
            mesh->getData(),
            mesh->getData()->_ubos.size(),
            mesh->isIndexed());
        }
      }
    });

    _vulkan->endMarker(cmd_buffer);
    _vulkan->endRendering(cmd_buffer);

    _vulkan->transitionImageLayout(cmd_buffer, _depthmap_images[0],
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

    _depthmap_descset_updated = true;
  }

  void Renderer::draw(
    VkCommandBuffer& cmd_buffer,
    DrawCommands& draw_cmds,
    VkImageView& colorview,
    VkImage& color,
    VkImageView& depthview,
    VkImage& depth,
    std::vector<Entity*> const& entities,
    std::latch& count_down,
    unsigned int thread_id,
    bool shadows)
  {
    if (entities.empty()) return;

    vkResetCommandBuffer(cmd_buffer, 0);

    _vulkan->beginCommandBuffer(cmd_buffer);

    if (shadows) {
      drawShadowMap(cmd_buffer);
    }

    _vulkan->transitionImageLayout(cmd_buffer, color,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

    _vulkan->transitionImageLayout(cmd_buffer, depth,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

    _vulkan->beginRendering(
      cmd_buffer,
      colorview,
      depthview,
      VK_ATTACHMENT_LOAD_OP_CLEAR,
      VK_ATTACHMENT_STORE_OP_STORE);

    _vulkan->setViewPort(cmd_buffer);
    _vulkan->setScissor(cmd_buffer);

    _vulkan->startMarker(cmd_buffer, "drawing", 0.2f, 0.2f, 0.9f);

    //std::vector<VkDrawIndexedIndirectCommand> drawCommands{};
    //drawCommands.reserve(_Entities.size());

    //unsigned int firstInstance { 0 };
    //std::ranges::for_each(_Entities, [&](auto const& entity) {
    //  auto mesh_component = _component_manager->get<MeshComponent>(entity->getID());
    //  if (mesh_component) {
    //    Mesh* mesh = mesh_component->has<Mesh>();

    //    drawCommands.emplace_back(
    //       mesh->getData()->_vertices.size(),
    //       1, 0, 0, firstInstance);
    //    firstInstance += 1;
    //  }
    //});

    //auto indirectBuffer = _API->createIndirectCommandsBuffer(drawCommands);

    std::string last_shader{};
    bool need_pipeline_update{true};

    size_t num{ 0 };
    std::ranges::for_each(entities, [&](auto const& entity) {
      auto mesh_component = _component_manager->get<MeshComponent>(entity->getID());
      if (mesh_component) {

        Mesh* mesh = mesh_component->template has<Mesh>();
        auto const& current_shader {mesh->getShaderName()};
        auto pipeline = getPipeline(current_shader);

        if (last_shader.empty()) {
          last_shader = current_shader;
        }
        if (last_shader != current_shader) {
          need_pipeline_update = true;
          last_shader = current_shader;
        }

        if (mesh->hasPushConstants()) {
          mesh->applyPushConstants(cmd_buffer, pipeline->pipelineLayout, this, mesh);
        }

        vkCmdBindDescriptorSets(cmd_buffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipeline->pipelineLayout,
                                0, 1, mesh->getDescSet(), 0, nullptr);

        if (need_pipeline_update) {
          _vulkan->bindPipeline(cmd_buffer, pipeline->pipeline);
          need_pipeline_update = false;
        }

        _vulkan->draw(cmd_buffer, *mesh->getDescSet(), *pipeline, mesh->getData(), mesh->isIndexed());
        /*vkCmdDrawIndexedIndirect(
          _CommandBuffersEntities[_CurrentFrame],
          indirectBuffer.buffer,
          0,
          1,
          sizeof(VkDrawIndexedIndirectCommand));*/
          
        num += 1;
      }
    });
    _vulkan->endMarker(cmd_buffer);
    endRendering(cmd_buffer, color, depth);
    
    draw_cmds.insert(&cmd_buffer, &_entities_sema_finished[thread_id]);

    count_down.count_down();
  }

  void Renderer::renderScene()
  {
    if (_entities.size() <= 0) {
      swapBufferEntities();
      return;
    };

    vkWaitForFences(_vulkan->getDevice(), 1, &_fences_in_flight[_current_frame], VK_TRUE, UINT64_MAX);
    vkResetFences(_vulkan->getDevice(), 1, &_fences_in_flight[_current_frame]);
    VkResult result = vkAcquireNextImageKHR(_vulkan->getDevice(), _swapchain, UINT64_MAX, _image_available[_current_frame], VK_NULL_HANDLE, &_image_index);

    //@todo clean
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        PLP_ERROR("Error on vkAcquireNextImageKHR {}", result);
    }

    std::latch count_down{1};

    //if (_entity_manager->getSkybox()) {
    ////  std::jthread pack2([this, &draw_cmds, &count_down]() {
        //draw(_CommandBuffersEntitiesBis[_CurrentFrame],
        //    draw_cmds,
        //    _ImageViews[_CurrentFrame],
        //    _Images[_CurrentFrame],
        //    _DepthImageViews[_CurrentFrame],
        //    _DepthImages[_CurrentFrame],
        //    {_entity_manager->getSkybox()},
        //    count_down,
        //    0,
        //    false);
    //};
    //  pack2.detach();
    //} else {
      //count_down.count_down();
//    };

    if (_entities.size() > 0) {
  //    std::jthread pack1([this, &draw_cmds, &count_down]() {
          draw(
            _cmd_buffer_entities[_current_frame],
            _draw_cmds,
            _image_views[_current_frame],
            _images[_current_frame],
            _depth_image_views[_current_frame],
            _depth_images[_current_frame], 
            _entities,
            count_down,
            1,
            true
          );
        }
    //  );

    //  pack1.detach();
    //} else {
    //  PLP_DEBUG("count down !");
      //count_down.count_down();
    //}

    count_down.wait();
    
    submit(_draw_cmds);
  }

  void Renderer::clearRendererScreen()
  {
    // {
    //     vkResetCommandBuffer(_CommandBuffersEntities[_CurrentFrame], 0);
    //     vkResetCommandBuffer(_CommandBuffersHUD[_CurrentFrame], 0);
    //     vkResetCommandBuffer(_CommandBuffersSkybox[_CurrentFrame], 0);

    //     beginRendering(_CommandBuffersEntities[_CurrentFrame]);
    //     //do nothing !
    //     endRendering(_CommandBuffersEntities[_CurrentFrame]);

    //     beginRendering(_CommandBuffersHUD[_CurrentFrame]);
    //     //do nothing !
    //     endRendering(_CommandBuffersHUD[_CurrentFrame]);

    //     beginRendering(_CommandBuffersSkybox[_CurrentFrame], VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
    //     //do nothing !
    //     endRendering(_CommandBuffersSkybox[_CurrentFrame]);

    //     std::vector<VkCommandBuffer> cmds{};
    //     cmds.emplace_back(_CommandBuffersSkybox[_CurrentFrame]);
    //     cmds.emplace_back(_CommandBuffersEntities[_CurrentFrame]);
    //     cmds.emplace_back(_CommandBuffersHUD[_CurrentFrame]);

    //     if (getDrawBbox()) {
    //         vkResetCommandBuffer(_CommandBuffersBbox[_CurrentFrame], 0);

    //         beginRendering(_CommandBuffersBbox[_CurrentFrame]);
    //         //do nothing !
    //         endRendering(_CommandBuffersBbox[_CurrentFrame]);
    //         cmds.emplace_back(_CommandBuffersBbox[_CurrentFrame]);
    //     }

    //     submit();
    // }
  }

  void Renderer::destroy()
  {
    /* _API->destroySwapchain(_API->getDevice(), _SwapChain, {}, _SwapChainImageViews);

    _API->destroyFences();

    for (auto item: _SwapChainDepthImages) {
        vkDestroyImage(_API->getDevice(), item, nullptr);
    }
    for (auto item : _SwapChainDepthImageViews) {
        vkDestroyImageView(_API->getDevice(), item, nullptr);
    }
    for (auto& buffer : _UniformBuffers.first) {
        _API->destroyBuffer(buffer);
    }*/
    for (VkDescriptorSetLayout descriptorset_layout : _descriptorset_layouts) {
      vkDestroyDescriptorSetLayout(_vulkan->getDevice(), descriptorset_layout, nullptr);
    }
    for (VkDescriptorPool descriptor_pool : _descriptor_pools) {
      vkDestroyDescriptorPool(_vulkan->getDevice(), descriptor_pool, nullptr);
    }
  }

  void Renderer::immediateSubmit(
    std::function<void(VkCommandBuffer cmd)> && function,
    int queueIndex)
  {
    auto command_pool = _vulkan->createCommandPool();
    VkCommandBuffer cmd = _vulkan->allocateCommandBuffers(command_pool)[0];
    _vulkan->beginCommandBuffer(cmd);
    function(cmd);
    _vulkan->endCommandBuffer(cmd);
    _vulkan->queueSubmit(cmd);
    vkDestroyCommandPool(_vulkan->getDevice(), command_pool, nullptr);
  }

  //@todo do to much to refacto
  void Renderer::beginRendering(
    VkCommandBuffer& cmd_buffer,
    VkImageView& imageview,
    VkImage& image,
    VkImageView& depth_imageview,
    VkImage& depth_image,
    VkAttachmentLoadOp load_op,
    VkAttachmentStoreOp store_op,
    bool continuous_cmd_buffer)
  {
      if (!continuous_cmd_buffer) _vulkan->beginCommandBuffer(cmd_buffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

      _vulkan->transitionImageLayout(cmd_buffer, image,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

      _vulkan->transitionImageLayout(cmd_buffer, depth_image,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

      _vulkan->beginRendering(cmd_buffer,
        imageview,
        depth_imageview,
        load_op,
        store_op);

      _vulkan->setViewPort(cmd_buffer);
      _vulkan->setScissor(cmd_buffer);
  }

  void Renderer::endRendering(VkCommandBuffer& cmd_buffer, VkImage image, VkImage depthImage)
  {
      _vulkan->endRendering(cmd_buffer);

      _vulkan->transitionImageLayout(cmd_buffer, image,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT);

      _vulkan->transitionImageLayout(cmd_buffer, depthImage,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_DEPTH_BIT);

      _vulkan->endCommandBuffer(cmd_buffer);
  }

  void Renderer::submit(DrawCommands const& draw_cmds)
  {
    if (draw_cmds.cmd_buffers.empty()) {
      vkResetFences(_vulkan->getDevice(), 1, &_fences_in_flight[_current_frame]);
      return;
    }
    
    std::vector<VkSemaphore> wait_semaphores{ _image_available[_current_frame] };

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pCommandBuffers = *draw_cmds.cmd_buffers.data();
    submit_info.commandBufferCount = static_cast<uint32_t>(draw_cmds.cmd_buffers.size());
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pWaitDstStageMask = draw_cmds.stage_flags.data();
    submit_info.signalSemaphoreCount = static_cast<uint32_t>(draw_cmds.semaphores.size());
    submit_info.pSignalSemaphores = *draw_cmds.semaphores.data();
    submit_info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size());
    submit_info.pWaitSemaphores = wait_semaphores.data();

    auto queue = _vulkan->getGraphicsQueues().at(0);

    std::vector<VkSwapchainKHR> swapchains{ _swapchain };

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = static_cast<uint32_t>(draw_cmds.semaphores.size());
    present_info.pWaitSemaphores = *draw_cmds.semaphores.data();
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains.data();
    present_info.pImageIndices = &_current_frame;

    present_info.pResults = nullptr;

    _vulkan->submit(queue, submit_info, present_info, _fences_in_flight[_current_frame]);

    //_CurrentFrame = (_CurrentFrame + 1) % _MAX_FRAMES_IN_FLIGHT;
    _current_frame += 1;
    _current_frame = std::clamp(_current_frame, uint32_t{ 0 }, uint32_t{ 1 });
    //PLP_WARN("current frame: {}", _ImageIndex);
    if (!_entities_buffer.empty()) {
      swapBufferEntities();
    }

    _draw_cmds.clear();
    onFinishRender();
  }

  void Renderer::swapBufferEntities()
  {
    {
      std::lock_guard guard(_mutex_entity_submit);
      copy(_entities_buffer.begin(), _entities_buffer.end(), back_inserter(_entities));
      _entities_buffer.clear();
      _entities_buffer.shrink_to_fit();
    }
  }

  void Renderer::setRayPick(float x, float y, float z,  int width,  int height)
  {
      glm::vec3 rayNds = glm::vec3(x, y, z);
      glm::vec4 rayClip = glm::vec4(rayNds.x, rayNds.y, -1.0, 1.0);
      glm::vec4 rayEye = glm::inverse(getPerspective()) * rayClip;
      rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);

      glm::vec4 tmp = (glm::inverse(getCamera()->getView()) * rayEye);
      glm::vec3 rayWor = glm::vec3(tmp.x, tmp.y, tmp.z);
  }

  void Renderer::clear()
  {
      _entity_manager->clear();
  }

  void Renderer::onFinishRender()
  {
      //Event event{ "OnFinishRender" };
      //for (const auto& observer : _Observers) {
      //    observer->notify(event);
      //}
  }

  //void Renderer::attachObserver(IObserver* observer)
  //{
  //  _Observers.push_back(observer);
  //}

  /*VkRenderPass Renderer::createImGuiRenderPass(VkFormat format)
  {
      VkRenderPass renderPass{};

      VkAttachmentDescription attachment = {};
      attachment.format = format;
      attachment.samples = VK_SAMPLE_COUNT_1_BIT;
      attachment.load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      attachment.store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;
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

      VkResult result = vkCreateRenderPass(_API->getDevice(), & info, nullptr, & renderPass);

      if (result != VK_SUCCESS) {
          PLP_FATAL("failed to create imgui render pass : {}", result);
      }
      return renderPass;
  }*/

  void Renderer::showGrid(bool show)
  {
      for (auto & hud : *_entity_manager->getHUD()) {
          if ("grid" == hud->getName()) {
              hud->setVisible(show);
          }
      }
  }

  void Renderer::addPipeline(std::string const & shaderName, VulkanPipeline pipeline)
  {
      _pipelines[shaderName] = std::move(pipeline);
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
      return _vulkan->createGraphicsPipeline(
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
       std::lock_guard guard(_mutex_entity_submit);
      copy(entities.begin(), entities.end(), back_inserter(_entities_buffer));
      _depthmap_descset_updated = false;
    }
  }

  void Renderer::addEntity(Entity* entity)
  {
     {
      std::lock_guard guard(_mutex_entity_submit);
      _entities_buffer.emplace_back(entity);
      _depthmap_descset_updated = false;
    }
  }

  void Renderer::updateData(std::string const& name, UniformBufferObject const& ubo, std::vector<Vertex> const& vertices)
  {
    {
      std::lock_guard guard(_mutex_entity_submit);

      auto const& entity_it = std::ranges::find_if(_entities, [&name](auto const& entity) {
        return entity->getName() == name;
      });

      if (entity_it != _entities.end()) {
        auto const& entity = *entity_it;
        auto mesh_component = _component_manager->get<MeshComponent>(entity->getID());
        if (mesh_component) {
          Mesh* mesh = mesh_component->has<Mesh>();
          //std::copy(vertices.begin(), vertices.end(), back_inserter(mesh->getData()->_vertices));
          mesh->addUbos({ std::move(ubo) });

          //auto basicRdrImpl = _component_manager->get<RenderComponent>(entity->getID());
          //auto deltatime = std::chrono::duration<float, std::milli>(0);
          //basicRdrImpl->visit(deltatime, mesh);
          return;
        }
      }
    }
  }
}
