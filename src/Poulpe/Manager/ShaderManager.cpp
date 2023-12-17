#include "ShaderManager.hpp"

#include "Poulpe/Core/Tools.hpp"

namespace Poulpe
{
    ShaderManager::ShaderManager()
    {
        m_Shaders = std::make_unique<VulkanShaders>();
    }

    std::function<void()> ShaderManager::load(nlohmann::json config)
    {
        m_Config = config;

        std::function shaderFuture = [this]() {
            for (auto & shader : m_Config["shader"].items()) {

                auto key = static_cast<std::string>(shader.key());
                auto data = shader.value();

                addShader(key, data["vert"], data["frag"]);
            }

            m_LoadingDone = true;
        };

        return shaderFuture;
    }

    void ShaderManager::addShader(std::string const & name, std::string const & vertPath, std::string const & fragPath)
    {

        if (!std::filesystem::exists(vertPath)) {
            PLP_FATAL("vertex shader file {} does not exits.", vertPath);
            return;
        }

        if (!std::filesystem::exists(fragPath)) {
            PLP_FATAL("fragment shader file {} does not exits.", fragPath);
            return;
        }

        auto vertShaderCode = Poulpe::Tools::readFile(vertPath);
        auto fragShaderCode = Poulpe::Tools::readFile(fragPath);
    
        VkShaderModule vertexShaderModule = m_Renderer->rdr()->createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = m_Renderer->rdr()->createShaderModule(fragShaderCode);

        std::array<VkShaderModule, 2> module = { vertexShaderModule, fragShaderModule };
        
        m_Shaders->shaders[name] = module;

        createGraphicPipeline(name);
    }

    void ShaderManager::createGraphicPipeline(std::string const & shaderName)
    {
        std::vector<VkDescriptorPoolSize> poolSizes{};
        VkDescriptorPoolSize cp1;
        cp1.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        cp1.descriptorCount = 10;

        VkDescriptorPoolSize cp2;
        cp2.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cp2.descriptorCount = 10000;
        poolSizes.emplace_back(cp1);
        poolSizes.emplace_back(cp2);

        poolSizes.emplace_back(cp1);
        poolSizes.emplace_back(cp2);

        VkDescriptorPoolSize cp3;
        cp3.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        cp3.descriptorCount = 10;

        VkDescriptorSetLayout descSetLayout;

        auto descriptorPool = m_Renderer->rdr()->createDescriptorPool(poolSizes, 10000);

        if (shaderName == "skybox") {
          descSetLayout = createDescriptorSetLayoutForSkybox();
        } else if (shaderName == "grid" || shaderName == "2d") {
          descSetLayout = createDescriptorSetLayoutForHUD();
        } else {
          descSetLayout = createDescriptorSetLayout();
        }

        std::vector<VkDescriptorSetLayout> dSetLayout = { descSetLayout };

        std::vector<VkPushConstantRange> vkPcs = {};
        VkPushConstantRange vkPc;
        vkPc.offset = 0;
        vkPc.size = sizeof(constants);
        vkPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        vkPcs.emplace_back(vkPc);
        VkPipelineLayout pipelineLayout = m_Renderer->rdr()->createPipelineLayout(dSetLayout, vkPcs);

        auto shaders = getShadersInfo(shaderName);
        auto bDesc = Vertex::GetBindingDescription();
        auto attDesc = Vertex::GetAttributeDescriptions();
        auto vertexInputInfo = getVertexBindingDesc(bDesc, attDesc);
        VkPipeline graphicPipeline = VK_NULL_HANDLE;

        //@todo clean
        if (shaderName == "shadowMap" || shaderName == "shadowMapSpot" || shaderName == "quad") {
            graphicPipeline = m_Renderer->rdr()->createGraphicsPipeline(m_Renderer->rdrPass(), pipelineLayout,
            shaderName, shaders, vertexInputInfo, VK_CULL_MODE_NONE, true, true, true, true, VK_POLYGON_MODE_FILL, false, true);
        } else {
          graphicPipeline = m_Renderer->rdr()->createGraphicsPipeline(m_Renderer->rdrPass(), pipelineLayout,
            shaderName, shaders, vertexInputInfo, VK_CULL_MODE_BACK_BIT, true, true, true, true, VK_POLYGON_MODE_FILL);
        }

        VulkanPipeline pipeline{};
        pipeline.pipeline = graphicPipeline;
        pipeline.pipelineLayout = pipelineLayout;
        pipeline.descPool = descriptorPool;
        pipeline.descSetLayout = descSetLayout;
        pipeline.shaders = shaders;

        m_Renderer->addPipeline(shaderName, pipeline);

        //if (shaderName == "shadowMap") m_Renderer->prepareShadowMap();
    }

