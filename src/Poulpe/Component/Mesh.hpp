#pragma once

#include "Texture.hpp"
#include "Vertex.hpp"
#include "Vertex2D.hpp"

#include "Poulpe/Core/PlpTypedef.hpp"

#include "Poulpe/Renderer/Vulkan/DeviceMemory.hpp" //@todo should not be here

#include <atomic>
#include <functional>

namespace Poulpe
{
  class LightManager;
  class Renderer;
  class TextureManager;

  //@todo should not be IVisitor
  class Mesh
  {
  public:
    Data* getData() { return & m_Data; }
    VkDescriptorSet* getDescSet() { return & m_DescSet; }
    material_t const getMaterial() const { return m_Material; }
    std::string const getName() const { return m_Name; }
    inline const std::string getShaderName() const { return m_ShaderName; }
    std::vector<Buffer>* getStorageBuffers() { return & m_StorageBuffers; }
    inline std::vector<Buffer>* getUniformBuffers() { return & m_UniformBuffers; }
    ObjectBuffer* getObjectBuffer() { return &m_ObjectBuffer; };

    void addStorageBuffer(Buffer& buffer) { m_StorageBuffers.emplace_back(std::move(buffer)); }
    void init(
          Renderer* const renderer,
          TextureManager* const textureManager, 
          LightManager* const lightManager) {}
        
    bool isDirty() { return m_IsDirty.load(); }

    virtual void setApplyPushConstants(
      std::function<void(VkCommandBuffer& commandBuffer,
        VkPipelineLayout pipelineLayout,
        Renderer* const renderer,
        Mesh* const mesh)> fn) { m_ApplyPushConstants = fn; }

    void setData(Data data) { m_Data = std::move(data); }
    void setDescSet(VkDescriptorSet descSet) { m_DescSet = descSet; }
    void setShadowMapDescSet(VkDescriptorSet descSet) { m_ShadowMapDescSet = descSet; }
    void setHasBufferStorage(bool has = true) { m_HasStorageBuffer = has; }
    void setHasPushConstants(bool has = true) { m_HasPushContants = has; }
    void setIsDirty(bool dirty = true) { m_IsDirty.store(dirty); }
    void setName(std::string const& name) { m_Name = name; }
    inline void setShaderName(std::string_view name) { m_ShaderName = name; }
    void setObjectBuffer(ObjectBuffer objectBuffer) { m_ObjectBuffer = std::move(objectBuffer); }

    void addUbos(const std::vector<UniformBufferObject>& ubos);

    template<typename... TArgs>
    void applyPushConstants(TArgs&&... args) { m_ApplyPushConstants(std::forward<TArgs>(args)...); }

    VkDescriptorSet* getShadowMapDescSet() { return & m_ShadowMapDescSet; }

    bool hasAnimation() const { return m_HasAnimation; }
    bool hasBbox() const { return m_HasBbox; }
    bool hasBufferStorage() const { return m_HasStorageBuffer; }
    bool hasPushConstants() const { return m_HasPushContants; }
    bool hasShadow() { return m_HasShadow; }

    void setHasAnimation(bool has = true) { m_HasAnimation = has; }
    void setHasBbox(bool hasBbox = false) { m_HasBbox = hasBbox; }
    void setIsIndexed(bool indexed) { m_IsIndexed = indexed; }
    void setIsPointLight(bool is = true) { m_IsPointLight = is; }
    void setMaterial(material_t material) { m_Material = material; }
    void setHasShadow(bool hasShadow) { m_HasShadow = hasShadow; }
        
    bool isIndexed() const { return m_IsIndexed; }
    bool isPointLight() const { return m_IsPointLight; }

    /*void addBBox(BBox* bbox) { m_BoundingBox = std::unique_ptr<BBox>(bbox); }
    BBox* getBBox() { return m_BoundingBox.get(); }*/

  private:
    //std::unique_ptr<BBox> m_BoundingBox;
    std::string m_Name{};
    std::string m_ShaderName{};

    std::atomic<bool> m_IsDirty{ true };

    bool m_HasAnimation{ true };
    bool m_HasBbox{ false };
    bool m_HasPushContants{ false };
    bool m_HasStorageBuffer{ false };
    bool m_IsIndexed{ true };
    bool m_IsPointLight{ false };
    bool m_HasShadow{ false };

    std::vector<Buffer> m_StorageBuffers{};
    std::vector<Buffer> m_UniformBuffers{};
    ObjectBuffer m_ObjectBuffer;

    Data m_Data{};
    VkDescriptorSet m_DescSet{};
    VkDescriptorSet m_ShadowMapDescSet{};
    material_t m_Material{};

    std::function<void(VkCommandBuffer& commandBuffer,
      VkPipelineLayout pipelineLayout,
      Renderer* const renderer,
      Mesh* const mesh)> m_ApplyPushConstants = nullptr;
  };
}
