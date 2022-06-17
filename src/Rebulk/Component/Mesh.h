#pragma once

#include <volk.h>
#include "Texture.h"
#include "Buffer.h"
#include "Vertex.h"
#include "Vertex2D.h"
#include "Rebulk/Component/Drawable.h"

namespace Rbk
{
    class Mesh : public Drawable
    {
    public:
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        uint32_t materialId = 0;
        std::string texture;
        std::string name;
        std::vector<UniformBufferObject> ubos;
        std::pair<VkBuffer, VkDeviceMemory> vertexBuffer = { nullptr, nullptr };
        std::pair<VkBuffer, VkDeviceMemory> indicesBuffer = { nullptr, nullptr };
        std::vector<std::pair<VkBuffer, VkDeviceMemory>> uniformBuffers;
        std::vector<VkDescriptorSet> descriptorSets;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;
        VkPipelineCache pipelineCache = nullptr;
        glm::vec4 cameraPos;
        std::string shader;
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
