module;
#include <volk.h>

#include <concepts>
#include <memory>
#include <unordered_map>
#include <vector>

module Engine.Managers.DestroyManager;

import Engine.Component.Entity;
import Engine.Component.Texture;
import Engine.Renderer;
namespace Poulpe
{
  void DestroyManager::cleanDeviceMemory()
  {
      _device_memory_pool->clear();
  }

  template<std::derived_from<Entity> T>
  void DestroyManager::cleanEntity( T* const)
  {
      /*for (auto buffer : *entity->getMesh()->getUniformBuffers()) {
          _renderer->destroyBuffer(buffer.buffer);
      }

      _renderer->destroyBuffer(entity->getMesh()->getData()->_vertex_buffer.buffer);
      _renderer->destroyBuffer(entity->getMesh()->getData()->_indices_buffer.buffer);*/
  }

  template<std::derived_from<Entity> T>
  void DestroyManager::cleanEntities(std::vector<std::unique_ptr<T>> & entities)
  {
      for (auto & entity : entities) {
          cleanEntity(entity.get());
      }
  }

  template<std::derived_from<Entity> T>
  void DestroyManager::cleanEntities(std::vector<T*> & entities)
  {
      for (auto entity : entities) {
          cleanEntity(entity);
      }
  }

  void DestroyManager::setRenderer(Renderer* const renderer)
  {
      _renderer = renderer;
  }

  void DestroyManager::cleanShaders(std::unordered_map<std::string, std::unordered_map<std::string, VkShaderModule>> shaders)
  {
      for (auto &[name, shader] : shaders) {
          vkDestroyShaderModule(_renderer->getDevice(), shader["vert"], nullptr);
          vkDestroyShaderModule(_renderer->getDevice(), shader["frag"], nullptr);
      }
  }

  void  DestroyManager::cleanTexture(Texture& textures)
  {
      vkDestroySampler(_renderer->getDevice(), textures.getSampler(), nullptr);
      vkDestroyImage(_renderer->getDevice(), textures.getImage(), nullptr);
      vkDestroyImageView(_renderer->getDevice(), textures.getImageView(), nullptr);
  }

  void DestroyManager::cleanTextures(std::unordered_map<std::string, Texture>& textures)
  {
      for (auto& texture : textures) {
          cleanTexture(texture.second);
      }
  }

  template void DestroyManager::cleanEntities<Entity>(std::vector<Entity*> & entities);
  template void DestroyManager::cleanEntities<Entity>(std::vector<std::unique_ptr<Entity>> & entities);
}
