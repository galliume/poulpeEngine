module;

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <string_view>
#include <vector>
#include <volk.h>

module Engine.Renderer.Vulkan.Crosshair;

import Engine.Component.Components;
import Engine.Renderer.RendererComponentTypes;

namespace Poulpe
{
  void Crosshair::createDescriptorSet(
    Renderer *const,
    ComponentRenderingInfo const&)
  {
    //Texture const tex{ _texture_manager->getTextures()["crosshair_1"] };
    //Texture const tex2{ _texture_manager->getTextures()["crosshair_2"] };

    //std::vector<VkDescriptorImageInfo> imageInfos{};
    //imageInfos.reserve(2);
    //imageInfos.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    //imageInfos.emplace_back(tex2.getSampler(), tex2.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    //auto const& pipeline = _renderer->getPipeline(mesh->getShaderName());

    //VkDescriptorSet descset = _renderer->getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1);

    //_renderer->getAPI()->updateDescriptorSets(*mesh->getUniformBuffers(), descset, imageInfos);
    //mesh->setDescSet(descset);
  }

  void Crosshair::operator()(
    Renderer *const,
    ComponentRenderingInfo const&)
  {
    /* if (!mesh && !mesh->isDirty()) return;

    const std::vector<Vertex> vertices {
      {{-0.025f, -0.025f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
      {{0.025f, -0.025f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
      {{0.025f, 0.025f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
      {{-0.025f, 0.025f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} }
    };
    const std::vector<uint32_t> indices {
        0, 1, 2, 2, 3, 0
    };

    auto commandPool = _renderer->getAPI()->createCommandPool();

    UniformBufferObject ubo{};

    Data data;
    data._textures.emplace_back("crosshair");
    data._texture_index = 0;
    data._vertex_buffer = _renderer->getAPI()->createVertexBuffer(commandPool, vertices);
    data._indices_buffer = _renderer->getAPI()->createIndexBuffer(commandPool, indices);
    data._ubos.emplace_back(ubo);
    data._indices = indices;

    mesh->setName("crosshair");
    mesh->setShaderName("2d");
    mesh->getUniformBuffers().emplace_back(_renderer->getAPI()->createUniformBuffers(1, commandPool));

    vkDestroyCommandPool(_renderer->getDevice(), commandPool, nullptr);
    setPushConstants(mesh);

    for (uint32_t i{ 0 }; i < mesh->getUniformBuffers().size(); i++) {
      data._ubos[i].projection = _renderer->getPerspective();

      _renderer->getAPI()->updateUniformBuffer(mesh->getUniformBuffers().at(i), &data._ubos);
    }

    createDescriptorSet(mesh);
    mesh->setData(data);
    mesh->setIsDirty(false);*/
  }
}
