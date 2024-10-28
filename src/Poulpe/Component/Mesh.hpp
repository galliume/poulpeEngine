#pragma once

#include "Texture.hpp"
#include "Vertex.hpp"
#include "Vertex2D.hpp"

#include "Poulpe/Core/IVisitor.hpp"
#include "Poulpe/Core/PlpTypedef.hpp"

#include "Poulpe/Manager/ILightManager.hpp"
#include "Poulpe/Manager/ITextureManager.hpp"

#include "Poulpe/Renderer/IGraphicsAPI.hpp"
#include "Poulpe/Renderer/Vulkan/DeviceMemory.hpp" //@todo should not be here

#include <functional>

namespace Poulpe
{
    //@todo should not be IVisitor
    class Mesh : public IVisitor, public IVisitable
    {
    public:
        Mesh() = default;
        ~Mesh() override = default;

        Data* getData() override { return & m_Data; }
        VkDescriptorSet* getDescSet() override { return & m_DescSet; }
        
        material_t const getMaterial() const override { return m_Material; }
        std::string const getName() const override { return m_Name; }
        inline const std::string getShaderName() const override { return m_ShaderName; }
        std::vector<Buffer>* getStorageBuffers() override { return & m_StorageBuffers; }
        inline std::vector<Buffer>* getUniformBuffers() override { return & m_UniformBuffers; }
        ObjectBuffer* getObjectBuffer() { return &m_ObjectBuffer; };

        void addStorageBuffer(Buffer& buffer) override { m_StorageBuffers.emplace_back(std::move(buffer)); }
        void init(
             IRenderer* const renderer,
             ITextureManager* const textureManager, 
             ILightManager* const lightManager) override {}
        
        bool isDirty() override { return m_IsDirty.load(); }
        void visit(std::chrono::duration<float> deltaTime, IVisitable* const  visitable) override;

        virtual void setApplyPushConstants(
          std::function<void(VkCommandBuffer& commandBuffer,
            VkPipelineLayout pipelineLayout,
            IRenderer* const renderer,
            IVisitable* const mesh)> fn) override { m_ApplyPushConstants = fn; }

        void setData(Data data) override { m_Data = std::move(data); }
        void setDescSet(VkDescriptorSet descSet) override { m_DescSet = descSet; }
        void setShadowMapDescSet(VkDescriptorSet descSet) override { m_ShadowMapDescSet = descSet; }
        void setHasBufferStorage(bool has = true) override { m_HasStorageBuffer = has; }
        void setHasPushConstants(bool has = true) override { m_HasPushContants = has; }
        void setIsDirty(bool dirty = true) override { m_IsDirty.store(dirty); }
        void setName(std::string_view  name) override { m_Name = name; }
        inline void setShaderName(std::string_view name) override { m_ShaderName = name; }
        void setObjectBuffer(ObjectBuffer objectBuffer) override { m_ObjectBuffer = std::move(objectBuffer); }

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
        std::string m_Name;
        std::string m_ShaderName;

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
          IRenderer* const renderer,
          IVisitable* const mesh)> m_ApplyPushConstants = nullptr;
    };
}
