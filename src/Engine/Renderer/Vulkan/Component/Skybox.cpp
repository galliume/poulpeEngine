module;


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

#include <volk.h>

module Engine.Renderer.Vulkan.Skybox;

import std;

import Engine.Component.Components;
import Engine.Component.Texture;
import Engine.Component.Vertex;

import Engine.Core.Logger;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;

import Engine.Renderer.RendererComponentTypes;

namespace Poulpe
{
  void Skybox::operator()(
    Renderer *const renderer,
    ComponentRenderingInfo const& component_rendering_info)
  {
    stage_flag_bits = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    auto const& mesh = component_rendering_info.mesh;

    if (!mesh && !mesh->isDirty()) return;


    std::vector<Vertex> const vertices {
      {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {}},
      {{-1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {}},
      {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {}},
      {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {}},
      {{ 1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {}},
      {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {}},

      {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{-1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{-1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },

      {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{ 1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{ 1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },

      {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{-1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{ 1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },

      {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{ 1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{-1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },

      {{-1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} },
      {{ 1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {} }
    };

    auto cmd_pool = renderer->getAPI()->createCommandPool();

    std::vector<UniformBufferObject> ubos{};
    ubos.reserve(1);

    if (component_rendering_info.mode == ComponentRenderingInfo::MODE::CREATION) {
      UniformBufferObject ubo{};
      ubo.model = glm::mat4(0.0f);
      //ubo.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
      ubo.projection = renderer->getPerspective();
      ubos.push_back({ ubo });

      Buffer uniform_buffer = renderer->getAPI()->createUniformBuffers(1);
      mesh->getUniformBuffers().emplace_back(uniform_buffer);
    } else {
      auto const& mesh_data = mesh->getData();
      ubos = mesh_data->_ubos[0];
    }

    Data data{};
    data._textures.emplace_back("skybox");
    data._vertices = vertices;
    data._vertex_buffer = renderer->getAPI()->createVertexBuffer(vertices, renderer->getCurrentFrameIndex());
    data._ubos.resize(1);
    data._ubos[0] = ubos;
    data._texture_index = 0;

    vkDestroyCommandPool(renderer->getDevice(), cmd_pool, nullptr);

    mesh->setName("skybox");
    mesh->setShaderName("skybox");
    mesh->setIsIndexed(false);
    mesh->setData(data);
    mesh->setIsDirty(false);
    mesh->setHasShadow(false);

    renderer->getAPI()->updateUniformBuffer(mesh->getUniformBuffers().at(0), &data._ubos.at(0));

    createDescriptorSet(renderer, component_rendering_info);
  }

  void Skybox::createDescriptorSet(
    Renderer *const renderer,
    ComponentRenderingInfo const& component_rendering_info)
  {
    auto const& mesh = component_rendering_info.mesh;

    Texture tex{ component_rendering_info.textures->at(component_rendering_info.skybox_name) };
    tex.setSampler(renderer->getAPI()->createKTXSampler(
    TextureWrapMode::CLAMP_TO_EDGE,
    TextureWrapMode::CLAMP_TO_EDGE,
    tex.getMipLevels()));

    std::vector<VkDescriptorImageInfo> image_infos{};
    image_infos.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_GENERAL);

    auto const pipeline = renderer->getPipeline(mesh->getShaderName());
    VkDescriptorSet descset = renderer->getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1);

    renderer->getAPI()->updateDescriptorSets(mesh->getUniformBuffers(), descset, image_infos);

    mesh->setDescSet(descset);
  }
}
