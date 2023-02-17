#pragma once

#include <functional>
#include "Texture.hpp"
#include "Vertex.hpp"
#include "Vertex2D.hpp"
#include "Rebulk/Component/Entity.hpp"
#include "Rebulk/Core/TinyObjLoader.hpp"
#include "Rebulk/Core/Buffer.hpp"
#include "Rebulk/Core/MeshData.hpp"

namespace Rbk
{
    struct constants 
    {
        glm::vec4 data;
        glm::vec4 cameraPos;
        glm::vec4 fogColor;
        glm::vec4 lightPos;
        glm::mat4 view;
        float ambiantLight;
        float fogDensity;
    };

    class VulkanAdapter;

    class Mesh : public Entity
    {
    public:
        Mesh();
        ~Mesh() = default;

        std::vector<std::shared_ptr<Mesh>> Init(const std::string& name,
            const std::string& path,
            const std::vector<std::string>& textureNames,
            const std::string& shader,
            const glm::vec3& pos,
            const glm::vec3& scale,
            glm::vec3 rotation = glm::vec3(1.0f),
            bool shouldInverseTextureY = true);

        Data* GetData() { return &m_Data; }
        void SetData(Data& data) { m_Data = data; }
        
        inline const std::string GetShaderName() const { return m_ShaderName; }
        inline void SetShaderName(std::string_view name) { m_ShaderName = name; }
        inline std::vector<VkDescriptorSet> GetDescriptorSets() { return m_DescriptorSets; }

        void AddUbos(const std::vector<UniformBufferObject>& ubos);
        std::function<void(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, std::shared_ptr<VulkanAdapter>, Data& data)> ApplyPushConstants = nullptr;
        
        bool IsDirty() { return m_IsDirty; }
        void SetIsDirty(bool dirty = true) { m_IsDirty = dirty; }
        void SetHasBbox(bool hasBbox = false) { m_HasBbox = hasBbox; }
        bool HasBbox() { return m_HasBbox; }

    //@todo make it private
    public:
        std::vector<Buffer> m_UniformBuffers;
        glm::vec4 m_CameraPos;
        std::vector<VkDescriptorSet> m_DescriptorSets;
        VkPipelineLayout m_PipelineLayout;
        VkPipeline m_GraphicsPipeline;
    
    private:
        Data m_Data;
        std::string m_ShaderName;
        bool m_IsDirty = true;
        bool m_HasBbox = true;
    };
}
