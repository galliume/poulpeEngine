#include "ShadowMap.hpp"

namespace Poulpe
{
    struct constants;

    void ShadowMap::createDescriptorSet(Mesh* mesh)
    {
      Texture const tex{ _texture_manager->getTextures()[mesh->getData()->_textures.at(0)] };

      std::vector<VkDescriptorImageInfo> imageInfos{};
      imageInfos.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      auto const pipeline = _renderer->getPipeline(mesh->getShaderName());
      VkDescriptorSet descSet = _renderer->createDescriptorSets(pipeline->descPool, { pipeline->descSetLayout }, 1);

      for (size_t i{ 0 }; i < mesh->getUniformBuffers()->size(); ++i) {

        _renderer->updateDescriptorSets(
          *mesh->getUniformBuffers(),
          *mesh->getStorageBuffers(),
          descSet, imageInfos);
      }

      mesh->setDescSet(descSet);
    }

    void ShadowMap::setPushConstants(Mesh* mesh)
    {
        mesh->setApplyPushConstants([](
            VkCommandBuffer & cmd_buffer, 
            VkPipelineLayout pipelineLayout,
            Renderer* const renderer, Mesh* const meshS) {

            constants pushConstants{};
            pushConstants.view = renderer->getCamera()->lookAt();
            pushConstants.viewPos = renderer->getCamera()->getPos();

            vkCmdPushConstants(cmd_buffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants),
                & pushConstants);
        });

        mesh->setHasPushConstants();
    }

    void ShadowMap::operator()(std::chrono::duration<float> const& deltaTime, Mesh* mesh)
    {
      if (!mesh && !mesh->isDirty()) return;

      uint32_t const totalInstances{ static_cast<uint32_t>(mesh->getData()->_ubos.size()) };
      uint32_t const maxUniformBufferRange{ _renderer->getDeviceProperties().limits.maxUniformBufferRange };
      unsigned long long const uniformBufferChunkSize{ maxUniformBufferRange / sizeof(UniformBufferObject) };
      uint32_t const uniformBuffersCount{ static_cast<uint32_t>(std::ceil(static_cast<float>(totalInstances) / static_cast<float>(uniformBufferChunkSize))) };

      //@todo fix memory management...
      unsigned long long uboOffset{ (totalInstances > uniformBufferChunkSize) ? uniformBufferChunkSize : totalInstances };
      unsigned long long uboRemaining { (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0};
      unsigned long long nbUbo { uboOffset};

      auto commandPool = _renderer->createCommandPool();

      for (size_t i{ 0 }; i < uniformBuffersCount; ++i) {

        mesh->getData()->_ubos_offset.emplace_back(uboOffset);
        Buffer uniformBuffer = _renderer->createUniformBuffers(nbUbo, commandPool);
        mesh->getUniformBuffers()->emplace_back(uniformBuffer);

        uboOffset = (uboRemaining > uniformBufferChunkSize) ? uboOffset + uniformBufferChunkSize : uboOffset + uboRemaining;
        nbUbo = (uboRemaining > uniformBufferChunkSize) ? uniformBufferChunkSize : uboRemaining;
        uboRemaining = (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0;
      }

      auto const& data = mesh->getData();

      data->_vertex_buffer = _renderer->createVertexBuffer(commandPool, data->_vertices);
      data->_indices_buffer = _renderer->createIndexBuffer(commandPool, data->_Indices);
      data->_texture_index = 0;

      vkDestroyCommandPool(_renderer->getDevice(), commandPool, nullptr);

      for (size_t i{ 0 }; i < mesh->getData()->_ubos.size(); ++i) {
        mesh->getData()->_ubos[i].projection = _renderer->getPerspective();

        if (_texture_manager->getTextures().contains(mesh->getData()->_bump_map)) {
          auto const tex = _texture_manager->getTextures()[mesh->getData()->_bump_map];
          mesh->getData()->_ubos[i].texSize = glm::vec2(tex.getWidth(), tex.getHeight());
        }
      }

      // Material material{};
      // material.ambient = mesh->getMaterial().ambient;
      // material.diffuse = mesh->getMaterial().diffuse;
      // material.specular = mesh->getMaterial().specular;
      // material.transmittance = mesh->getMaterial().transmittance;
      // material.emission = mesh->getMaterial().emission;
      // material.shiIorDiss = glm::vec3(mesh->getMaterial().shininess,
      //   mesh->getMaterial().ior, mesh->getMaterial().illum);

      // ObjectBuffer objectBuffer{};
      // objectBuffer.pointLights[0] = _light_manager->getPointLights().at(0);
      // objectBuffer.pointLights[1] = _light_manager->getPointLights().at(1);

      // objectBuffer.spotLight = _light_manager->getSpotLights().at(0);
      // objectBuffer.ambientLight = _light_manager->getAmbientLight();
      // objectBuffer.material = material;

      // auto const size = sizeof(objectBuffer);
      // auto storageBuffer = _renderer->createStorageBuffers(size);
      // mesh->addStorageBuffer(storageBuffer);
      // _renderer->updateStorageBuffer(mesh->getStorageBuffers()->at(0), objectBuffer);
      // mesh->setHasBufferStorage();

      unsigned int min{ 0 };
      unsigned int max{ 0 };

      for (size_t i{ 0 }; i < mesh->getUniformBuffers()->size(); ++i) {
        max = mesh->getData()->_ubos_offset.at(i);
        auto ubos = std::vector<UniformBufferObject>(mesh->getData()->_ubos.begin() + min, mesh->getData()->_ubos.begin() + max);

        _renderer->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &ubos);

        min = max;
      }

      createDescriptorSet(mesh);
      setPushConstants(mesh);
      mesh->setIsDirty(false);
    }
}
