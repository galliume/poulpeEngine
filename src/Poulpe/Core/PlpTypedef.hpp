#pragma once

#include "Poulpe/Component/Vertex.hpp"

namespace Poulpe
{
    class DeviceMemory;

    struct CameraUBO {
      alignas(16) glm::mat4 view;
      alignas(16) glm::mat4 proj;
      alignas(16) glm::mat4 projXview;
    };

    struct UniformBufferObject
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 projection;
        alignas(4) float constant{ 1.0f };
        alignas(4) float linear{ 0.09f };
        alignas(4) float quadratic{ 0.032f };
        alignas(8) glm::vec2 texSize;
    };

    struct CubeUniformBufferObject : UniformBufferObject
    {
        int index;
    };

    struct Light
    {
        alignas(16) glm::vec3 color;
        alignas(16) glm::vec3 direction;
        alignas(16) glm::vec3 position;
        //ambiance diffuse specular
        alignas(16) glm::vec3 ads;
        //constant, linear, quadratiq
        alignas(16) glm::vec3 clq;
        //cutOff, outerCutoff Blank
        alignas(16) glm::vec3 coB{ 1.0f, 0.0f, 0.0f };
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 projection;
        alignas(16) glm::mat4 lightSpaceMatrix;
    };

    struct Material
    {
        alignas(16) glm::vec3 ambient;
        alignas(16) glm::vec3 diffuse;
        alignas(16) glm::vec3 specular;
        alignas(16) glm::vec3 transmittance;
        alignas(16) glm::vec3 emission;
        //shininess, ior, diss
        alignas(16) glm::vec3 shiIorDiss;
    };

    struct ObjectBuffer
    {
        Light ambientLight;
        std::array<Light, 2> pointLights;
        Light spotLight;
        Material material;
    };

    struct material_t {
      std::string name;
      glm::vec3 ambient{ 1.0 };
      glm::vec3 diffuse{ 1.0 };
      glm::vec3 specular{ 1.0 };
      glm::vec3 transmittance{ 1.0 };
      glm::vec3 emission{ 1.0 };
      float shininess{ 1.0 };
      float ior{ 1.0 };       // index of refraction
      float dissolve{ 1.0 };  // 1 == opaque; 0 == fully transparent
      int illum{ 1 };
      std::string ambientTexname;             // map_Ka
      std::string diffuseTexname;             // map_Kd
      std::string specularTexname;            // map_Ks
      std::string specularHighlightTexname;  // map_Ns
      std::string bumpTexname;                // map_bump, map_Bump, bump
      std::string alphaTexname; // map_d
    };

    struct constants
    {
      //textureID blank blank
      alignas(16) glm::vec3 textureIDBB;
      alignas(16) glm::mat4 view;
      alignas(16) glm::vec4 viewPos;
    };

    struct Buffer {
      VkBuffer buffer;
      DeviceMemory* memory;
      uint32_t offset;
      uint32_t size;
    };

    struct Data {
      std::string m_Name;
      std::vector<std::string> m_Textures;
      std::string m_TextureSpecularMap;
      std::string m_TextureBumpMap;
      std::string m_TextureNormalMap;
      std::string m_TextureAlpha;
      std::vector<Vertex> m_Vertices;
      std::vector<uint32_t> m_Indices;
      std::vector<UniformBufferObject> m_Ubos;
      std::vector<uint32_t> m_UbosOffset;
      Buffer m_VertexBuffer = { nullptr, nullptr, 0, 0 };
      Buffer m_IndicesBuffer = { nullptr, nullptr, 0, 0 };
      uint32_t m_TextureIndex = 0;
      glm::vec3 m_OriginPos;
      glm::vec3 m_OriginScale;
      glm::vec3 m_OriginRotation;
      glm::vec3 m_CurrentPos;
      glm::vec3 m_CurrentRotation;
      glm::vec3 m_CurrentScale;
      glm::vec3 tangeant;
    };

    //struct BBox
    //{
    //  glm::mat4 position;
    //  glm::vec3 center;
    //  glm::vec3 size;
    //  std::unique_ptr<Mesh> mesh;
    //  float maxX;
    //  float minX;
    //  float maxY;
    //  float minY;
    //  float maxZ;
    //  float minZ;
    //};
}
