#pragma once

#include <functional>
#include <volk.h>
#include "Texture.h"
#include "Vertex.h"
#include "Vertex2D.h"
#include "Rebulk/Component/Entity.h"
#include "Rebulk/Core/TinyObjLoader.h"
#include "Rebulk/Core/Buffer.h"

namespace Rbk
{
    struct Data
    {
        std::string m_Name;
        std::string m_Texture;
        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
        std::vector<UniformBufferObject> m_Ubos;
        std::pair<VkBuffer, VkDeviceMemory> m_VertexBuffer = { nullptr, nullptr };
        std::pair<VkBuffer, VkDeviceMemory> m_IndicesBuffer = { nullptr, nullptr };
        uint32_t m_TextureIndex = 0;
    };

    struct constants 
    {
        uint32_t textureID;
        glm::vec3 cameraPos;
        float ambiantLight;
        float fogDensity;
        glm::vec3 fogColor;
        glm::vec3 lightPos;
    };

    class Mesh : public Entity
    {
    public:
        Mesh();
        ~Mesh();

        void Init(const std::string& name,
            const std::string& path,
            const std::vector<std::string>& textureNames,
            const std::string& shader,
            const glm::vec3& pos,
            const glm::vec3& scale,
            glm::vec3 rotation = glm::vec3(1.0f),
            bool shouldInverseTextureY = true);

        std::vector<Data>* GetData() { return &m_Data; };
        
        inline const std::string GetShaderName() const { return m_ShaderName; };
        inline void SetShaderName(std::string_view name) { m_ShaderName = name; };
        inline const std::vector<VkDescriptorSet> GetDescriptorSets() const { return m_DescriptorSets; };

        void AddUbos(const std::vector<UniformBufferObject>& ubos);
        std::function<void(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)> ApplyPushConstants = nullptr;

    //@todo make it private
    public:
        std::vector<std::pair<VkBuffer, VkDeviceMemory>> m_UniformBuffers;
        glm::vec4 m_CameraPos;
        std::vector<VkDescriptorSet> m_DescriptorSets;
        VkPipelineLayout m_PipelineLayout;
        VkPipeline m_GraphicsPipeline;
    
    private:
        std::vector<Data> m_Data = {};
        std::string m_ShaderName;
    };
}
