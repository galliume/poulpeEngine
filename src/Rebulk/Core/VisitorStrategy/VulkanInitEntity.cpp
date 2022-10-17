#include "rebulkpch.h"
#include <limits>

#include "VulkanInitEntity.h"

namespace Rbk
{
    struct constants;

    VulkanInitEntity::VulkanInitEntity(
        std::shared_ptr<VulkanAdapter> adapter,
        VkDescriptorPool descriptorPool) :
        m_Adapter(adapter),
        m_DescriptorPool(descriptorPool)
    {

    }

    void VulkanInitEntity::Visit(std::shared_ptr<Entity> entity)
    {
        std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);
        if (!mesh) return;

        uint32_t totalInstances = static_cast<uint32_t>(mesh->GetData()->size());
        uint32_t maxUniformBufferRange = m_Adapter->Rdr()->GetDeviceProperties().limits.maxUniformBufferRange;
        uint32_t uniformBufferChunkSize = maxUniformBufferRange / sizeof(UniformBufferObject);
        uint32_t uniformBuffersCount = static_cast<uint32_t>(std::ceil(static_cast<float>(totalInstances) / static_cast<float>(uniformBufferChunkSize)));
        //mesh->m_CameraPos = m_Camera->GetPos();

         for (uint32_t i = 0; i < uniformBuffersCount; i++) {
            std::pair<VkBuffer, VkDeviceMemory> uniformBuffer = m_Adapter->Rdr()->CreateUniformBuffers(uniformBufferChunkSize);
            mesh->m_UniformBuffers.emplace_back(uniformBuffer);
        }

        std::vector<VkDescriptorImageInfo> imageInfos;

        uint32_t index = 0;
        //a cmd pool per entity ?
        auto commandPool = m_Adapter->Rdr()->CreateCommandPool();

        for (Data& data : *mesh->GetData()) {

            data.m_VertexBuffer = m_Adapter->Rdr()->CreateVertexBuffer(commandPool, data.m_Vertices);
            data.m_IndicesBuffer = m_Adapter->Rdr()->CreateIndexBuffer(commandPool, data.m_Indices);
            data.m_TextureIndex = index;

            Texture tex = m_Adapter->GetTextureManager()->GetTextures()[data.m_Texture];

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = tex.GetImageView();
            imageInfo.sampler = tex.GetSampler();

            imageInfos.emplace_back(imageInfo);

            CreateBBoxEntity(mesh->GetBBox().at(index));

            index++;

        }

