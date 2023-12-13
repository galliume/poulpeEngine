#pragma once

#include "Texture.hpp"
#include "Vertex.hpp"
#include "Vertex2D.hpp"

#include "Poulpe/Core/Buffer.hpp"

#include "Poulpe/Renderer/Vulkan/DeviceMemory.hpp"

#include <functional>

namespace Poulpe
{
    class VulkanAdapter;//@todo should not be here

    struct material_t {
      std::string name;
      glm::vec3 ambient;
      glm::vec3 diffuse;
      glm::vec3 specular;
      glm::vec3 transmittance;
      glm::vec3 emission;
      float shininess;
      float ior;       // index of refraction
      float dissolve;  // 1 == opaque; 0 == fully transparent
      int illum;
      std::string ambientTexname;             // map_Ka
      std::string diffuseTexname;             // map_Kd
      std::string specularTexname;            // map_Ks
      std::string specularHighlightTexname;  // map_Ns
      std::string bumpTexname;                // map_bump, map_Bump, bump
    };

    struct constants
    {
      //textureID blank blank
        alignas(16) glm::vec3 textureIDBB;
        alignas(16) glm::mat4 view;
        alignas(16) glm::vec4 viewPos;
        alignas(16) glm::vec3 mapsUsed = glm::vec3(0.0f);
    };

    class Mesh
    {
    public:
         struct Buffer {
            VkBuffer buffer;
            DeviceMemory* memory;
            uint32_t offset;
            uint32_t size;
        };

        struct Data {
            std::string m_Name;
            std::string m_Texture;
            std::string m_TextureSpecularMap;
            std::string m_TextureBumpMap;
            std::string m_TextureNormalMap;
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
            //x = specularMap, y = bumMap
            glm::vec3 mapsUsed = glm::vec3(0.0f);
            glm::vec3 tangeant;
        };

        struct BBox
        {
            glm::mat4 position;
            glm::vec3 center;
            glm::vec3 size;
            std::unique_ptr<Mesh> mesh;
            float maxX;
            float minX;
            float maxY;
            float minY;
            float maxZ;
            float minZ;
        };

    public:
        Mesh() = default;
        ~Mesh() = default;

        inline const std::string getShaderName() const { return m_ShaderName; }
        inline void setShaderName(std::string_view name) { m_ShaderName = name; }
        
        void addUbos(const std::vector<UniformBufferObject>& ubos);
        std::function<void(VkCommandBuffer & commandBuffer, VkPipelineLayout pipelineLayout,
            VulkanAdapter* vulkanAdapter, Mesh* mesh)> applyPushConstants = nullptr;
        
        bool isDirty() { return m_IsDirty.load(); }
        void setIsDirty(bool dirty = true) { m_IsDirty.store(dirty); }
        void setHasBbox(bool hasBbox = false) { m_HasBbox = hasBbox; }
        void setHasAnimation(bool hasAnimation = false) { m_HasAnimation = hasAnimation; }
        bool hasBbox() { return m_HasBbox; }
        bool hasAnimation() { return m_HasAnimation; }
        
        void addBBox(BBox* bbox) { m_BoundingBox = std::unique_ptr<BBox>(bbox); }
        BBox* getBBox() { return m_BoundingBox.get(); }

        void setHasPushConstants(bool has = true) { m_HasPushContants = has; }
        bool hasPushConstants() { return m_HasPushContants; }

        inline std::vector<Mesh::Buffer>* getUniformBuffers() { return & m_UniformBuffers; }

        inline glm::vec4 getCameraPos() { return m_CameraPos; }

        void setName(std::string const & name) { m_Name = name; }
        std::string getName() { return m_Name; }

        void setSpritesCount(uint32_t count) { m_SpritesCount = count; }
        uint32_t getNextSpriteIndex();

        Data* getData() { return & m_Data; }
        void setData(Data data) { m_Data = std::move(data); }

        void setMaterial(material_t material) { m_Material = material; }
        material_t getMaterial() { return m_Material; }
        bool isIndexed() { return m_IsIndexed; }
        void setIsIndexed(bool indexed) { m_IsIndexed = indexed; }

        void addStorageBuffer(Buffer buffer) { m_StorageBuffers.emplace_back(buffer); }
        std::vector<Buffer>* getStorageBuffers() { return & m_StorageBuffers; }

        void setHasBufferStorage(bool has = true) { m_HasStorageBuffer = has; }
        bool hasBufferStorage() { return m_HasStorageBuffer; }

        void setIsPointLight(bool is = true) { m_IsPointLight = is; }
        bool isPointLight() { return m_IsPointLight; }

        void setDescSet(VkDescriptorSet descSet) { m_DescSet = descSet; }
        VkDescriptorSet* getDescSet() { return & m_DescSet; }

    private:
        std::unique_ptr<BBox> m_BoundingBox;

        std::string m_ShaderName;
        std::atomic<bool> m_IsDirty{ true };
        bool m_HasBbox = false;
        bool m_HasAnimation = false;

        bool m_HasPushContants = false;
        std::vector<Mesh::Buffer> m_UniformBuffers;
        std::vector<Mesh::Buffer> m_StorageBuffers;
        glm::vec4 m_CameraPos;
        std::string m_Name;

        uint32_t m_SpritesCount = 0;
        uint32_t m_SpritesIndex = 0;
        
        Data m_Data;

        material_t m_Material;
        bool m_IsIndexed{ true };
        bool m_HasStorageBuffer{ false };
        bool m_IsPointLight{ false };

        VkDescriptorSet m_DescSet;
    };
}
