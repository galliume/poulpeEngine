#include "Grid.hpp"

#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    struct cPC;

    Grid::Grid(VulkanAdapter* adapter, ShaderManager* shaderManager, TextureManager* textureManager) :
        m_Adapter(adapter),
        m_ShaderManager(shaderManager),
        m_TextureManager(textureManager)
    {
        std::vector<VkDescriptorPoolSize> poolSizes{};
        VkDescriptorPoolSize cp1;
        cp1.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        cp1.descriptorCount = 10;
        VkDescriptorPoolSize cp2;
        cp2.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cp2.descriptorCount = 10;
        poolSizes.emplace_back(cp1);
        poolSizes.emplace_back(cp2);

        m_DescriptorPool = adapter->rdr()->createDescriptorPool(poolSizes, 10);
    }

    void Grid::visit(Mesh* mesh)
    {
        if (!mesh && !mesh->isDirty()) return;

        std::vector<Vertex> const vertices = {
            {{-1.f, -1.f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.f, -1.f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-1.f, 1.f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
        };
        std::vector<uint32_t> const indices = {
            0, 1, 2, 2, 3, 0
        };

        auto commandPool = m_Adapter->rdr()->createCommandPool();

        UniformBufferObject ubo{};

        Mesh::Data gridData;
        gridData.m_Texture = "grid";
        gridData.m_TextureIndex = 0;
        gridData.m_VertexBuffer = m_Adapter->rdr()->createVertexBuffer(commandPool, vertices);
        gridData.m_IndicesBuffer = m_Adapter->rdr()->createIndexBuffer(commandPool, indices);
        gridData.m_Ubos.emplace_back(ubo);
        gridData.m_Indices = indices;
        gridData.m_Vertices = vertices;

        vkDestroyCommandPool(m_Adapter->rdr()->getDevice(), commandPool, nullptr);

        mesh->setName("grid");
        mesh->setShaderName("grid");
        mesh->getUniformBuffers()->emplace_back(m_Adapter->rdr()->createUniformBuffers(1));
        mesh->setDescriptorSetLayout(createDescriptorSetLayout());
        mesh->setDescriptorSets(createDescriptorSet(mesh));
        mesh->setPipelineLayout(createPipelineLayout(mesh->getDescriptorSetLayout()));

        setPushConstants(mesh);

        m_Adapter->getDescriptorSetLayouts()->emplace_back(mesh->getDescriptorSetLayout());

        auto shaders = getShaders(mesh->getShaderName());
        auto bDesc = Vertex::GetBindingDescription();
        auto attrDesc = Vertex::GetAttributeDescriptions();
        auto vertexInputInfo = getVertexBindingDesc(bDesc, attrDesc);

        mesh->setGraphicsPipeline(m_Adapter->rdr()->createGraphicsPipeline(m_Adapter->rdrPass(), mesh->getPipelineLayout(),
            mesh->getShaderName(), shaders, vertexInputInfo, VK_CULL_MODE_NONE, true, true, true, true,
            VulkanAdapter::s_PolygoneMode));

        for (uint32_t i = 0; i < mesh->getUniformBuffers()->size(); i++) {
            //gridData.m_Ubos[i].view = m_Adapter->GetCamera()->LookAt();
            gridData.m_Ubos[i].proj = m_Adapter->getPerspective();

            m_Adapter->rdr()->updateUniformBuffer(mesh->getUniformBuffers()->at(i), & gridData.m_Ubos);
        }
        mesh->setData(gridData);
        mesh->setIsDirty(false);
    }

    VkDescriptorSetLayout Grid::createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 2;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };

         VkDescriptorSetLayout desriptorSetLayout = m_Adapter->rdr()->createDescriptorSetLayout(bindings);

         return desriptorSetLayout;
    }

    std::vector<VkDescriptorSet> Grid::createDescriptorSet(Mesh* mesh)
    {
        Texture ctex = m_TextureManager->getTextures()["minecraft_grass"];

        std::vector<VkDescriptorImageInfo> imageInfos{};
        VkDescriptorImageInfo cimageInfo{};
        cimageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        cimageInfo.imageView = ctex.getImageView();
        cimageInfo.sampler = ctex.getSampler();

        imageInfos.emplace_back(cimageInfo);

        VkDescriptorSetLayoutBinding cuboLayoutBinding{};
        cuboLayoutBinding.binding = 0;
        cuboLayoutBinding.descriptorCount = 1;
        cuboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        cuboLayoutBinding.pImmutableSamplers = nullptr;
        cuboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding csamplerLayoutBinding{};
        csamplerLayoutBinding.binding = 1;
        csamplerLayoutBinding.descriptorCount = imageInfos.size();
        csamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        csamplerLayoutBinding.pImmutableSamplers = nullptr;
        csamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkDescriptorSetLayoutBinding> cbindings = { cuboLayoutBinding, csamplerLayoutBinding };

        VkDescriptorSet descSet = m_Adapter->rdr()->createDescriptorSets(m_DescriptorPool, { mesh->getDescriptorSetLayout() }, 1);
        m_Adapter->rdr()->updateDescriptorSets(*mesh->getUniformBuffers(), descSet, imageInfos);

        return { descSet };
    }

    VkPipelineLayout Grid::createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout)
    {
        std::vector<VkDescriptorSetLayout> dSetLayout = { descriptorSetLayout };

        std::vector<VkPushConstantRange> vkPcs = {};
        VkPushConstantRange vkPc;
        vkPc.offset = 0;
        vkPc.size = sizeof(Grid::pc);
        vkPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        vkPcs.emplace_back(vkPc);

         VkPipelineLayout pipelineLayout = m_Adapter->rdr()->createPipelineLayout(dSetLayout, vkPcs);

        return pipelineLayout;
    }

    std::vector<VkPipelineShaderStageCreateInfo> Grid::getShaders(std::string const & name)
    {
        std::vector<VkPipelineShaderStageCreateInfo> shadersStageInfos;

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = m_ShaderManager->getShaders()->shaders[name][0];
        vertShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(vertShaderStageInfo);

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = m_ShaderManager->getShaders()->shaders[name][1];
        fragShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(fragShaderStageInfo);

        return shadersStageInfos;
    }

    VkPipelineVertexInputStateCreateInfo Grid::getVertexBindingDesc(VkVertexInputBindingDescription bDesc,
        std::array<VkVertexInputAttributeDescription, 3> attDesc)
    {
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
        vertexInputInfo.pVertexBindingDescriptions = & bDesc;
        vertexInputInfo.pVertexAttributeDescriptions = attDesc.data();

        return vertexInputInfo;
    }

    void Grid::setPushConstants(Mesh* mesh)
    {
        Grid::pc pc;
        pc.point = glm::vec4(0.1f, 50.f, 0.f, 0.f);
        pc.view = m_Adapter->getCamera()->lookAt();

        mesh->applyPushConstants = [&pc](VkCommandBuffer & commandBuffer, VkPipelineLayout pipelineLayout,
            VulkanAdapter* adapter, [[maybe_unused]] Mesh::Data * data) {
            pc.point = glm::vec4(0.1f, 50.f, 0.f, 0.f);
            pc.view = adapter->getCamera()->lookAt();
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Grid::pc), & pc);
        };
        mesh->setHasPushConstants();
    }
}
