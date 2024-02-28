#pragma once

#include <array>

#include <glm/glm.hpp>

namespace Poulpe
{
  struct Vertex
  {
    alignas(16) glm::vec3 pos;
    alignas(16) glm::vec3 normal;
    alignas(8) glm::vec2 texCoord;
    alignas(16) glm::vec4 tangent;
    alignas(16) glm::vec3 bitangent;
    //faceId texture ID blank blank
    alignas(16) glm::vec4 fidtidBB;
    alignas(16) glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() 
    {
      VkVertexInputBindingDescription bindingDescription{};
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(Vertex);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 7> getAttributeDescriptions() 
    {
      std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions{};
      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(Vertex, pos);

      attributeDescriptions[1].binding = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[1].offset = offsetof(Vertex, normal);

      attributeDescriptions[2].binding = 0;
      attributeDescriptions[2].location = 2;
      attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

      attributeDescriptions[3].binding = 0;
      attributeDescriptions[3].location = 3;
      attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[3].offset = offsetof(Vertex, tangent);

      attributeDescriptions[4].binding = 0;
      attributeDescriptions[4].location = 4;
      attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[4].offset = offsetof(Vertex, bitangent);

      attributeDescriptions[5].binding = 0;
      attributeDescriptions[5].location = 5;
      attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[5].offset = offsetof(Vertex, fidtidBB);

      attributeDescriptions[6].binding = 0;
      attributeDescriptions[6].location = 6;
      attributeDescriptions[6].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[6].offset = offsetof(Vertex, color);

      return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const
    {
      return pos == other.pos && normal == other.normal 
        && texCoord == other.texCoord;
    }
  };
}

namespace std {
    template<> struct hash<Poulpe::Vertex> {
        size_t operator()(Poulpe::Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}
