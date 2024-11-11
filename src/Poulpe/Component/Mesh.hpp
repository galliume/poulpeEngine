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
    Data* getData() { return & _Data; }
    VkDescriptorSet* getDescSet() { return & _DescSet; }
    material_t const getMaterial() const { return _Material; }
    std::string const getName() const { return _name; }
    inline const std::string getShaderName() const { return _ShaderName; }
    std::vector<Buffer>* getStorageBuffers() { return & _StorageBuffers; }
    inline std::vector<Buffer>* getUniformBuffers() { return & _UniformBuffers; }
    ObjectBuffer* getObjectBuffer() { return &_ObjectBuffer; };

    void addStorageBuffer(Buffer& buffer) { _StorageBuffers.emplace_back(std::move(buffer)); }
    void init(
          Renderer* const renderer,
          TextureManager* const texture_manager, 
          LightManager* const light_manager) {}
        
    bool isDirty() { return _IsDirty.load(); }

    virtual void setApplyPushConstants(
      std::function<void(VkCommandBuffer& cmd_buffer,
        VkPipelineLayout pipelineLayout,
        Renderer* const renderer,
        Mesh* const mesh)> fn) { _ApplyPushConstants = fn; }

    void setData(Data data) { _Data = std::move(data); }
    void setDescSet(VkDescriptorSet descSet) { _DescSet = descSet; }
    void setShadowMapDescSet(VkDescriptorSet descSet) { _ShadowMapDescSet = descSet; }
    void setHasBufferStorage(bool has = true) { _HasStorageBuffer = has; }
    void setHasPushConstants(bool has = true) { _HasPushContants = has; }
    void setIsDirty(bool dirty = true) { _IsDirty.store(dirty); }
    void setName(std::string const& name) { _name = name; }
    inline void setShaderName(std::string_view name) { _ShaderName = name; }
    void setObjectBuffer(ObjectBuffer objectBuffer) { _ObjectBuffer = std::move(objectBuffer); }

    void addUbos(const std::vector<UniformBufferObject>& ubos);

    template<typename... TArgs>
    void applyPushConstants(TArgs&&... args) { _ApplyPushConstants(std::forward<TArgs>(args)...); }

    VkDescriptorSet* getShadowMapDescSet() { return & _ShadowMapDescSet; }

    bool hasAnimation() const { return _HasAnimation; }
    bool hasBbox() const { return _HasBbox; }
    bool hasBufferStorage() const { return _HasStorageBuffer; }
    bool hasPushConstants() const { return _HasPushContants; }
    bool hasShadow() { return _HasShadow; }

    void setHasAnimation(bool has = true) { _HasAnimation = has; }
    void setHasBbox(bool hasBbox = false) { _HasBbox = hasBbox; }
    void setIsIndexed(bool indexed) { _IsIndexed = indexed; }
    void setIsPointLight(bool is = true) { _IsPointLight = is; }
    void setMaterial(material_t material) { _Material = material; }
    void setHasShadow(bool hasShadow) { _HasShadow = hasShadow; }
        
    bool isIndexed() const { return _IsIndexed; }
    bool isPointLight() const { return _IsPointLight; }

    /*void addBBox(BBox* bbox) { _BoundingBox = std::unique_ptr<BBox>(bbox); }
    BBox* getBBox() { return _BoundingBox.get(); }*/

  private:
    //std::unique_ptr<BBox> _BoundingBox;
    std::string _name{};
    std::string _ShaderName{};

    std::atomic<bool> _IsDirty{ true };

    bool _HasAnimation{ true };
    bool _HasBbox{ false };
    bool _HasPushContants{ false };
    bool _HasStorageBuffer{ false };
    bool _IsIndexed{ true };
    bool _IsPointLight{ false };
    bool _HasShadow{ false };

    std::vector<Buffer> _StorageBuffers{};
    std::vector<Buffer> _UniformBuffers{};
    ObjectBuffer _ObjectBuffer;

    Data _Data{};
    VkDescriptorSet _DescSet{};
    VkDescriptorSet _ShadowMapDescSet{};
    material_t _Material{};

    std::function<void(VkCommandBuffer& cmd_buffer,
      VkPipelineLayout pipelineLayout,
      Renderer* const renderer,
      Mesh* const mesh)> _ApplyPushConstants = nullptr;
  };
}
