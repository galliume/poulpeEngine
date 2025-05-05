#pragma once

#include "Texture.hpp"
#include "Vertex.hpp"
#include "Vertex2D.hpp"

#include "Poulpe/Core/PlpTypedef.hpp"

#include <atomic>
#include <functional>

namespace Poulpe
{
  class LightManager;
  class Renderer;
  class TextureManager;

  class Mesh
  {
  public:
    Data* getData() { return & _data; }
    VkDescriptorSet* getDescSet() { return & _descset; }
    material_t& getMaterial() { return _material; }
    std::string const getName() const { return _name; }
    inline const std::string getShaderName() const { return _shader_name; }
    std::vector<Buffer>* getStorageBuffers() { return & _storage_buffers; }
    inline std::vector<Buffer>* getUniformBuffers() { return & _uniform_buffers; }
    ObjectBuffer* getObjectBuffer() { return &_object_buffer; };

    void addStorageBuffer(Buffer& buffer) { _storage_buffers.emplace_back(std::move(buffer)); }
    void init(
          Renderer* const renderer,
          TextureManager* const texture_manager,
          LightManager* const light_manager) {}

    bool isDirty() { return _is_dirty.load(); }

    virtual void setApplyPushConstants(
      std::function<void(VkCommandBuffer& cmd_buffer,
        VkPipelineLayout pipeline_layout,
        Renderer* const renderer,
        Mesh* const mesh)> fn) { _apply_push_constants = fn; }

    void setData(Data data) { _data = std::move(data); }
    void setDescSet(VkDescriptorSet descset) { _descset = descset; }
    void setShadowMapDescSet(VkDescriptorSet descset) { _shadowmap_descset = descset; }
    void setHasBufferStorage(bool has = true) { _has_storage_buffer = has; }
    void setHasPushConstants(bool has = true) { _has_push_contants = has; }
    void setIsDirty(bool dirty = true) { _is_dirty.store(dirty); }
    void setDebugNormal(bool const debug) { _debug_normal = debug; }
    void setName(std::string const& name) { _name = name; }
    inline void setShaderName(std::string_view name) { _shader_name = name; }
    void setObjectBuffer(ObjectBuffer objectBuffer) { _object_buffer = std::move(objectBuffer); }

    void addUbos(std::vector<std::vector<UniformBufferObject>> const& ubos);

    template<typename... TArgs>
    void applyPushConstants(TArgs&&... args) { _apply_push_constants(std::forward<TArgs>(args)...); }

    VkDescriptorSet* getShadowMapDescSet() { return & _shadowmap_descset; }

    bool hasAnimation() const { return _has_animation; }
    bool hasBufferStorage() const { return _has_storage_buffer; }
    bool hasPushConstants() const { return _has_push_contants; }
    bool hasShadow() const { return _has_shadow; }
    bool debugNormal() const { return _debug_normal; }

    void setHasAnimation(bool has = true) { _has_animation = has; }
    void setIsIndexed(bool indexed) { _is_indexed = indexed; }
    void setIsPointLight(bool is = true) { _is_point_light = is; }
    void setMaterial(material_t material) { _material = material; }
    void setHasShadow(bool has_shadow) { _has_shadow = has_shadow; }

    bool is_indexed() const { return _is_indexed; }
    bool is_point_light() const { return _is_point_light; }

    glm::vec4 getOptions() { return _options; };
    void setOptions(glm::vec4 options) { _options = options; };

  private:
    std::string _name{};
    std::string _shader_name{};

    std::atomic<bool> _is_dirty{ true };

    bool _has_animation{ true };
    bool _has_push_contants{ false };
    bool _has_storage_buffer{ false };
    bool _is_indexed{ true };
    bool _is_point_light{ false };
    bool _has_shadow{ false };
    bool _debug_normal{ false };

    std::vector<Buffer> _storage_buffers{};
    std::vector<Buffer> _uniform_buffers{};
    ObjectBuffer _object_buffer;

    unsigned int _default_anim{};

    Data _data{};
    VkDescriptorSet _descset{};
    VkDescriptorSet _shadowmap_descset{};
    material_t _material{};

    std::function<void(VkCommandBuffer& cmd_buffer,
      VkPipelineLayout pipeline_layout,
      Renderer* const renderer,
      Mesh* const mesh)> _apply_push_constants = nullptr;

    glm::vec4 _options{ 0.0 }; //used for options as push constants
  };
}