    VkDescriptorSetLayout ShaderManager::createDescriptorSetLayoutForSkybox()
    {
      VkDescriptorSetLayoutBinding uboLayoutBinding{};
      uboLayoutBinding.binding = 0;
      uboLayoutBinding.descriptorCount = 1;
      uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      uboLayoutBinding.pImmutableSamplers = nullptr;
      uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

      VkDescriptorSetLayoutBinding samplerLayoutBinding{};
      samplerLayoutBinding.binding = 1;
      samplerLayoutBinding.descriptorCount = 1;
      samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      samplerLayoutBinding.pImmutableSamplers = nullptr;
      samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      std::vector<VkDescriptorSetLayoutBinding> bindings = {
          uboLayoutBinding, samplerLayoutBinding };

      VkDescriptorSetLayout desriptorSetLayout = m_Renderer->rdr()->createDescriptorSetLayout(bindings);

      return desriptorSetLayout;
    }

    VkDescriptorSetLayout ShaderManager::createDescriptorSetLayoutForHUD()
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

      std::vector<VkDescriptorSetLayoutBinding> bindings = {
          uboLayoutBinding, samplerLayoutBinding };

      VkDescriptorSetLayout desriptorSetLayout = m_Renderer->rdr()->createDescriptorSetLayout(bindings);

      return desriptorSetLayout;
    }

    VkDescriptorSetLayout ShaderManager::createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 5;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding storageLayoutBinding{};
        storageLayoutBinding.binding = 2;
        storageLayoutBinding.descriptorCount = 1;
        storageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        storageLayoutBinding.pImmutableSamplers = nullptr;
        storageLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            uboLayoutBinding, samplerLayoutBinding, storageLayoutBinding };

      VkDescriptorSetLayout desriptorSetLayout = m_Renderer->rdr()->createDescriptorSetLayout(bindings);

      return desriptorSetLayout;
    }

    std::vector<VkPipelineShaderStageCreateInfo> ShaderManager::getShadersInfo(std::string const & shaderName)
    {
      std::vector<VkPipelineShaderStageCreateInfo> shadersStageInfos;

      VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
      vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
      vertShaderStageInfo.module = m_Shaders->shaders[shaderName][0];
      vertShaderStageInfo.pName = "main";
      shadersStageInfos.emplace_back(vertShaderStageInfo);

      VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
      fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      fragShaderStageInfo.module = m_Shaders->shaders[shaderName][1];
      fragShaderStageInfo.pName = "main";
      shadersStageInfos.emplace_back(fragShaderStageInfo);

      return shadersStageInfos;
    }

    VkPipelineVertexInputStateCreateInfo ShaderManager::getVertexBindingDesc(VkVertexInputBindingDescription bDesc,
      std::array<VkVertexInputAttributeDescription, 5> attDesc)
    {
      VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
      vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertexInputInfo.vertexBindingDescriptionCount = 1;
      vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
      vertexInputInfo.pVertexBindingDescriptions = & bDesc;
      vertexInputInfo.pVertexAttributeDescriptions = attDesc.data();

      return vertexInputInfo;

    }
    void ShaderManager::clear()
    {
        m_Shaders->shaders.clear();
        m_LoadingDone = false;
    }
}
