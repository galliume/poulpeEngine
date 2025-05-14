export module Poulpe.Manager:DestroyManager;

import Poulpe.Component.Entity;
import Poulpe.Component.Mesh;

import Poulpe.Renderer.Vulkan.Renderer;

//@TODO refactor all destroy system...
class DeviceMemoryPool;

export class DestroyManager
{
public:
    DestroyManager() = default;

    inline void addMemoryPool(DeviceMemoryPool* const deviceMemoryPool) { _device_memory_pool = deviceMemoryPool; }
    void cleanDeviceMemory();

    template<std::derived_from<Entity> T>
    void cleanEntity(T* entity);

    template<std::derived_from<Entity> T>
    void cleanEntities(std::vector<std::unique_ptr<T>> & entities);

    template<std::derived_from<Entity> T>
    void cleanEntities(std::vector<T*> & entities);

    void cleanShaders(std::unordered_map<std::string, std::unordered_map<std::string, VkShaderModule>> shaders);
    void cleanTexture(Texture textures);
    void cleanTextures(std::unordered_map<std::string, Texture> textures);
    void setRenderer(Renderer* renderer);

private:
    DeviceMemoryPool* _device_memory_pool;
    Renderer* _renderer;
};
