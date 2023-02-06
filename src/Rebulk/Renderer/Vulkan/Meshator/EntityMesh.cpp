#include "rebulkpch.h"

#include "EntityMesh.h"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.h"

namespace Rbk
{
    struct constants;

    EntityMesh::EntityMesh(
        std::shared_ptr<VulkanAdapter> adapter,
        std::shared_ptr<EntityManager> entityManager,
        std::shared_ptr<ShaderManager> shaderManager,
        std::shared_ptr<TextureManager> textureManager,
        VkDescriptorPool descriptorPool) :
        m_Adapter(adapter),
        m_EntityManager(entityManager),
        m_ShaderManager(shaderManager),
        m_TextureManager(textureManager),
        m_DescriptorPool(descriptorPool)
    {

    }

    void EntityMesh::Visit(std::shared_ptr<Entity> entity)
    {

        std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);
        if (!mesh && !mesh->IsDirty()) return;

        uint32_t totalInstances = static_cast<uint32_t>(mesh->GetData()->m_Ubos.size());
        uint32_t maxUniformBufferRange = m_Adapter->Rdr()->GetDeviceProperties().limits.maxUniformBufferRange;
        uint32_t uniformBufferChunkSize = maxUniformBufferRange / sizeof(UniformBufferObject);
        uint32_t uniformBuffersCount = static_cast<uint32_t>(std::ceil(static_cast<float>(totalInstances) / static_cast<float>(uniformBufferChunkSize)));

        //@todo fix memory management...
        uint32_t uboOffset = (totalInstances > uniformBufferChunkSize) ? uniformBufferChunkSize : totalInstances;
        uint32_t uboRemaining = (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0;
        uint32_t nbUbo = uboOffset;

         for (uint32_t i = 0; i < uniformBuffersCount; i++) {

            mesh->GetData()->m_UbosOffset.emplace_back(uboOffset);
            Buffer uniformBuffer = m_Adapter->Rdr()->CreateUniformBuffers(nbUbo);
            mesh->m_UniformBuffers.emplace_back(uniformBuffer);

            uboOffset = (uboRemaining > uniformBufferChunkSize) ? uboOffset + uniformBufferChunkSize : uboOffset + uboRemaining;
            nbUbo = (uboRemaining > uniformBufferChunkSize) ? uniformBufferChunkSize : uboRemaining;
            uboRemaining = (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0;
        }

        std::vector<VkDescriptorImageInfo> imageInfos;

        uint32_t index = 0;
        auto commandPool = m_Adapter->Rdr()->CreateCommandPool();
        auto data = mesh->GetData();

        data->m_VertexBuffer = m_Adapter->Rdr()->CreateVertexBuffer(commandPool, data->m_Vertices);
        data->m_IndicesBuffer = m_Adapter->Rdr()->CreateIndexBuffer(commandPool, data->m_Indices);
        data->m_TextureIndex = index;

        Texture tex = m_TextureManager->GetTextures()[data->m_Texture];

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = tex.GetImageView();
        imageInfo.sampler = tex.GetSampler();

        imageInfos.emplace_back(imageInfo);
        index++;

        for (uint32_t i = 0; i < data->m_Ubos.size(); i++) {
            //data.m_Ubos[i].view = m_Adapter->GetCamera()->LookAt();
            data->m_Ubos[i].proj = m_Adapter->GetPerspective();
        }

        auto min = 0;
        auto max = 0;

        for (uint32_t i = 0; i < mesh->m_UniformBuffers.size(); i++) {
            max = mesh->GetData()->m_UbosOffset.at(i);
            auto ubos = std::vector<UniformBufferObject>(mesh->GetData()->m_Ubos.begin() + min, mesh->GetData()->m_Ubos.begin() + max);

            m_Adapter->Rdr()->UpdateUniformBuffer(
                mesh->m_UniformBuffers[i],
                ubos,
                ubos.size()
            );

            min = max;
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

        for (auto ubo : mesh->m_UniformBuffers) {
            VkDescriptorSet meshDescriptorSets = m_Adapter->Rdr()->CreateDescriptorSets(m_DescriptorPool, { desriptorSetLayout }, 1);
            m_Adapter->Rdr()->UpdateDescriptorSets({ ubo }, meshDescriptorSets, imageInfos);
            for (uint32_t i = 0; i < m_Adapter->GetSwapChainImages()->size(); i++) {
                mesh->m_DescriptorSets.emplace_back(meshDescriptorSets);
            }
        }

        constants pushConstants;
        pushConstants.data = glm::vec4(0.f, Rbk::VulkanAdapter::s_AmbiantLight.load(), Rbk::VulkanAdapter::s_FogDensity.load(), 0.f);
        pushConstants.cameraPos = m_Adapter->GetCamera()->GetPos();
        pushConstants.fogColor = glm::vec4({ Rbk::VulkanAdapter::s_FogColor[0].load(), Rbk::VulkanAdapter::s_FogColor[1].load(), Rbk::VulkanAdapter::s_FogColor[2].load(), 0.f });
        pushConstants.lightPos = glm::vec4(m_Adapter->GetLights().at(0), 0.f);
        pushConstants.view = m_Adapter->GetCamera()->LookAt();
        pushConstants.ambiantLight = Rbk::VulkanAdapter::s_AmbiantLight.load();
        pushConstants.fogDensity = Rbk::VulkanAdapter::s_FogDensity.load();

        mesh->ApplyPushConstants = [=, &pushConstants, &mesh](VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, std::shared_ptr<VulkanAdapter> adapter, const Data& data) {
            pushConstants.data = glm::vec4(static_cast<float>(data.m_TextureIndex), Rbk::VulkanAdapter::s_AmbiantLight.load(), Rbk::VulkanAdapter::s_FogDensity.load(), 0.f);
            pushConstants.cameraPos = adapter->GetCamera()->GetPos();
            pushConstants.fogColor = glm::vec4({ Rbk::VulkanAdapter::s_FogColor[0].load(), Rbk::VulkanAdapter::s_FogColor[1].load(), Rbk::VulkanAdapter::s_FogColor[2].load(), 0.f });
            pushConstants.lightPos = glm::vec4(adapter->GetLights().at(0), 0.f);
            pushConstants.view = adapter->GetCamera()->LookAt();
            pushConstants.ambiantLight = Rbk::VulkanAdapter::s_AmbiantLight.load();
            pushConstants.fogDensity = Rbk::VulkanAdapter::s_FogDensity.load();
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants), &pushConstants);
        };

        mesh->SetHasPushConstants();

        std::vector<VkDescriptorSetLayout>dSetLayout = { desriptorSetLayout };

        std::vector<VkPushConstantRange> vkPcs = {};
        VkPushConstantRange vkPc;
        vkPc.offset = 0;
        vkPc.size = sizeof(constants);
        vkPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        vkPcs.emplace_back(vkPc);
        mesh->m_PipelineLayout = m_Adapter->Rdr()->CreatePipelineLayout(mesh->m_DescriptorSets, dSetLayout, vkPcs);

        VkVertexInputBindingDescription bDesc = Vertex::GetBindingDescription();
        std::vector<VkPipelineShaderStageCreateInfo>shadersStageInfos;

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = m_ShaderManager->GetShaders()->shaders[mesh->GetShaderName()][0];
        vertShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(vertShaderStageInfo);

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = m_ShaderManager->GetShaders()->shaders[mesh->GetShaderName()][1];
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

        if (m_EntityManager->ShowBBox()) {
            CreateBBoxEntity(mesh);
        }
    }

