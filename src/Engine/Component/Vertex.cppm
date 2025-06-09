module;
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

#include <vulkan/vulkan.h>

#include <array>
#include <vector>

export module Engine.Component.Vertex;

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
  export struct Vertex
  {
    alignas(16) glm::vec3 pos;
    alignas(16) glm::vec3 normal;
    alignas(8)  glm::vec2 texture_coord;
    alignas(16) glm::vec4 tangent;
    alignas(16) glm::vec4 color;

    glm::vec3 original_pos;
    std::vector<uint32_t> bone_ids;
    std::vector<float> bone_weights;

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
      attributeDescriptions[4].offset = offsetof(Vertex, color);

      return attributeDescriptions;
    }

  /*  bool operator==(const Vertex& other) const
    {
      return pos == other.pos && normal == other.normal
        && texture_coord == other.texture_coord && other.tangent == tangent
        && color == other.color;
    }*/
    Vertex operator-(Vertex const& rhs)
    {
      Vertex v = rhs;
      v.pos = rhs.pos - pos;

      return v;
    }
  };

  //namespace std {
  //  template<> struct hash<Vertex> {
  //    size_t operator()(Vertex const& vertex) const {
  //      return ((((((((hash<glm::vec3>()(vertex.pos) ^
  //        (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
  //        (hash<glm::vec2>()(vertex.texture_coord) << 1)) >> 1) ^
  //        (hash<glm::vec3>()(vertex.tangent) << 1)) >> 1) ^
  //        (hash<glm::vec3>()(vertex.color) << 1)));
  //    }
  //  };
  //}

  export struct Vertex2D
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

      // bool operator==(const Vertex2D& other) const
      // {
      //     return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
      // }
  };

  // namespace std {
  //     template<> struct hash<Vertex2D> {
  //         size_t operator()(Vertex2D const& vertex) const {
  //             return ((hash<glm::vec2>()(vertex.pos) ^
  //                 (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
  //                 (hash<glm::vec2>()(vertex.texCoord) << 1);
  //         }
  //     };
  // }
}
