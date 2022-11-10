#pragma once

#include <functional>
#include <volk.h>
#include "Texture.h"
#include "Vertex.h"
#include "Vertex2D.h"
#include "Rebulk/Component/Entity.h"
#include "Rebulk/Core/TinyObjLoader.h"
#include "Rebulk/Core/Buffer.h"
#include "Rebulk/Core/MeshData.h"

namespace Rbk
{
    struct constants 
    {
        glm::vec4 data;
        glm::vec4 cameraPos;
        glm::vec4 fogColor;
        glm::vec4 lightPos;
        glm::vec4 rayPick;
        glm::mat4 view;
    };

    class VulkanAdapter;

    class Mesh : public Entity
    {
    public:
        Mesh();
        ~Mesh() = default;

        void Init(const std::string& name,
            const std::string& path,
            const std::vector<std::string>& textureNames,
            const std::string& shader,
            const glm::vec3& pos,
            const glm::vec3& scale,
            glm::vec3 rotation = glm::vec3(1.0f),
            bool shouldInverseTextureY = true);

        std::vector<Data>* GetData() { return &m_Data; }
        
        inline const std::string GetShaderName() const { return m_ShaderName; }
        inline void SetShaderName(std::string_view name) { m_ShaderName = name; }
        inline const std::vector<VkDescriptorSet> GetDescriptorSets() const { return m_DescriptorSets; }

        void AddUbos(const std::vector<UniformBufferObject>& ubos);
        std::function<void(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, std::shared_ptr<VulkanAdapter>, Data& data)> ApplyPushConstants = nullptr;
        
        bool IsDirty() { return m_IsDirty; }
        void SetIsDirty(bool dirty = true) { m_IsDirty = dirty; }

    //@todo make it private
    public:
        std::vector<Buffer> m_UniformBuffers;
        glm::vec4 m_CameraPos;
        std::vector<VkDescriptorSet> m_DescriptorSets;
        VkPipelineLayout m_PipelineLayout;
        VkPipeline m_GraphicsPipeline;
    
    private:
        std::vector<Data> m_Data = {};
        std::string m_ShaderName;
        bool m_IsDirty = true;
    };
}
