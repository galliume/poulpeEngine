#pragma once
#include <volk.h>
#include <array>
#include <glm/glm.hpp>

namespace Poulpe
{
    struct Vertex2D
    {
        alignas(8) glm::vec2 pos;
        alignas(16) glm::vec3 normal;
        alignas(8) glm::vec2 texCoord;

        static VkVertexInputBindingDescription getBindingDescription() {

            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex2D);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {

            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex2D, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex2D, normal);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex2D, texCoord);

            return attributeDescriptions;
        }

        bool operator==(const Vertex2D& other) const
        {
            return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
        }
    };
}

namespace std {
    template<> struct hash<Poulpe::Vertex2D> {
        size_t operator()(Poulpe::Vertex2D const& vertex) const {
            return ((hash<glm::vec2>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}
