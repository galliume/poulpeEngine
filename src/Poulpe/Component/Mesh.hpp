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
      std::string ambientTexname;             // map_Ka
      std::string diffuseTexname;             // map_Kd
      std::string specularTexname;            // map_Ks
      std::string specularHighlightTexname;  // map_Ns
      std::string bumpTexname;                // map_bump, map_Bump, bump
    };

    struct constants
    {
        int textureID;
        glm::mat4 view;
        glm::vec4 viewPos;
        glm::vec3 ambient;
        glm::vec3 ambientLight;
        glm::vec3 lightDir;
        glm::vec3 diffuseLight;
        glm::vec3 specular;
        glm::vec3 specularLight;
        float shininess;
        glm::vec3 mapsUsed{ 0, 0, 0 };
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
            glm::vec3 mapsUsed{ 0, 0, 0 };
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

        inline std::vector<VkDescriptorSet> getDescriptorSets() { return m_DescriptorSets; }
        inline VkPipeline getGraphicsPipeline(unsigned int index = 0) { return m_GraphicsPipeline.at(index); }
        inline size_t getGraphicsPipelineSize() { return m_GraphicsPipeline.size(); }
        inline VkPipelineLayout getPipelineLayout() { return m_PipelineLayout; }
        inline VkDescriptorPool getDescriptorPool() { return m_DescriptorPool; }
        inline VkDescriptorSetLayout getDescriptorSetLayout() { return m_DescriptorSetLayout; }

        inline const std::string getShaderName() const { return m_ShaderName; }
        inline void setShaderName(std::string_view name) { m_ShaderName = name; }
        void setDescriptorSets(std::vector<VkDescriptorSet> descSets) { m_DescriptorSets = descSets; }
        void setDescriptorPool(VkDescriptorPool descPool) { m_DescriptorPool = descPool; }
        void setPipelineLayout(VkPipelineLayout pipelineLayout) { m_PipelineLayout = pipelineLayout; }
        void setDescriptorSetLayout(VkDescriptorSetLayout descLayout) { m_DescriptorSetLayout = descLayout; }
        void setGraphicsPipeline(VkPipeline pipeline) { m_GraphicsPipeline.emplace_back(pipeline); }

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

    private:
        std::vector<VkDescriptorSet> m_DescriptorSets;
        VkPipelineLayout m_PipelineLayout;
        std::vector<VkPipeline> m_GraphicsPipeline;
        VkDescriptorSetLayout m_DescriptorSetLayout;
        VkDescriptorPool m_DescriptorPool;
        std::unique_ptr<BBox> m_BoundingBox;

        std::string m_ShaderName;
        std::atomic<bool> m_IsDirty{ true };
        bool m_HasBbox = false;
        bool m_HasAnimation = false;

        bool m_HasPushContants = false;
        std::vector<Mesh::Buffer> m_UniformBuffers;
        glm::vec4 m_CameraPos;
        std::string m_Name;

        uint32_t m_SpritesCount = 0;
        uint32_t m_SpritesIndex = 0;
        
        Data m_Data;

        material_t m_Material;
    };
}
