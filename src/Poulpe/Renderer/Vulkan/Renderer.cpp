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

    _imageviews.resize(_images.size());
    _samplers.resize(_images.size());
    _depth_images.resize(_images.size());
    _depth_imageviews.resize(_images.size());
    _depth_samplers.resize(_images.size());

    _depth_images2.resize(_images.size());
    _depth_imageviews2.resize(_images.size());
    _depth_samplers2.resize(_images.size());

    _imageviews2.resize(_images.size());
    _images2.resize(_images.size());
    _samplers2.resize(_images.size());

    _imageviews3.resize(_images.size());
    _images3.resize(_images.size());
    _samplers3.resize(_images.size());

    for (size_t i{ 0 }; i < _images.size(); ++i) {
      VkImage image;

      _vulkan->createImage(_vulkan->getSwapChainExtent().width, _vulkan->getSwapChainExtent().height, 1,
          VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT 
          | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image);

      _imageviews[i] = _vulkan->createImageView(_images[i],
        _vulkan->getSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
      _samplers[i] = _vulkan->createTextureSampler(1);

      VkImage depth_image;

      _vulkan->createImage(
        _vulkan->getSwapChainExtent().width,
        _vulkan->getSwapChainExtent().height, 1,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_SAMPLED_BIT
        | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
        | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depth_image);

      VkImageView depth_imageview = _vulkan->createImageView(
        depth_image,
        VK_FORMAT_D24_UNORM_S8_UINT,
        1,
        VK_IMAGE_ASPECT_DEPTH_BIT);

      _depth_images[i] = std::move(depth_image);
      _depth_imageviews[i] = std::move(depth_imageview);
      _depth_samplers[i] = std::move(_vulkan->createTextureSampler(1));

      VkImage depth_image2;

      _vulkan->createImage(
        _vulkan->getSwapChainExtent().width,
        _vulkan->getSwapChainExtent().height,
        1,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_SAMPLED_BIT
        | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
        | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depth_image2);

      VkImageView depth_imageview2 = _vulkan->createImageView(
        depth_image2,
        VK_FORMAT_D24_UNORM_S8_UINT,
        1,
        VK_IMAGE_ASPECT_DEPTH_BIT);

      _depth_images2[i] = std::move(depth_image2);
      _depth_imageviews2[i] = std::move(depth_imageview2);
      _depth_samplers2[i] = std::move(_vulkan->createTextureSampler(1));
    }

    _cmd_pool_entities = _vulkan->createCommandPool();
    _cmd_pool_entities2 = _vulkan->createCommandPool();
    _cmd_pool_entities3 = _vulkan->createCommandPool();

    _cmd_buffer_entities = _vulkan->allocateCommandBuffers(_cmd_pool_entities,
      static_cast<uint32_t>(_imageviews.size()));

    _cmd_buffer_entities2 = _vulkan->allocateCommandBuffers(_cmd_pool_entities2,
      static_cast<uint32_t>(_imageviews.size()));

    _cmd_buffer_entities3 = _vulkan->allocateCommandBuffers(_cmd_pool_entities3,
      static_cast<uint32_t>(_imageviews.size()));

    _cmd_pool_shadowmap = _vulkan->createCommandPool();
    _cmd_buffer_shadowmap = _vulkan->allocateCommandBuffers(_cmd_pool_shadowmap,
      static_cast<uint32_t>(_imageviews.size()));

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
    _image_available.resize(_MAX_RENDER_THREAD);

    for (size_t i { 0 }; i < _MAX_RENDER_THREAD; ++i) {
      result = vkCreateSemaphore(_vulkan->getDevice(), &sema_create_info, nullptr, &_entities_sema_finished[i]);
      if (VK_SUCCESS != result) PLP_ERROR("can't create _entities_sema_finished semaphore");
    }
    for (size_t i { 0 }; i < _MAX_FRAMES_IN_FLIGHT; ++i) {
      result = vkCreateSemaphore(_vulkan->getDevice(), &sema_create_info, nullptr, &_image_available[i]);
      if (VK_SUCCESS != result) PLP_ERROR("can't create _image_available semaphore");

      result = vkCreateFence(_vulkan->getDevice(), &fence_info, nullptr, &_images_in_flight[i]);
      if (VK_SUCCESS != result) PLP_ERROR("can't create _images_in_flight fence");

      result = vkCreateFence(_vulkan->getDevice(), &fence_info, nullptr, &_fences_in_flight[i]);
      if (VK_SUCCESS != result) PLP_ERROR("can't create _fences_in_flight fence");
    }
  }

  void Renderer::setPerspective()
  {
    _perspective = glm::perspective(
      glm::radians(45.0f),
      static_cast<float>(_vulkan->getSwapChainExtent().width) / static_cast<float>(_vulkan->getSwapChainExtent().height),
      0.1f, 50.f);
    _perspective[1][1] *= -1;
  }

  void Renderer::setDeltatime(float delta_time)
  {
    _delta_time = delta_time;
  }

  void Renderer::drawShadowMap(
    VkCommandBuffer& cmd_buffer,
    DrawCommands& draw_cmds,
    VkImageView& depthview,
    VkImage& depth,
    std::vector<Entity*> const& entities,
    std::latch& count_down,
    unsigned int const thread_id
  )
  {
    std::string const pipeline_name{ "shadowMap" };
    auto const& pipeline = getPipeline(pipeline_name);
  
    _vulkan->beginCommandBuffer(cmd_buffer);
    _vulkan->startMarker(cmd_buffer, "shadow_map_" + pipeline_name, 0.1f, 0.2f, 0.3f);
    
    _vulkan->transitionImageLayout(cmd_buffer, depth,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkClearColorValue color_clear = {};
    color_clear.float32[0] = 1.0f;
    color_clear.float32[1] = 1.0f;
    color_clear.float32[2] = 1.0f;
    color_clear.float32[3] = 0.0f;

    VkClearDepthStencilValue depth_stencil = { 1.f, 0 };

    VkRenderingAttachmentInfo depth_attachment_info{ };
    depth_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depth_attachment_info.imageView = depthview;
    depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment_info.clearValue.depthStencil = depth_stencil;
    depth_attachment_info.clearValue.color = color_clear;

    uint32_t const width{ _vulkan->getSwapChainExtent().width * 4 };
    uint32_t const height{ _vulkan->getSwapChainExtent().height * 4 };
    //uint32_t const width{ 2048 };
    //uint32_t const height{ 2048 };

    VkRenderingInfo  rendering_info{ };
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.renderArea.extent.width = width;
    rendering_info.renderArea.extent.height = height;
    rendering_info.layerCount = 1;
    rendering_info.pDepthAttachment = &depth_attachment_info;
    rendering_info.colorAttachmentCount = 0;
    //rendering_info.flags = VK_SUBPASS_CONTENTS_INLINE;

    vkCmdBeginRenderingKHR(cmd_buffer, &rendering_info);

    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
    VkRect2D scissor = { { 0, 0 }, { width, height } };

    vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

    //float const depth_bias_constant{ 1.0f };
    //float const depth_bias_slope{ 1.5f };
    //float const depth_bias_clamp{ 0.0f };

    //vkCmdSetDepthClampEnableEXT(cmd_buffer, VK_TRUE);
    //vkCmdSetDepthBias(cmd_buffer, depth_bias_constant, depth_bias_clamp, depth_bias_slope);    
    std::ranges::for_each(entities, [&](auto const& entity) {
      auto mesh_component = _component_manager->get<MeshComponent>(entity->getID());
      if (mesh_component) {
        Mesh* mesh = mesh_component->template has<Mesh>();

        if (mesh->has_shadow() && !mesh->getUniformBuffers()->empty()) {

          uint32_t min{ 0 };
          uint32_t max{ 0 };

          for (size_t i{ 0 }; i < mesh->getUniformBuffers()->size(); ++i) {
            max = mesh->getData()->_ubos_offset.at(i);
            auto ubos = std::vector<UniformBufferObject>(mesh->getData()->_ubos.begin() + min, mesh->getData()->_ubos.begin() + max);
            min = max;
            if (ubos.empty()) continue;
            _vulkan->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &ubos);
          }

          vkCmdBindDescriptorSets(
            cmd_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->pipeline_layout,
            0, 1, mesh->getShadowMapDescSet(), 0, nullptr);

          _vulkan->bindPipeline(cmd_buffer, pipeline->pipeline);

          _vulkan->draw(
            cmd_buffer,
            *mesh->getShadowMapDescSet(),
            *pipeline,
            mesh->getData(),
            mesh->getData()->_ubos.size(),
            mesh->is_indexed());
        }
      }
    });

    _vulkan->endMarker(cmd_buffer);
    _vulkan->endRendering(cmd_buffer);

     _vulkan->transitionImageLayout(cmd_buffer, depth,
       VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT); 
 /*     _vulkan->transitionImageLayout(cmd_buffer, depth,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_DEPTH_BIT);*/

    _vulkan->endCommandBuffer(cmd_buffer);

    std::vector<VkPipelineStageFlags> flags { VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
    //VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
    _draw_cmds.insert(&cmd_buffer, &_entities_sema_finished[thread_id], thread_id, flags);

    _update_shadow_map = false;
    count_down.count_down();
  }

  void Renderer::draw(
    VkCommandBuffer& cmd_buffer,
    DrawCommands& draw_cmds,
    VkImageView& colorview,
    VkImage& color,
    VkImageView& depthview,
    VkImage& depth,
    std::vector<Entity*> const& entities,
    VkAttachmentLoadOp const load_op,
    VkAttachmentStoreOp const store_op,
    std::latch& count_down,
    unsigned int const thread_id,
    bool const has_depth_attachment)
  {
    if (entities.empty()) {
      count_down.count_down();
      return;
    }

    _vulkan->beginCommandBuffer(cmd_buffer);

    _vulkan->transitionImageLayout(cmd_buffer, color,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

    if (has_depth_attachment) {
      _vulkan->transitionImageLayout(cmd_buffer, depth,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    }

    _vulkan->beginRendering(
      cmd_buffer,
      colorview,
      depthview,
      load_op,
      store_op,
      has_depth_attachment);

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

    size_t num{ 0 };
    std::ranges::for_each(entities, [&](auto const& entity) {
      auto mesh_component = _component_manager->get<MeshComponent>(entity->getID());
      if (mesh_component) {

        Mesh* mesh = mesh_component->template has<Mesh>();
        auto pipeline = getPipeline(mesh->getShaderName());

        if (mesh->hasPushConstants()) {
          mesh->applyPushConstants(cmd_buffer, pipeline->pipeline_layout, this, mesh);
        }

        vkCmdBindDescriptorSets(
          cmd_buffer,
          VK_PIPELINE_BIND_POINT_GRAPHICS,
          pipeline->pipeline_layout,
          0, 1, mesh->getDescSet(), 0, nullptr);

        _vulkan->bindPipeline(cmd_buffer, pipeline->pipeline);
        
        _vulkan->draw(
          cmd_buffer,
          *mesh->getDescSet(),
          *pipeline,
          mesh->getData(),
          mesh->is_indexed());
        /*vkCmdDrawIndexedIndirect(
          _CommandBuffersEntities[_current_frame],
          indirectBuffer.buffer,
          0,
          1,
          sizeof(VkDrawIndexedIndirectCommand));*/
          
        num += 1;
      }
    });
    _vulkan->endMarker(cmd_buffer);
    endRendering(cmd_buffer, color, depth, has_depth_attachment);

    std::vector<VkPipelineStageFlags> flags { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    if (has_depth_attachment) flags.emplace_back(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    _draw_cmds.insert(&cmd_buffer, &_entities_sema_finished[thread_id], thread_id, flags);

    count_down.count_down();
  }

  void Renderer::renderScene()
  {
    vkWaitForFences(_vulkan->getDevice(), 1, &_fences_in_flight[_current_frame], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(_vulkan->getDevice(), _swapchain, UINT64_MAX, _image_available[_current_frame], VK_NULL_HANDLE, &_image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(_vulkan->getDevice(), 1, &_fences_in_flight[_current_frame]);

    std::latch count_down{3};

    if (_entity_manager->getSkybox()) {
      std::jthread skybox_thread([&]() {
        draw(
          _cmd_buffer_entities2[_current_frame],
          _draw_cmds,
          _imageviews[_current_frame],
          _images[_current_frame],
          _depth_imageviews[_current_frame],
          _depth_images[_current_frame],
          {_entity_manager->getSkybox()},
          VK_ATTACHMENT_LOAD_OP_CLEAR,
          VK_ATTACHMENT_STORE_OP_STORE,
          count_down,
          0,
          false);
       });
       skybox_thread.join();
    } else {
      count_down.count_down();
    }

    if (_entities.size() > 0) {
      if (_update_shadow_map) {
        std::jthread shadow_map_thread([&]() {
          drawShadowMap(
            _cmd_buffer_entities3[_current_frame],
            _draw_cmds,
            _depthmap_imageviews[_current_frame],
            _depthmap_images[_current_frame],
            _entities,
            count_down,
            1
          );
        });
        shadow_map_thread.detach();
      } else {
        count_down.count_down();
      }
        
      std::jthread entities_thread([&]() {
        draw(
         _cmd_buffer_entities[_current_frame],
         _draw_cmds,
         _imageviews[_current_frame],
         _images[_current_frame],
         _depth_imageviews[_current_frame],
         _depth_images[_current_frame],
         _entities,
         VK_ATTACHMENT_LOAD_OP_CLEAR,
         VK_ATTACHMENT_STORE_OP_STORE,
         count_down,
         2, true
        );
      });
      entities_thread.detach();
    } else {
      count_down.count_down();
      count_down.count_down();
    }

    count_down.wait();

    submit(_draw_cmds);

    if (_entities.size() <= 0) {
      swapBufferEntities();
    };
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
    bool const has_depth_attachment,
    bool continuous_cmd_buffer)
  {
      if (!continuous_cmd_buffer) _vulkan->beginCommandBuffer(cmd_buffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

      _vulkan->transitionImageLayout(cmd_buffer, image,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

      if (has_depth_attachment) {
        _vulkan->transitionImageLayout(cmd_buffer, depth_image,
          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
      }

      _vulkan->beginRendering(
        cmd_buffer,
        imageview,
        depth_imageview,
        load_op,
        store_op,
        has_depth_attachment);

      _vulkan->setViewPort(cmd_buffer);
      _vulkan->setScissor(cmd_buffer);
  }

  void Renderer::endRendering(
    VkCommandBuffer& cmd_buffer,
    VkImage& image,
    VkImage& depth_image,
    bool const has_depth_attachment)
  {
    _vulkan->endRendering(cmd_buffer);

    _vulkan->transitionImageLayout(cmd_buffer, image,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT);

    //if (has_depth_attachment) {
    //  _vulkan->transitionImageLayout(cmd_buffer, depth_image,
    //    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    //}

    _vulkan->endCommandBuffer(cmd_buffer);
  }

  void Renderer::submit(DrawCommands const& draw_cmds)
  {
    if (!draw_cmds.has_cmd()) {
      vkResetFences(_vulkan->getDevice(), 1, &_fences_in_flight[_current_frame]);
      return;
    }

    std::vector<VkSubmitInfo> submit_infos{};
    bool has_semaphore{ false };

    std::vector<VkSemaphore*> semaphores{ };

    for (size_t i{ 0 }; i < draw_cmds.cmd_buffers.size(); ++i) {

      if (draw_cmds.cmd_buffers.at(i) == nullptr) continue;

      VkSubmitInfo submit_info{};
      submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submit_info.pCommandBuffers = draw_cmds.cmd_buffers.at(i);
      submit_info.commandBufferCount = 1;
      submit_info.pWaitDstStageMask = draw_cmds.stage_flags.at(i).data();
      submit_info.signalSemaphoreCount = 1;
      submit_info.pSignalSemaphores = draw_cmds.semaphores.at(i);

      //@todo ugly but works ?
      if (!has_semaphore) {
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &_image_available[_current_frame];
        has_semaphore = true;
      }
      submit_infos.emplace_back(submit_info);
      
      semaphores.push_back(draw_cmds.semaphores.at(i));
    };
    
    auto queue = _vulkan->getGraphicsQueues().at(0);

    std::vector<VkSwapchainKHR> swapchains{ _swapchain };

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = static_cast<uint32_t>(semaphores.size());
    present_info.pWaitSemaphores = *semaphores.data();
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains.data();
    present_info.pImageIndices = &_image_index;

    present_info.pResults = nullptr;

    _vulkan->submit(queue, submit_infos, present_info, _fences_in_flight[_current_frame]);

    _current_frame = (_current_frame + 1) % _MAX_FRAMES_IN_FLIGHT;

    _draw_cmds.clear();
    onFinishRender();

    if (!_entities_buffer.empty()) {
      swapBufferEntities();
    }
  }

  void Renderer::setRayPick(
    float const x,
    float const y,
    float const z,
    int const width,
    int const height)
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

  void Renderer::showGrid(bool const show)
  {
    for (auto & hud : *_entity_manager->getHUD()) {
      if ("grid" == hud->getName()) {
        hud->setVisible(show);
      }
    }
  }

  void Renderer::addPipeline(
    std::string const& shaderName,
    VulkanPipeline& pipeline)
  {
    _pipelines[shaderName] = std::move(pipeline);
  }

  void Renderer::addEntities(std::vector<Entity*> entities)
  {
    {
      std::lock_guard guard(_mutex_entity_submit);
      copy(entities.begin(), entities.end(), back_inserter(_entities_buffer));
    }
  }

  void Renderer::addEntity(Entity* entity, bool const is_last)
  {
    {
      std::lock_guard guard(_mutex_entity_submit);

      _entities_buffer.emplace_back(entity);

      _force_entities_buffer_swap = is_last;
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
          //auto delta_time = std::chrono::duration<float, std::milli>(0);
          //basicRdrImpl->visit(delta_time, mesh);
          return;
        }
      }
    }
  }

  void Renderer::swapBufferEntities()
  {
    if (_entities_buffer.size() < _entities_buffer_swap_treshold 
        && !_force_entities_buffer_swap) return;

    {
      std::lock_guard guard(_mutex_entity_submit);
      copy(_entities_buffer.begin(), _entities_buffer.end(), back_inserter(_entities));
      _entities_buffer.clear();
      _entities_buffer.shrink_to_fit();
      _update_shadow_map = true;
      _force_entities_buffer_swap = false;
    }
  }
}
