#pragma once

#include <volk.h>
#include "Texture.h"
#include "Buffer.h"
#include "Vertex.h"
#include "Vertex2D.h"
#include "Rebulk/Component/Entity.h"
#include "Rebulk/Component/Drawable.h"
#include "Rebulk/Core/TinyObjLoader.h"

namespace Rbk
{
    struct Data
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        uint32_t materialId = 0;
    };

    class Mesh : public Entity, public Drawable
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

    //@todo make it private
    public:
        std::string m_Texture;
        std::string m_Name;
        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
        std::vector<UniformBufferObject> m_Ubos;
        std::pair<VkBuffer, VkDeviceMemory> m_VertexBuffer = { nullptr, nullptr };
        std::pair<VkBuffer, VkDeviceMemory> m_IndicesBuffer = { nullptr, nullptr };
        std::vector<std::pair<VkBuffer, VkDeviceMemory>> m_UniformBuffers;
        std::vector<VkDescriptorSet> m_DescriptorSets;
        VkPipelineLayout m_PipelineLayout;
        VkPipeline m_GraphicsPipeline;
        VkPipelineCache m_PipelineCache = nullptr;
        glm::vec4 m_CameraPos;
        std::string m_Shader;
    };
}

namespace std {
    template<> struct hash<Rbk::Vertex> {
        size_t operator()(Rbk::Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
    template<> struct hash<Rbk::Vertex2D> {
        size_t operator()(Rbk::Vertex2D const& vertex) const {
            return ((hash<glm::vec2>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}
