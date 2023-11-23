#pragma once

#include "Entity.hpp"

#include <functional>

namespace Poulpe
{
    struct constants
    {
        glm::vec4 data;
        glm::vec4 cameraPos;
        glm::vec4 fogColor;
        glm::vec4 lightPos;
        glm::mat4 view;
        float ambiantLight;
        float fogDensity;
    };

    class VulkanAdapter;//@todo should not be here

    class Mesh : public Entity
    {
    public:
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
        Mesh();
        ~Mesh() = default;

        std::vector<Mesh*> init(std::string const & name, std::string const & path,
            std::vector<std::string> const & textureNames, std::string const & shader,
            glm::vec3 const & pos, glm::vec3 const & scale, glm::vec3 rotation,
            bool shouldInverseTextureY);
        
        inline std::vector<VkDescriptorSet> getDescriptorSets() { return m_DescriptorSets; }
        inline VkPipeline getGraphicsPipeline() { return m_GraphicsPipeline; }
        inline VkPipelineLayout getPipelineLayout() { return m_PipelineLayout; }
        inline glm::vec4 getCameraPos() { return m_CameraPos; }
        inline VkDescriptorPool getDescriptorPool() { return m_DescriptorPool; }
        inline VkDescriptorSetLayout getDescriptorSetLayout() { return m_DescriptorSetLayout; }

        inline const std::string getShaderName() const { return m_ShaderName; }
        inline void setShaderName(std::string_view name) { m_ShaderName = name; }
        void setDescriptorSets(std::vector<VkDescriptorSet> descSets) { m_DescriptorSets = descSets; }
        void setDescriptorPool(VkDescriptorPool descPool) { m_DescriptorPool = descPool; }
        void setPipelineLayout(VkPipelineLayout pipelineLayout) { m_PipelineLayout = pipelineLayout; }
        void setDescriptorSetLayout(VkDescriptorSetLayout descLayout) { m_DescriptorSetLayout = descLayout; }
        void setGraphicsPipeline(VkPipeline pipeline) { m_GraphicsPipeline = pipeline; }

        void addUbos(const std::vector<UniformBufferObject>& ubos);
        std::function<void(VkCommandBuffer & commandBuffer, VkPipelineLayout pipelineLayout,
            VulkanAdapter* vulkanAdapter, Data * data)> applyPushConstants = nullptr;
        
        bool isDirty() override { return m_IsDirty.load(); }
        void setIsDirty(bool dirty = true) { m_IsDirty.store(dirty); }
        void setHasBbox(bool hasBbox = false) { m_HasBbox = hasBbox; }
        bool hasBbox() { return m_HasBbox; }
        
        void addBBox(BBox* bbox) { m_BoundingBox = std::unique_ptr<BBox>(bbox); }
        BBox* getBBox() { return m_BoundingBox.get(); }

    private:
        glm::vec4 m_CameraPos;
        std::vector<VkDescriptorSet> m_DescriptorSets;
        VkPipelineLayout m_PipelineLayout;
        VkPipeline m_GraphicsPipeline;
        VkDescriptorSetLayout m_DescriptorSetLayout;
        VkDescriptorPool m_DescriptorPool;
        std::unique_ptr<BBox> m_BoundingBox;

        std::string m_ShaderName;
        std::atomic<bool> m_IsDirty{ true };
        bool m_HasBbox = true;
    };
}
