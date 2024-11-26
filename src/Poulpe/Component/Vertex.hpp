#pragma once

#include <array>

#include <glm/glm.hpp>

namespace Poulpe
{
  /*
    format:
    float : VK_FORMAT_R32_SFLOAT
    vec2 : VK_FORMAT_R32G32_SFLOAT
    vec3 : VK_FORMAT_R32G32B32_SFLOAT
    vec4 : VK_FORMAT_R32G32B32A32_SFLOAT
  */
  struct Vertex
  {
    alignas(16) glm::vec3 pos;
    alignas(16) glm::vec3 normal;
    alignas(16) glm::vec2 texture_coord;
    alignas(16) glm::vec4 tangent;
    //faceId texture ID blank blank
    alignas(16) glm::vec4 fidtidBB;
    alignas(16) glm::vec3 color;

    std::vector<int> bones_ids{};
    std::vector<float> weights{};

    static VkVertexInputBindingDescription getBindingDescription()
    {
      VkVertexInputBindingDescription bindingDescription{};
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(Vertex);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions() 
    {
      std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions{};
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
      attributeDescriptions[2].offset = offsetof(Vertex, texture_coord);

      attributeDescriptions[3].binding = 0;
      attributeDescriptions[3].location = 3;
      attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      attributeDescriptions[3].offset = offsetof(Vertex, tangent);

      attributeDescriptions[4].binding = 0;
      attributeDescriptions[4].location = 4;
      attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      attributeDescriptions[4].offset = offsetof(Vertex, fidtidBB);

      attributeDescriptions[5].binding = 0;
      attributeDescriptions[5].location = 5;
      attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[5].offset = offsetof(Vertex, color);

      return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const
    {
      return pos == other.pos && normal == other.normal 
        && texture_coord == other.texture_coord && other.tangent == tangent
        && fidtidBB == other.fidtidBB && color == other.color;
    }
  };
}

namespace std {
  template<> struct hash<Poulpe::Vertex> {
    size_t operator()(Poulpe::Vertex const& vertex) const {
      return ((((((((hash<glm::vec3>()(vertex.pos) ^
        (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
        (hash<glm::vec2>()(vertex.texture_coord) << 1)) >> 1) ^
        (hash<glm::vec3>()(vertex.tangent) << 1)) >> 1) ^
        (hash<glm::vec3>()(vertex.fidtidBB) << 1)) >> 1) ^
        (hash<glm::vec3>()(vertex.color) << 1);
    }
  };
}
