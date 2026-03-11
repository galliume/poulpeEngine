module Engine.Renderer.Vulkan.Skybox;

import std;

import Engine.Component.Mesh;
import Engine.Component.Texture;

import Engine.Core.Constants;
import Engine.Core.Logger;
import Engine.Core.MaterialTypes;
import Engine.Core.GLM;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;
import Engine.Core.Vertex;
import Engine.Core.Volk;

import Engine.Renderer.RendererComponentTypes;
import Engine.Renderer.VulkanRenderer;

namespace Poulpe
{
  void Skybox::operator()(
    Renderer & renderer,
    Mesh & mesh,
    RendererContext const& render_context)
  {
    stage_flag_bits = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    if (!mesh.isDirty()) return;
    std::vector<Vertex> const vertices {
        // 16-byte vec4s | 12-byte vec3s (Pos, Normal, Orig) | 8-byte UV | Bones/Weights/Total
        // Back face
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f, 1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 1.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f,-1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 1.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f,-1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 1.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f,-1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 1.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f, 1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 1.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f, 1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 1.0f}, {1.f,0.f} },

        // Left face
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f,-1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 1.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f,-1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 1.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f, 1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 1.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f, 1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 1.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f, 1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 1.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f,-1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 1.0f}, {1.f,0.f} },

        // Right face
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f,-1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f,-1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f, 1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f, 1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f, 1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f,-1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },

        // Front face, 0.0f, 0.0f
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f,-1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f, 1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f, 1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f, 1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f,-1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f,-1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },

        // Top face, 0.0f, 0.0f
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f, 1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f, 1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f, 1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f, 1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f, 1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f, 1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },

        // Bottom face, 0.0f, 0.0f
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f,-1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f,-1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f,-1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f,-1.f,-1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, {-1.f,-1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} },
        {{0.f,0.f,0.f,0.f}, {0.f,0.f,0.f,0.f}, { 1.f,-1.f, 1.f, 0.0f}, {1.f,0.f,0.f, 0.0f}, {1.f,0.f} }
    };
    auto cmd_pool = renderer.getAPI().createCommandPool();

    std::vector<UniformBufferObject> ubos{};
    ubos.reserve(1);

    if (render_context.mode == RendererContext::Mode::CREATION
      || mesh.getUniformBuffers().empty()) {
      UniformBufferObject ubo{};
      ubo.model = glm::mat4(0.0f);
      //ubo.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
      ubo.projection = renderer.getPerspective();
      ubos.push_back({ ubo });

      mesh.getUniformBuffers().emplace_back(
        renderer.getAPI().createUniformBuffers(1, renderer.getCurrentFrameIndex()));
    } else {
      auto const& mesh_data = mesh.getData();
      ubos = mesh_data->_ubos[0];
    }

    Data data{};
    data._textures.emplace_back("skybox");
    data._vertices = vertices;
    data._vertex_buffer = renderer.getAPI().createVertexBuffer(vertices, renderer.getCurrentFrameIndex());
    data._ubos.resize(1);
    data._ubos[0] = ubos;
    data._texture_index = 0;

    vkDestroyCommandPool(renderer.getDevice(), cmd_pool, nullptr);

    mesh.setName("skybox");
    mesh.setShaderName("skybox");
    mesh.setIsIndexed(false);
    mesh.setData(data);
    mesh.setIsDirty(false);
    mesh.setHasShadow(false);

    renderer.getAPI().updateUniformBuffer(mesh.getUniformBuffers().at(0), &data._ubos.at(0), renderer.getCurrentFrameIndex());

    createDescriptorSet(renderer, mesh, render_context);
  }

  void Skybox::createDescriptorSet(
    Renderer & renderer,
    Mesh & mesh,
    RendererContext const& render_context)
  {
    render_context.skybox->setSampler(renderer.getAPI().createKTXSampler(
    TextureWrapMode::CLAMP_TO_EDGE,
    TextureWrapMode::CLAMP_TO_EDGE,
    render_context.skybox->getMipLevels()));

    std::vector<VkDescriptorImageInfo> image_infos{};
    image_infos.emplace_back(render_context.skybox->getSampler(), render_context.skybox->getImageView(), VK_IMAGE_LAYOUT_GENERAL);

    VkDescriptorSet descset {
      renderer.getAPI().createDescriptorSets(renderer.getPipeline(mesh.getShaderName()), 1) };

    renderer.getAPI().updateDescriptorSets(mesh.getUniformBuffers(), descset, image_infos);

    mesh.setDescSet(descset);
  }
}
