#pragma once

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

    struct cPC
    {
        uint32_t textureID;
    };

    class Mesh : public Entity
    {
    public:
        Mesh();
        ~Mesh();

        void Init(std::string name,
            std::string path,
            std::vector<std::string> textureNames,
            std::string shader,
            glm::vec3 pos,
            glm::vec3 scale,
            glm::vec3 axisRot = glm::vec3(1.0f),
            float rotAngle = 0.0f,
            bool shouldInverseTextureY = true);

        std::vector<Data>* GetData() { return &m_Data; };
        inline std::string GetShaderName() { return m_ShaderName; };
        void AddUbos(const std::vector<UniformBufferObject> ubos);

    //@todo make it private
    public:
        std::vector<std::pair<VkBuffer, VkDeviceMemory>> m_UniformBuffers;
        glm::vec4 m_CameraPos;
        std::vector<VkDescriptorSet> m_DescriptorSets;
        VkPipelineLayout m_PipelineLayout;
        VkPipeline m_GraphicsPipeline;
        VkPipelineCache m_PipelineCache = nullptr;
    
    private:
        std::vector<Data> m_Data;
        std::string m_ShaderName;
    };
}
