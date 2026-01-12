module;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

#include <vulkan/vulkan.h>

export module Engine.Component.Vertex;

import std;

namespace Poulpe
{
  /*
    format:
    float : VK_FORMAT_R32_SFLOAT
    vec2 : VK_FORMAT_R32G32_SFLOAT
    vec3 : VK_FORMAT_R32G32B32_SFLOAT
    vec4 : VK_FORMAT_R32G32B32A32_SFLOAT

    float alignas(4)
    vec2 alignas(8)
    vec3/vec4/mat4 alignas(16)
    */
  export struct VertexBones
  {
    int32_t bone_ids[4];        // 16 bytes
    float   bone_weights[4];    // 16 bytes

    glm::vec3 original_pos;     // 12 bytes
    
    float total_weight;         // 4 bytes
  };

  export struct Vertex
  {
    alignas(16) glm::vec4 tangent;
    alignas(16) glm::vec4 color;

    alignas(16) glm::vec4 pos;
    alignas(16) glm::vec4 normal;

    alignas(8) glm::vec2 texture_coord;

    static VkVertexInputBindingDescription getBindingDescription()
    {
      VkVertexInputBindingDescription bindingDescription{};
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(Vertex);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription,  5> getAttributeDescriptions()
    {
      std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};
      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(Vertex, tangent);

      // Location 1: Color (vec4)
      attributeDescriptions[1].binding = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      attributeDescriptions[1].offset = offsetof(Vertex, color);

      // Location 2: Position (vec3)
      attributeDescriptions[2].binding = 0;
      attributeDescriptions[2].location = 2;
      attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      attributeDescriptions[2].offset = offsetof(Vertex, pos);

      // Location 3: Normal (vec3)
      attributeDescriptions[3].binding = 0;
      attributeDescriptions[3].location = 3;
      attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      attributeDescriptions[3].offset = offsetof(Vertex, normal);

      // Location 4: Original Position (vec3)
      // attributeDescriptions[4].binding = 0;
      // attributeDescriptions[4].location = 4;
      // attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
      // attributeDescriptions[4].offset = offsetof(Vertex, original_pos);

      // Location 5: Texture Coord (vec2)
      attributeDescriptions[4].binding = 0;
      attributeDescriptions[4].location = 4;
      attributeDescriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[4].offset = offsetof(Vertex, texture_coord);

      // // Location 6: Bone IDs (ivec4) - Note the _SINT format
      // attributeDescriptions[6].binding = 0;
      // attributeDescriptions[6].location = 6;
      // attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SINT; 
      // attributeDescriptions[6].offset = offsetof(Vertex, bone_ids);

      // // Location 7: Bone Weights (vec4)
      // attributeDescriptions[7].binding = 0;
      // attributeDescriptions[7].location = 7;
      // attributeDescriptions[7].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      // attributeDescriptions[7].offset = offsetof(Vertex, bone_weights);

      // // Location 8: Total Weight (float)
      // attributeDescriptions[8].binding = 0;
      // attributeDescriptions[8].location = 8;
      // attributeDescriptions[8].format = VK_FORMAT_R32_SFLOAT;
      // attributeDescriptions[8].offset = offsetof(Vertex, total_weight);

      return attributeDescriptions;
    }
  };
}
