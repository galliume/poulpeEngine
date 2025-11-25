module;
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

#include <volk.h>

module Engine.Renderer.VulkanRenderer;

import std;

import Engine.Component.Components;

import Engine.Core.Logger;
import Engine.Core.MeshTypes;

import Engine.GUI.Window;

import Engine.Managers.ConfigManagerLocator;

import Engine.Renderer.Vulkan.Mesh;

namespace Poulpe
{
  Renderer::Renderer(Window* const window)
  {
    _vulkan = std::make_unique<VulkanAPI>(window);
    setPerspective();
  }

  void Renderer::init()
  {
    _swapchain = _vulkan->createSwapChain(_images);
    auto const buffer_size {_images.size()};

    _imageviews.resize(buffer_size);
    _samplers.resize(buffer_size);
    _depth_images.resize(buffer_size);
    _depth_imageviews.resize(buffer_size);
    _depth_samplers.resize(buffer_size);

    _depth_images2.resize(buffer_size);
    _depth_imageviews2.resize(buffer_size);
    _depth_samplers2.resize(buffer_size);

    _imageviews2.resize(buffer_size);
    _images2.resize(buffer_size);
    _samplers2.resize(buffer_size);
    _imageviews3.resize(buffer_size);
    _images3.resize(buffer_size);
    _samplers3.resize(buffer_size);

    for (std::size_t i{ 0 }; i < buffer_size; ++i) {
      VkImage image;

      _vulkan->createImage(
        _vulkan->getSwapChainExtent().width,
        _vulkan->getSwapChainExtent().height, 1,
        VK_SAMPLE_COUNT_1_BIT,
        _vulkan->PLP_VK_FORMAT_COLOR,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        image);

      _imageviews[i] = _vulkan->createImageView(
        _images[i],
        _vulkan->getSwapChainImageFormat(),
        VK_IMAGE_ASPECT_COLOR_BIT, 4);

      _samplers[i] = _vulkan->createTextureSampler(1);

      VkImage image2;

      _vulkan->createImage(
        _vulkan->getSwapChainExtent().width,
        _vulkan->getSwapChainExtent().height, 1,
        VK_SAMPLE_COUNT_1_BIT,
        _vulkan->PLP_VK_FORMAT_COLOR,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        image2);

      _images2[i] = std::move(image2);

      _imageviews2[i] = _vulkan->createImageView(
        _images2[i],
        _vulkan->PLP_VK_FORMAT_COLOR,
        VK_IMAGE_ASPECT_COLOR_BIT, 4);

      _samplers2[i] = _vulkan->createTextureSampler(1);

      VkImage depth_image;

      _vulkan->createImage(
        _vulkan->getSwapChainExtent().width,
        _vulkan->getSwapChainExtent().height, 1,
        VK_SAMPLE_COUNT_1_BIT,
        _vulkan->PLP_VK_FORMAT_DEPTH,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_SAMPLED_BIT
        | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
        | VK_IMAGE_USAGE_TRANSFER_DST_BIT
        | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depth_image);

      VkImageView depth_imageview = _vulkan->createImageView(
        depth_image,
        _vulkan->PLP_VK_FORMAT_DEPTH,
        1,
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
        _vulkan->PLP_VK_FORMAT_DEPTH,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_SAMPLED_BIT
        | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
        | VK_IMAGE_USAGE_TRANSFER_DST_BIT
        | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depth_image2);

      VkImageView depth_imageview2 = _vulkan->createImageView(
        depth_image2,
        _vulkan->PLP_VK_FORMAT_DEPTH,
        1,
        1,
        VK_IMAGE_ASPECT_DEPTH_BIT);

      _depth_images2[i] = std::move(depth_image2);
      _depth_imageviews2[i] = std::move(depth_imageview2);
      _depth_samplers2[i] = std::move(_vulkan->createTextureSampler(1));
    }

    _cmd_pool_entities = _vulkan->createCommandPool();
    _cmd_pool_entities2 = _vulkan->createCommandPool();
    _cmd_pool_entities3 = _vulkan->createCommandPool();
    _cmd_pool_entities4 = _vulkan->createCommandPool();

    _cmd_buffer_entities = _vulkan->allocateCommandBuffers(_cmd_pool_entities,
      static_cast<uint32_t>(_imageviews.size()));

    _cmd_buffer_entities2 = _vulkan->allocateCommandBuffers(_cmd_pool_entities2,
      static_cast<uint32_t>(_imageviews.size()));

    _cmd_buffer_entities3 = _vulkan->allocateCommandBuffers(_cmd_pool_entities3,
      static_cast<uint32_t>(_imageviews.size()));

    _cmd_buffer_entities4 = _vulkan->allocateCommandBuffers(_cmd_pool_entities4,
      static_cast<uint32_t>(_imageviews.size()));

    _cmd_pool_shadowmap = _vulkan->createCommandPool();
    _cmd_buffer_shadowmap = _vulkan->allocateCommandBuffers(_cmd_pool_shadowmap,
      static_cast<uint32_t>(_imageviews.size()));

    for (std::size_t i { 0 }; i < buffer_size; ++i) {
      VkImage image{};
      _vulkan->createDepthMapImage(image, true);
      _depthmap_images.emplace_back(image);
      _depthmap_imageviews.emplace_back(_vulkan->createDepthMapImageView(image, true));
      _depthmap_imageviews_rendering.emplace_back(_vulkan->createDepthMapImageView(image, true, false));

      _depthmap_samplers.emplace_back(_vulkan->createDepthMapSampler());
    }
    for (std::size_t i { 0 }; i < buffer_size; ++i) {
      VkImage image{};
      _vulkan->createDepthMapImage(image, false, 4);
      _csm_images.emplace_back(image);
      _csm_imageviews.emplace_back(_vulkan->createDepthMapImageView(image, false, false, 4));
      _csm_imageviews_rendering.emplace_back(_vulkan->createDepthMapImageView(image, false, false, 4));

      _csm_samplers.emplace_back(_vulkan->createDepthMapSampler());
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

    _fences_in_flight.resize(buffer_size);
    _images_in_flight.resize(buffer_size, VK_NULL_HANDLE);
    _timeline_semaphores.resize(buffer_size, VK_NULL_HANDLE);

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    _sema_render_completes.resize(buffer_size);
    _sema_present_completes.resize(buffer_size);
    _image_available.resize(buffer_size);
    _current_timeline_values.resize(buffer_size);

    VkSemaphoreTypeCreateInfo timeline_create_info{};
    timeline_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    timeline_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    timeline_create_info.initialValue = 0;

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = &timeline_create_info;

    for (std::size_t i { 0 }; i < buffer_size; ++i) {
      _sema_render_completes[i].resize(buffer_size);
      for (std::size_t x { 0 }; x < buffer_size; ++x) {
        result = vkCreateSemaphore(_vulkan->getDevice(), &sema_create_info, nullptr, &_sema_render_completes[i][x]);
        if (VK_SUCCESS != result) Logger::error("can't create _sema_render_completes semaphore");
      }
    }
    for (std::size_t i { 0 }; i < buffer_size; ++i) {
      result = vkCreateSemaphore(_vulkan->getDevice(), &sema_create_info, nullptr, &_sema_present_completes[i]);
      if (VK_SUCCESS != result) Logger::error("can't create _sema_present_completes semaphore");
    }
    for (std::size_t i { 0 }; i < buffer_size; ++i) {
      result = vkCreateSemaphore(_vulkan->getDevice(), &sema_create_info, nullptr, &_image_available[i]);
      if (VK_SUCCESS != result) Logger::error("can't create _image_available semaphore");

      result = vkCreateFence(_vulkan->getDevice(), &fence_info, nullptr, &_images_in_flight[i]);
      if (VK_SUCCESS != result) Logger::error("can't create _images_in_flight fence");

      result = vkCreateFence(_vulkan->getDevice(), &fence_info, nullptr, &_fences_in_flight[i]);
      if (VK_SUCCESS != result) Logger::error("can't create _fences_in_flight fence");

      result = vkCreateSemaphore(_vulkan->getDevice(), &semaphore_create_info, nullptr, &_timeline_semaphores[i]);
      if (VK_SUCCESS != result) Logger::error("can't create _fences_in_flight fence");
    }
  }

  void Renderer::setPerspective()
  {
    _perspective = glm::perspective(
      glm::radians(45.0f),
      static_cast<float>(_vulkan->getSwapChainExtent().width) / static_cast<float>(_vulkan->getSwapChainExtent().height),
      0.1f, 1000.f);
      //_perspective[1][1] *= -1;
  }

  void Renderer::setDeltatime(float delta_time)
  {
    _delta_time = delta_time;
  }

  void Renderer::start()
  {
    vkWaitForFences(_vulkan->getDevice(), 1, &_fences_in_flight[_current_frame], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(_vulkan->getDevice(), _swapchain, UINT64_MAX, _image_available[_current_frame], VK_NULL_HANDLE, &_image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(_vulkan->getDevice(), 1, &_fences_in_flight[_current_frame]);

    //clearScreen();
  }

  void Renderer::startRender()
  {
    auto& cmd_buffer = _cmd_buffer_entities[_current_frame];
    auto& colorview = _imageviews[_current_frame];
    auto& color = _images[_current_frame];
    auto& depthview = _depth_imageviews[_current_frame];
    auto& depthimage = _depth_images[_current_frame];
    auto const load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
    auto const store_op = VK_ATTACHMENT_STORE_OP_STORE;
    uint32_t const thread_id {1};
    auto const has_depth_attachment {true};

    _vulkan->beginCommandBuffer(cmd_buffer);

    VkImageLayout const undefined_layout{ VK_IMAGE_LAYOUT_UNDEFINED };
    VkImageLayout const begin_color_layout{ VK_IMAGE_LAYOUT_GENERAL };
    VkImageLayout const begin_depth_layout{ VK_IMAGE_LAYOUT_GENERAL };
    //VkImageLayout const general { VK_IMAGE_LAYOUT_GENERAL };
    VkImageAspectFlagBits const color_aspect { VK_IMAGE_ASPECT_COLOR_BIT };
    VkImageAspectFlagBits const depth_aspect{ VK_IMAGE_ASPECT_DEPTH_BIT };

    _vulkan->transitionImageLayout(cmd_buffer, color, undefined_layout, begin_color_layout, color_aspect);
    _vulkan->transitionImageLayout(cmd_buffer, depthimage, undefined_layout, begin_depth_layout, depth_aspect);

    _vulkan->beginRendering(
      cmd_buffer,
      colorview,
      depthview,
      load_op,
      store_op,
      begin_color_layout,
      has_depth_attachment);

    _vulkan->setViewPort(cmd_buffer);
    _vulkan->setScissor(cmd_buffer);

    float const marker_color_r {static_cast<float>(std::rand() % 255) / 255.0f};
    float const marker_color_g { static_cast<float>(std::rand() % 255) / 255.0f };
    float const marker_color_b { static_cast<float>(std::rand() % 255) / 255.0f };

    _vulkan->startMarker(
      cmd_buffer,
      "drawing_" + std::to_string(thread_id),
      marker_color_r, marker_color_g, marker_color_b);

    std::vector<VkBool32> blend_enable{ VK_FALSE };
    vkCmdSetColorBlendEnableEXT(cmd_buffer, 0, 1, blend_enable.data());
    VkColorBlendEquationEXT colorBlendEquation{};

    colorBlendEquation.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendEquation.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendEquation.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendEquation.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendEquation.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendEquation.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendEquation.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

    vkCmdSetColorBlendEquationEXT(cmd_buffer, 0, 1, &colorBlendEquation);
  }

  void Renderer::draw(RendererInfo const& renderer_info)
  {
    auto& cmd_buffer = _cmd_buffer_entities[_current_frame];
    auto const& mesh = renderer_info.mesh;
    auto const& camera = renderer_info.camera;
    auto const pipeline = getPipeline(mesh->getShaderName());

    constants push_constants{};
    push_constants.options = mesh->getOptions();
    push_constants.view_position = camera->getPos();
    push_constants.view = renderer_info.camera_view;

    if ("skybox" == mesh->getName()) {
      push_constants.view = glm::mat4(glm::mat3(camera->lookAt()));
    }

    vkCmdPushConstants(
      cmd_buffer,
      pipeline->pipeline_layout,
      renderer_info.stage_flag_bits,
      0,
      sizeof(constants),
      &push_constants);

    auto const alpha_mode{ mesh->getMaterial().alpha_mode };

    if (renderer_info.has_alpha_blend && alpha_mode > 0.0f) {
      std::vector<VkBool32> blend_enable{ VK_TRUE };
      vkCmdSetColorBlendEnableEXT(cmd_buffer, 0, 1, blend_enable.data());
    }

    vkCmdBindDescriptorSets(
      cmd_buffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipeline->pipeline_layout,
      0, 1, mesh->getDescSet(), 0, nullptr);

    if (mesh->getMaterial().double_sided == true && pipeline->pipeline_bis != VK_NULL_HANDLE) {
      _vulkan->bindPipeline(cmd_buffer, pipeline->pipeline_bis);
      } else {
      _vulkan->bindPipeline(cmd_buffer, pipeline->pipeline);
    }

    _vulkan->draw(
      cmd_buffer,
      mesh->getData(),
      mesh->is_indexed());

    if (mesh->debugNormal() &&  renderer_info.normal_debug) {
      auto const normal_pipeline = getPipeline("normal_debug");

      vkCmdBindDescriptorSets(
        cmd_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        normal_pipeline->pipeline_layout,
        0, 1, mesh->getDescSet(), 0, nullptr);

      _vulkan->bindPipeline(cmd_buffer, normal_pipeline->pipeline);

      _vulkan->draw(
        cmd_buffer,
        mesh->getData(),
        mesh->is_indexed());
    }
  }

  void Renderer::startShadowMap(SHADOW_TYPE const shadow_type)
  {
    VkCommandBuffer cmd_buffer;
    VkImage depth;
    VkImageView depthview;
    std::string marker_name {"shadow_map"};
    glm::vec3 marker_color { 0.1f, 0.2f, 0.3f };

    if (shadow_type == SHADOW_TYPE::CSM) {
      cmd_buffer = _cmd_buffer_shadowmap[_current_frame];
      depth = _csm_images[_current_frame];
      depthview = _csm_imageviews[_current_frame];
      marker_name = "csm";
    } else {
      cmd_buffer = _cmd_buffer_entities3[_current_frame];
      depth = _depthmap_images[_current_frame];
      depthview = _depthmap_imageviews[_current_frame];
      marker_color = {0.4, 0.2, 0.6};
    }

    uint32_t const layer_count = (shadow_type == SHADOW_TYPE::CSM) ? 4 : 6;

    _vulkan->beginCommandBuffer(cmd_buffer);
    _vulkan->startMarker(cmd_buffer, marker_name, marker_color.x, marker_color.y, marker_color.z);

    _vulkan->transitionImageLayout(
      cmd_buffer,
      depth,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_GENERAL,
      VK_IMAGE_ASPECT_DEPTH_BIT,
      layer_count);

    VkClearColorValue color_clear = {};
    color_clear.float32[0] = 1.0f;
    color_clear.float32[1] = 1.0f;
    color_clear.float32[2] = 1.0f;
    color_clear.float32[3] = 1.0f;

    VkClearDepthStencilValue depth_stencil = { 1.f, 0 };

    VkRenderingAttachmentInfo depth_attachment_info{ };
    depth_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depth_attachment_info.imageView = depthview;
    depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment_info.clearValue.depthStencil = depth_stencil;
    depth_attachment_info.clearValue.color = color_clear;

    auto const& appConfig { ConfigManagerLocator::get()->appConfig()["shadow_resolution"] };
    auto const width { appConfig["width"].get<uint32_t>() };

    VkRenderingInfo  rendering_info{ };
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.renderArea.extent.width = width;
    rendering_info.renderArea.extent.height = width;
    rendering_info.layerCount = (shadow_type == SHADOW_TYPE::CSM) ? 4 : 6;
    rendering_info.pDepthAttachment = &depth_attachment_info;
    rendering_info.colorAttachmentCount = 0;
    //rendering_info.flags = VK_SUBPASS_CONTENTS_INLINE;

    vkCmdBeginRenderingKHR(cmd_buffer, &rendering_info);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(width);
    viewport.width = static_cast<float>(width);
    viewport.height = -static_cast<float>(width);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
    VkRect2D scissor = { { 0, 0 }, { width, width } };

    vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

    float const depth_bias_constant{ 1.25f };
    float const depth_bias_slope{ 1.75f };
    float const depth_bias_clamp{ 0.0f };

    //vkCmdSetDepthClampEnableEXT(cmd_buffer, VK_TRUE);
    vkCmdSetDepthBias(cmd_buffer, depth_bias_constant, depth_bias_clamp, depth_bias_slope);

    std::vector<VkBool32> blend_enable{ VK_FALSE };
    vkCmdSetColorBlendEnableEXT(cmd_buffer, 0, 1, blend_enable.data());

    VkColorBlendEquationEXT colorBlendEquation{};
    colorBlendEquation.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendEquation.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendEquation.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendEquation.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendEquation.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendEquation.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendEquation.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

    vkCmdSetColorBlendEquationEXT(cmd_buffer, 0, 1, &colorBlendEquation);
  }

  void Renderer::drawShadowMap(
    RendererInfo const& renderer_info,
    SHADOW_TYPE const shadow_type,
    VkBuffer const&)
  {
    VkCommandBuffer cmd_buffer;
    std::string pipeline_name {"shadow_map"};

    if (shadow_type == SHADOW_TYPE::CSM) {
      cmd_buffer = _cmd_buffer_shadowmap[_current_frame];
      pipeline_name = "csm";
    } else {
      cmd_buffer = _cmd_buffer_entities3[_current_frame];
    }
    auto const& pipeline = getPipeline(pipeline_name);

    auto const& mesh = renderer_info.mesh;
    auto const& camera = renderer_info.camera;

    glm::vec4 options{0.0f};
    options.x = 50.f;

    constants push_constants{};
    push_constants.options = options;
    push_constants.view_position = camera->getPos();
    push_constants.view = renderer_info.camera_view;

    vkCmdPushConstants(
      cmd_buffer,
      pipeline->pipeline_layout,
      VK_SHADER_STAGE_VERTEX_BIT
      | VK_SHADER_STAGE_FRAGMENT_BIT,
      0,
      sizeof(constants),
      &push_constants);

    if (shadow_type == SHADOW_TYPE::CSM) {
      vkCmdBindDescriptorSets(
        cmd_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline->pipeline_layout,
        0, 1, mesh->getCSMDescSet(), 0, nullptr);
    } else {
      vkCmdBindDescriptorSets(
        cmd_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline->pipeline_layout,
        0, 1, mesh->getShadowMapDescSet(), 0, nullptr);
    }
    _vulkan->bindPipeline(cmd_buffer, pipeline->pipeline);

    _vulkan->draw(
      cmd_buffer,
      mesh->getData(),
      mesh->getData()->_ubos.size(),
      mesh->is_indexed());
  }

  void Renderer::endShadowMap(SHADOW_TYPE const shadow_type)
  {
    VkCommandBuffer cmd_buffer;
    //VkImage depth;
    uint32_t thread_id {0};

    if (shadow_type == SHADOW_TYPE::CSM) {
      cmd_buffer = _cmd_buffer_shadowmap[_current_frame];
      //depth = _csm_images[_current_frame];
      thread_id = 1;
    } else {
      cmd_buffer = _cmd_buffer_entities3[_current_frame];
      //depth = _depthmap_images[_current_frame];
    }

    _vulkan->endMarker(cmd_buffer);
    _vulkan->endRendering(cmd_buffer);

    //uint32_t const layer_count = (shadow_type == SHADOW_TYPE::CSM) ? 4 : 6;

    //  _vulkan->transitionImageLayout(cmd_buffer,
    //   depth,
    //    VK_IMAGE_LAYOUT_GENERAL,
    //    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    //    VK_IMAGE_ASPECT_DEPTH_BIT,
    //   layer_count);
 /*     _vulkan->transitionImageLayout(cmd_buffer, depth,
        VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_DEPTH_BIT);*/

    _vulkan->endCommandBuffer(cmd_buffer);

    std::vector<VkPipelineStageFlags> flags { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT };
    //VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
    _draw_cmds.insert(cmd_buffer, thread_id, false, flags);

    _update_shadow_map = false;
  }

  void Renderer::endRender()
  {
    auto& cmd_buffer = _cmd_buffer_entities[_current_frame];
    auto& color = _images[_current_frame];
    auto& depthimage = _depth_images[_current_frame];
    uint32_t const thread_id {2};
    auto const is_attachment {false};
    auto const has_depth_attachment {true};

    _vulkan->endMarker(cmd_buffer);
    endRendering(cmd_buffer, color, depthimage, is_attachment, has_depth_attachment);

    std::vector<VkPipelineStageFlags> flags { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    _draw_cmds.insert(cmd_buffer, thread_id, is_attachment, flags);
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
    int)
  {
    auto command_pool = _vulkan->createCommandPool();
    VkCommandBuffer cmd = _vulkan->allocateCommandBuffers(command_pool)[0];
    _vulkan->beginCommandBuffer(cmd);
    function(cmd);
    _vulkan->endCommandBuffer(cmd);
    _vulkan->queueSubmit(cmd);
    vkDestroyCommandPool(_vulkan->getDevice(), command_pool, nullptr);
  }

  void Renderer::endRendering(
    VkCommandBuffer& cmd_buffer,
    VkImage& image,
    VkImage&,
    bool const,
    bool const)
  {
    _vulkan->endRendering(cmd_buffer);

    // VkImageLayout final = (is_attachment) ? VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL: VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    _vulkan->transitionImageLayout(cmd_buffer, image,
      VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT);

    // if (has_depth_attachment) {
    //  _vulkan->transitionImageLayout(cmd_buffer, depth_image,
    //    VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    // }

    _vulkan->endCommandBuffer(cmd_buffer);
  }

  void Renderer::submit()
  {
    if (_sema_render_completes[_current_frame].size() - 1 < _image_index) {
      return;
    }
    auto const& draw_cmds { _draw_cmds };

    std::vector<VkCommandBuffer> cmds_buffer{};

    for (std::size_t i{ 0 }; i < draw_cmds.cmd_buffers.size(); ++i) {
      if (draw_cmds.cmd_buffers.at(i) == nullptr) continue;
      cmds_buffer.push_back(draw_cmds.cmd_buffers.at(i));
    }

    if (cmds_buffer.empty()) {
      vkResetFences(_vulkan->getDevice(), 1, &_fences_in_flight[_image_index]);
      _current_frame = (_current_frame + 1) % _max_frames_in_flight;
      return;
    }

    //auto &semaphore_present_complete = _sema_present_completes[_current_frame];
    auto &semaphore_render_complete = _sema_render_completes[_current_frame][_image_index];
    auto &timeline_semaphore = _timeline_semaphores[_current_frame];

    auto &_current_timeline_value = _current_timeline_values[_current_frame];
    std::uint64_t const draw_finished = _current_timeline_value + 1;
    std::uint64_t const finished = _current_timeline_value + 3;
    std::array<std::uint64_t, 1> wait_values = { 0 };
    std::array<std::uint64_t, 2> signal_values = { draw_finished, 0 };

    //VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    std::array<VkPipelineStageFlags, 2> graphics_wait_stage_masks = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    std::array<VkSemaphore, 1> graphics_wait_semaphores = { _image_available[_current_frame] };
    std::array<VkSemaphore, 2> graphics_signal_semaphores = { timeline_semaphore, semaphore_render_complete };

    VkTimelineSemaphoreSubmitInfoKHR timeline_submit_info{};
    timeline_submit_info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO_KHR;
    timeline_submit_info.waitSemaphoreValueCount = 1;
    timeline_submit_info.pWaitSemaphoreValues = wait_values.data();
    timeline_submit_info.signalSemaphoreValueCount = 2;
    timeline_submit_info.pSignalSemaphoreValues = signal_values.data();

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = static_cast<uint32_t>(cmds_buffer.size());
    submit_info.pCommandBuffers = cmds_buffer.data();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = graphics_wait_semaphores.data();
    submit_info.pWaitDstStageMask = graphics_wait_stage_masks.data();
    submit_info.signalSemaphoreCount = 2;
    submit_info.pSignalSemaphores = graphics_signal_semaphores.data();
    submit_info.pNext = &timeline_submit_info;

    auto queue = _vulkan->getGraphicsQueues().at(0);

    std::vector<VkSwapchainKHR> swapchains{ _swapchain };

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &semaphore_render_complete;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains.data();
    present_info.pImageIndices = &_image_index;
    present_info.pResults = nullptr;

    _vulkan->submit(queue, { submit_info }, present_info, _fences_in_flight[_current_frame]);

    _current_timeline_value = finished;

    _previous_frame = _current_frame;
    _current_frame = (_current_frame + 1) % _max_frames_in_flight;
    _draw_cmds.clear();
    onFinishRender();
  }

  void Renderer::setRayPick(
    float const,
    float const,
    float const,
    int const,
    int const)
  {
    // glm::vec3 rayNds = glm::vec3(x, y, z);
    // glm::vec4 rayClip = glm::vec4(rayNds.x, rayNds.y, -1.0, 1.0);
    // glm::vec4 rayEye = glm::inverse(getPerspective()) * rayClip;
    // rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);

    // glm::vec4 tmp = (glm::inverse(getCamera()->getView()) * rayEye);
    // glm::vec3 rayWor = glm::vec3(tmp.x, tmp.y, tmp.z);
  }

  void Renderer::clear()
  {
    //_entity_manager->clear();
    //@todo clean memory
    //_vulkan->getDeviceMemoryPool()->clear();
  }

  void Renderer::onFinishRender()
  {
    //Event event{ "OnFinishRender" };
    //for (const auto& observer : _Observers) {
    //    observer->notify(event);
    //}
  }

  void Renderer::addPipeline(
    std::string const& shaderName,
    VulkanPipeline& pipeline)
  {
    _pipelines[shaderName] = std::move(pipeline);
  }

  void Renderer::clearScreen()
  {
    VkClearColorValue clear_color = {};

    clear_color.float32[0] = 0.2f;
    clear_color.float32[1] = 0.4f;
    clear_color.float32[2] = 0.3f;
    clear_color.float32[3] = 0.0f;

    VkClearDepthStencilValue depth_stencil_clear_color{ 1.f, 0 };

    VkImageSubresourceRange image_range = {};
    image_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_range.levelCount = 1;
    image_range.layerCount = 1;

    VkImageSubresourceRange depth_image_range = {};
    depth_image_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depth_image_range.levelCount = 1;
    depth_image_range.layerCount = 1;

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    vkBeginCommandBuffer(_cmd_buffer_entities[_current_frame], &begin_info);
    vkCmdClearColorImage(_cmd_buffer_entities[_current_frame], _images[_current_frame], VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &image_range);
    vkCmdClearDepthStencilImage(_cmd_buffer_entities[_current_frame], _depth_images[_current_frame], VK_IMAGE_LAYOUT_GENERAL, &depth_stencil_clear_color, 1, &depth_image_range);
    vkEndCommandBuffer(_cmd_buffer_entities[_current_frame]);

    vkBeginCommandBuffer(_cmd_buffer_entities2[_current_frame], &begin_info);
    vkCmdClearColorImage(_cmd_buffer_entities2[_current_frame], _images[_current_frame], VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &image_range);
    vkCmdClearDepthStencilImage(_cmd_buffer_entities2[_current_frame], _depth_images2[_current_frame], VK_IMAGE_LAYOUT_GENERAL, &depth_stencil_clear_color, 1, &depth_image_range);
    vkEndCommandBuffer(_cmd_buffer_entities2[_current_frame]);

    vkBeginCommandBuffer(_cmd_buffer_entities3[_current_frame], &begin_info);
    vkCmdClearColorImage(_cmd_buffer_entities3[_current_frame], _images[_current_frame], VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &image_range);
    vkEndCommandBuffer(_cmd_buffer_entities3[_current_frame]);

    vkBeginCommandBuffer(_cmd_buffer_entities4[_current_frame], &begin_info);
    vkCmdClearColorImage(_cmd_buffer_entities4[_current_frame], _images[_current_frame], VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &image_range);
    vkEndCommandBuffer(_cmd_buffer_entities4[_current_frame]);
  }
}