        vkDestroyCommandPool(m_Adapter->Rdr()->GetDevice(), commandPool, nullptr);

        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = index;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };

        VkDescriptorSetLayout desriptorSetLayout = m_Adapter->Rdr()->CreateDescriptorSetLayout(
            bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
        );

        m_Adapter->GetDescriptorSetLayouts()->emplace_back(desriptorSetLayout);

        for (uint32_t i = 0; i < m_Adapter->GetSwapChainImages()->size(); i++) {
            VkDescriptorSet meshDescriptorSets = m_Adapter->Rdr()->CreateDescriptorSets(m_DescriptorPool, { desriptorSetLayout }, 1);
            m_Adapter->Rdr()->UpdateDescriptorSets(mesh->m_UniformBuffers, meshDescriptorSets, imageInfos);

            mesh->m_DescriptorSets.emplace_back(meshDescriptorSets);
        }

        std::vector<VkPushConstantRange> pushConstants = {};
        VkPushConstantRange vkPushconstants;
        vkPushconstants.offset = 0;
        vkPushconstants.size = sizeof(constants);
        vkPushconstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        pushConstants.emplace_back(vkPushconstants);
        std::vector<VkDescriptorSetLayout>dSetLayout = { desriptorSetLayout };

        mesh->m_PipelineLayout = m_Adapter->Rdr()->CreatePipelineLayout(mesh->m_DescriptorSets, dSetLayout, pushConstants);

        VkVertexInputBindingDescription bDesc = Vertex::GetBindingDescription();
        std::vector<VkPipelineShaderStageCreateInfo>shadersStageInfos;

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = m_Adapter->GetShaderManager()->GetShaders()->shaders[mesh->GetShaderName()][0];
        vertShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(vertShaderStageInfo);

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = m_Adapter->GetShaderManager()->GetShaders()->shaders[mesh->GetShaderName()][1];
        fragShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(fragShaderStageInfo);

        auto desc = Vertex::GetAttributeDescriptions();
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
        vertexInputInfo.pVertexBindingDescriptions = &bDesc;
        vertexInputInfo.pVertexAttributeDescriptions = desc.data();

        mesh->m_GraphicsPipeline = m_Adapter->Rdr()->CreateGraphicsPipeline(
            m_Adapter->RdrPass(),
            mesh->m_PipelineLayout,
            mesh->GetShaderName(),
            shadersStageInfos,
            vertexInputInfo,
            VK_CULL_MODE_BACK_BIT,
            true, true, true, true,
            VulkanAdapter::s_PolygoneMode
        );
    }

    void VulkanInitEntity::CreateBBoxEntity(Rbk::Entity::BBox box)
    {
        std::shared_ptr<Mesh> bbox = std::make_shared<Mesh>();

        const std::vector<Vertex> vertices = {
            {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},

            {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
        };
        const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0, //front
            1, 5, 6, 6, 2, 1, // right
            7, 6, 5, 5, 4, 7, // back
            4, 0, 3, 3, 7, 4, // left
            4, 5, 1, 1, 0, 4, // bottom
            3, 2, 6, 6, 7, 3 // top
        };


        UniformBufferObject ubo;
        ubo.model = glm::translate(glm::mat4(1.0f), box.center) * glm::scale(glm::mat4(1.0f), box.scale);

        ubo.model = glm::rotate(ubo.model, glm::radians(box.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo.model = glm::rotate(ubo.model, glm::radians(box.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.model = glm::rotate(ubo.model, glm::radians(box.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));


        //glm::vec3 center = box.center;
        //glm::vec3 size = box.size;
        //ubo.model = glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), size);

        //ubo.model = glm::rotate(ubo.model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        //ubo.model = glm::rotate(ubo.model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        //ubo.model = glm::rotate(ubo.model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        ubo.view = glm::mat4(1.0f);

        auto commandPool = m_Adapter->Rdr()->CreateCommandPool();

        Data data;
        data.m_Texture = "minecraft_grass";
        data.m_TextureIndex = 0;
        data.m_VertexBuffer = m_Adapter->Rdr()->CreateVertexBuffer(commandPool, vertices);
        data.m_IndicesBuffer = m_Adapter->Rdr()->CreateIndexBuffer(commandPool, indices);
        data.m_Ubos.emplace_back(ubo);
        data.m_Indices = indices;
        data.m_Vertices = vertices;

        bbox->SetName("bbox");
        bbox->SetShaderName("bbox");
        bbox->GetData()->emplace_back(data);

        uint32_t totalInstances = static_cast<uint32_t>(bbox->GetData()->size());
        uint32_t maxUniformBufferRange = m_Adapter->Rdr()->GetDeviceProperties().limits.maxUniformBufferRange;
        uint32_t uniformBufferChunkSize = maxUniformBufferRange / sizeof(UniformBufferObject);
        uint32_t uniformBuffersCount = static_cast<uint32_t>(std::ceil(static_cast<float>(totalInstances) / static_cast<float>(uniformBufferChunkSize)));
        //mesh->m_CameraPos = m_Camera->GetPos();

        for (uint32_t i = 0; i < uniformBuffersCount; i++) {
            std::pair<VkBuffer, VkDeviceMemory> uniformBuffer = m_Adapter->Rdr()->CreateUniformBuffers(uniformBufferChunkSize);
            bbox->m_UniformBuffers.emplace_back(uniformBuffer);
        }

        Texture tex = m_Adapter->GetTextureManager()->GetTextures()["minecraft_grass"];

        std::vector<VkDescriptorImageInfo>imageInfos{};
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = tex.GetImageView();
        imageInfo.sampler = tex.GetSampler();

        imageInfos.emplace_back(imageInfo);

        vkDestroyCommandPool(m_Adapter->Rdr()->GetDevice(), commandPool, nullptr);

        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = imageInfos.size();
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


        std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };

        VkDescriptorSetLayout desriptorSetLayout = m_Adapter->Rdr()->CreateDescriptorSetLayout(
            bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
        );

        m_Adapter->GetDescriptorSetLayouts()->emplace_back(desriptorSetLayout);

        for (uint32_t i = 0; i < m_Adapter->GetSwapChainImages()->size(); i++) {
            VkDescriptorSet descriptorSet = m_Adapter->Rdr()->CreateDescriptorSets(m_DescriptorPool, { desriptorSetLayout }, 1);
            m_Adapter->Rdr()->UpdateDescriptorSets(bbox->m_UniformBuffers, descriptorSet, imageInfos);
            bbox->m_DescriptorSets.emplace_back(descriptorSet);
        }
        std::vector<VkDescriptorSetLayout>dSetLayout = { desriptorSetLayout };

        bbox->m_PipelineLayout = m_Adapter->Rdr()->CreatePipelineLayout(bbox->m_DescriptorSets, dSetLayout, {});

        std::vector<VkPipelineShaderStageCreateInfo> shadersStageInfos;

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = m_Adapter->GetShaderManager()->GetShaders()->shaders["bbox"][0];
        vertShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(vertShaderStageInfo);

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = m_Adapter->GetShaderManager()->GetShaders()->shaders["bbox"][1];
        fragShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(fragShaderStageInfo);

        VkVertexInputBindingDescription bDesc = Vertex::GetBindingDescription();
        auto gridDesc = Vertex::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
        vertexInputInfo.pVertexBindingDescriptions = &bDesc;
        vertexInputInfo.pVertexAttributeDescriptions = gridDesc.data();

        bbox->m_GraphicsPipeline = m_Adapter->Rdr()->CreateGraphicsPipeline(
            m_Adapter->RdrPass(),
            bbox->m_PipelineLayout,
            bbox->GetShaderName(),
            shadersStageInfos,
            vertexInputInfo,
            VK_CULL_MODE_BACK_BIT,
            true, true, true, true,
            VulkanAdapter::s_PolygoneMode
        );

        m_Adapter->GetEntityManager()->AddBBox(bbox);
    }
}