    void EntityMesh::CreateBBoxEntity(std::shared_ptr<Mesh>& mesh)
    {
        mesh->GetBBox()->mesh = std::make_shared<Mesh>();
        Rbk::Entity::BBox* box = mesh->GetBBox().get();

        UniformBufferObject ubo;
        glm::mat4 transform = glm::translate(glm::mat4(1), box->center) * glm::scale(glm::mat4(1), box->size);
        ubo.model = box->position * transform;

        auto commandPool = m_Adapter->Rdr()->CreateCommandPool();

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

        Data data;
        data.m_Texture = "minecraft_grass";
        data.m_TextureIndex = 0;
        data.m_VertexBuffer = m_Adapter->Rdr()->CreateVertexBuffer(commandPool, vertices);
        data.m_IndicesBuffer = m_Adapter->Rdr()->CreateIndexBuffer(commandPool, indices);
        data.m_Indices = indices;
        data.m_Vertices = vertices;
        data.m_Ubos.emplace_back(ubo);

        for (uint32_t i = 0; i < data.m_Ubos.size(); i++) {
            data.m_Ubos[i].proj = m_Adapter->GetPerspective();
        }

        box->mesh->SetName("bbox_" + mesh->GetData()->m_Name);
        box->mesh->SetShaderName("bbox");
        box->mesh->SetData(data);

        uint32_t totalInstances = static_cast<uint32_t>(mesh->GetData()->m_Ubos.size());
        uint32_t maxUniformBufferRange = m_Adapter->Rdr()->GetDeviceProperties().limits.maxUniformBufferRange;
        uint32_t uniformBufferChunkSize = maxUniformBufferRange / sizeof(UniformBufferObject);
        uint32_t uniformBuffersCount = static_cast<uint32_t>(std::ceil(static_cast<float>(totalInstances) / static_cast<float>(uniformBufferChunkSize)));

        //@todo fix memory management...
        uint32_t uboOffset = (totalInstances > uniformBufferChunkSize) ? uniformBufferChunkSize : totalInstances;
        uint32_t uboRemaining = (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0;
        uint32_t nbUbo = uboOffset;

        for (uint32_t i = 0; i < uniformBuffersCount; i++) {

            box->mesh->GetData()->m_UbosOffset.emplace_back(uboOffset);
            Buffer uniformBuffer = m_Adapter->Rdr()->CreateUniformBuffers(nbUbo);
            box->mesh->m_UniformBuffers.emplace_back(uniformBuffer);

            uboOffset = (uboRemaining > uniformBufferChunkSize) ? uboOffset + uniformBufferChunkSize : uboOffset + uboRemaining;
            nbUbo = (uboRemaining > uniformBufferChunkSize) ? uniformBufferChunkSize : uboRemaining;
            uboRemaining = (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0;
        }

        Texture tex = m_TextureManager->GetTextures()["minecraft_grass"];

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

        for (auto ubo : box->mesh->m_UniformBuffers) {
            VkDescriptorSet meshDescriptorSets = m_Adapter->Rdr()->CreateDescriptorSets(m_DescriptorPool, { desriptorSetLayout }, 1);
            m_Adapter->Rdr()->UpdateDescriptorSets({ ubo }, meshDescriptorSets, imageInfos);
            for (uint32_t i = 0; i < m_Adapter->GetSwapChainImages()->size(); i++) {
                box->mesh->m_DescriptorSets.emplace_back(meshDescriptorSets);
            }
        }

        constants pushConstants;
        pushConstants.data = glm::vec4(0.f, Rbk::VulkanAdapter::s_AmbiantLight.load(), Rbk::VulkanAdapter::s_FogDensity.load(), 0.f);
        pushConstants.cameraPos = m_Adapter->GetCamera()->GetPos();
        pushConstants.fogColor = glm::vec4({ Rbk::VulkanAdapter::s_FogColor[0].load(), Rbk::VulkanAdapter::s_FogColor[1].load(), Rbk::VulkanAdapter::s_FogColor[2].load(), 0.f });
        pushConstants.lightPos = glm::vec4(m_Adapter->GetLights().at(0), 0.f);
        pushConstants.view = m_Adapter->GetCamera()->LookAt();
        pushConstants.ambiantLight = Rbk::VulkanAdapter::s_AmbiantLight.load();
        pushConstants.fogDensity = Rbk::VulkanAdapter::s_FogDensity.load();

        box->mesh->ApplyPushConstants = [=, &pushConstants, &mesh](VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, std::shared_ptr<VulkanAdapter> adapter, const Data& data) {
            pushConstants.data = glm::vec4(static_cast<float>(data.m_TextureIndex), Rbk::VulkanAdapter::s_AmbiantLight.load(), Rbk::VulkanAdapter::s_FogDensity.load(), 0.f);
            pushConstants.cameraPos = adapter->GetCamera()->GetPos();
            pushConstants.fogColor = glm::vec4({ Rbk::VulkanAdapter::s_FogColor[0].load(), Rbk::VulkanAdapter::s_FogColor[1].load(), Rbk::VulkanAdapter::s_FogColor[2].load(), 0.f });
            pushConstants.lightPos = glm::vec4(adapter->GetLights().at(0), 0.f);
            pushConstants.view = adapter->GetCamera()->LookAt();
            pushConstants.ambiantLight = Rbk::VulkanAdapter::s_AmbiantLight.load();
            pushConstants.fogDensity = Rbk::VulkanAdapter::s_FogDensity.load();
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants), &pushConstants);
        };

        box->mesh->SetHasPushConstants();

        std::vector<VkDescriptorSetLayout>dSetLayout = { desriptorSetLayout };

        std::vector<VkPushConstantRange> vkPcs = {};
        VkPushConstantRange vkPc;
        vkPc.offset = 0;
        vkPc.size = sizeof(constants);
        vkPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        vkPcs.emplace_back(vkPc);

        box->mesh->m_PipelineLayout = m_Adapter->Rdr()->CreatePipelineLayout(box->mesh->m_DescriptorSets, dSetLayout, vkPcs);

        std::vector<VkPipelineShaderStageCreateInfo> shadersStageInfos;

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = m_ShaderManager->GetShaders()->shaders["bbox"][0];
        vertShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(vertShaderStageInfo);

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = m_ShaderManager->GetShaders()->shaders["bbox"][1];
        fragShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(fragShaderStageInfo);

        VkVertexInputBindingDescription bDesc = Vertex::GetBindingDescription();
        auto desc = Vertex::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
        vertexInputInfo.pVertexBindingDescriptions = &bDesc;
        vertexInputInfo.pVertexAttributeDescriptions = desc.data();

        box->mesh->m_GraphicsPipeline = m_Adapter->Rdr()->CreateGraphicsPipeline(
            m_Adapter->RdrPass(),
            box->mesh->m_PipelineLayout,
            box->mesh->GetShaderName(),
            shadersStageInfos,
            vertexInputInfo,
            VK_CULL_MODE_BACK_BIT,
            true, true, true, true,
            VK_POLYGON_MODE_LINE
        );

        for (uint32_t i = 0; i < box->mesh->m_UniformBuffers.size(); i++) {
            m_Adapter->Rdr()->UpdateUniformBuffer(
                box->mesh->m_UniformBuffers[i],
                box->mesh->GetData()->m_Ubos,
                box->mesh->GetData()->m_Ubos.size()
            );
        }
    }
}
