export module Engine.Component.Mesh;

import std;

import Engine.Core.AnimationTypes;
import Engine.Core.MaterialTypes;
import Engine.Core.GLM;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;
import Engine.Core.Vertex;
import Engine.Core.Volk;

import Engine.Utils.IDHelper;

namespace Poulpe
{
  export class Mesh
  {
  public:
    Data * getData() { return & _data; }
    Data const * getData() const { return & _data; }
    VkDescriptorSet const * getDescSet() const { return & _descset; }
    VkDescriptorSet * getDescSet() { return & _descset; }
    std::vector<material_t> const& getMaterials() const { return _materials; }
    std::vector<material_t> & getMaterials() { return _materials; }
    std::string const getName() const { return _name; }
    inline const std::string getShaderName() const { return _shader_name; }
    std::vector<Buffer>* getStorageBuffers() { return & _storage_buffers; }
    inline std::vector<Buffer>& getUniformBuffers() { return _uniform_buffers; }
    ObjectBuffer* getObjectBuffer() { return &_object_buffer; }

    void addStorageBuffer(Buffer& buffer);
    bool isDirty() { return _is_dirty.load(); }

    void setData(Data data) { _data = std::move(data); }
    void setDescSet(VkDescriptorSet descset) { _descset = descset; }
    void setShadowMapDescSet(VkDescriptorSet descset) { _shadowmap_descset = descset; }
    void setCSMDescSet(VkDescriptorSet descset) { _csm_descset = descset; }
    void setHasBufferStorage(bool has = true) { _has_storage_buffer = has; }
    void setHasPushConstants(bool has = true) { _has_push_constants = has; }
    void setIsDirty(bool dirty = true) { _is_dirty.store(dirty); }
    void setDebugNormal(bool const debug) { _debug_normal = debug; }
    void setName(std::string const& name) { _name = name; }
    inline void setShaderName(std::string_view name) { _shader_name = name; }
    void setObjectBuffer(ObjectBuffer objectBuffer) { _object_buffer = std::move(objectBuffer); }

    void addUbos(std::vector<std::vector<UniformBufferObject>> const& ubos);

    VkDescriptorSet const * getShadowMapDescSet() const { return & _shadowmap_descset; }
    VkDescriptorSet * getShadowMapDescSet() { return & _shadowmap_descset; }
    VkDescriptorSet const * getCSMDescSet() const { return & _csm_descset; }
    VkDescriptorSet * getCSMDescSet() { return & _csm_descset; }

    bool hasAnimation() const { return _has_animation; }
    bool hasBufferStorage() const { return _has_storage_buffer; }
    bool hasPushConstants() const { return _has_push_constants; }
    bool hasShadow() const { return _has_shadow; }
    bool debugNormal() const { return _debug_normal; }

    void setHasAnimation(bool has = true) { _has_animation = has; }
    void setIsIndexed(bool indexed) { _is_indexed = indexed; }
    void setIsPointLight(bool is = true) { _is_point_light = is; }
    void addMaterial(material_t material) { _materials.emplace_back(std::move(material)); }
    void setHasShadow(bool has_shadow) { _has_shadow = has_shadow; }

    bool is_indexed() const { return _is_indexed; }
    bool is_point_light() const { return _is_point_light; }

    std::uint32_t getOptions() const { return _options; }
    void setOptions(std::uint32_t options) { _options = options; }

    VkShaderStageFlags stage_flag_bits { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT};

    bool isClipped() const { return _is_clipped; }
    void clipped(bool clipped) { _is_clipped = clipped; }

    bool hasBbox() const { return _has_bbox; }
    void bbox(bool bbox) { _has_bbox = bbox; }

    bool isSkybox() const { return _is_skybox; }
    void isSkybox(bool is_skybox) { _is_skybox = is_skybox; }
    void addChild(IDType id) { _children.emplace_back(id); }
    std::vector<IDType> getChildren() { return _children; }

    void setRoot() { _is_root = true; }
    bool isRoot() { return _is_root ;}

    private:
    std::string _name{};
    std::string _shader_name{};

    std::atomic<bool> _is_dirty{ true };

    bool _has_animation{ true };
    bool _has_push_constants{ false };
    bool _has_storage_buffer{ false };
    bool _is_indexed{ true };
    bool _is_point_light{ false };
    bool _has_shadow{ false };
    bool _debug_normal{ false };
    bool _is_clipped { false };
    bool _has_bbox { false };
    bool _is_skybox { false };

    std::vector<Buffer> _storage_buffers;
    std::vector<Buffer> _uniform_buffers;
    ObjectBuffer _object_buffer;

    Data _data{};
    VkDescriptorSet _descset{};
    VkDescriptorSet _shadowmap_descset{};
    VkDescriptorSet _csm_descset{};
    std::vector<material_t> _materials{};

    std::uint32_t _options{ 0 }; //used for options as push constants
    std::uint32_t _default_anim{};

    std::vector<IDType> _children{};
    bool _is_root { false };
  };
}
