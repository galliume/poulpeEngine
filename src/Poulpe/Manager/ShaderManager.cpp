#include "ShaderManager.hpp"

#include "Poulpe/Core/Tools.hpp"

namespace Poulpe
{
    ShaderManager::ShaderManager()
    {
        m_Shaders = std::make_unique<VulkanShaders>();
    }

    void ShaderManager::addShader(std::string const& name, std::string const& vertPath, std::string const& fragPath)
    {

      if (!std::filesystem::exists(vertPath)) {
        PLP_FATAL("vertex shader file {} does not exits.", vertPath);
        return;
      }

      if (!std::filesystem::exists(fragPath)) {
        PLP_FATAL("fragment shader file {} does not exits.", fragPath);
        return;
      }

      auto vertShaderCode = Tools::readFile(vertPath);
      auto fragShaderCode = Tools::readFile(fragPath);

      VkShaderModule vertexShaderModule = m_Renderer->createShaderModule(vertShaderCode);
      VkShaderModule fragShaderModule = m_Renderer->createShaderModule(fragShaderCode);

      m_Shaders->shaders[name] = { vertexShaderModule, fragShaderModule };

      createGraphicPipeline(name);
    }

    void ShaderManager::clear()
    {
      m_Shaders->shaders.clear();
      m_LoadingDone = false;
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

    template <DescSetLayoutType T>
    VkDescriptorSetLayout ShaderManager::createDescriptorSetLayout()
    {
      std::vector<VkDescriptorSetLayoutBinding> bindings {};

      if constexpr (T == DescSetLayoutType::Entity) {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 7;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding storageLayoutBinding{};
        storageLayoutBinding.binding = 2;
        storageLayoutBinding.descriptorCount = 1;
        storageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        storageLayoutBinding.pImmutableSamplers = nullptr;
        storageLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings = { uboLayoutBinding, samplerLayoutBinding, storageLayoutBinding };
      } else if constexpr (T == DescSetLayoutType::HUD) {
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

        bindings = { uboLayoutBinding, samplerLayoutBinding };
      } else if constexpr (T == DescSetLayoutType::Skybox) {
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

        bindings = { uboLayoutBinding, samplerLayoutBinding };
      } else if constexpr (T == DescSetLayoutType::Offscreen) {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding storageLayoutBinding{};
        storageLayoutBinding.binding = 1;
        storageLayoutBinding.descriptorCount = 1;
        storageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        storageLayoutBinding.pImmutableSamplers = nullptr;
        storageLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings = { uboLayoutBinding, storageLayoutBinding };
      } else {
        PLP_FATAL("unknown descSetLayoutType");
        throw std::runtime_error("unknown descSetLayoutType");
      }

      return m_Renderer->createDescriptorSetLayout(bindings);
    }

    void ShaderManager::createGraphicPipeline(std::string const & shaderName)
    {
      bool offscreen = (shaderName == "shadowMap") ? true : false;
      auto cullMode = VK_CULL_MODE_BACK_BIT;

      std::vector<VkDescriptorPoolSize> poolSizes{};
      VkDescriptorPoolSize dpsUbo;
      dpsUbo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      dpsUbo.descriptorCount = 1000;

      VkDescriptorPoolSize dpsIS;
      dpsIS.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      dpsIS.descriptorCount = 1000;

      poolSizes.emplace_back(dpsUbo);

      if (!offscreen) poolSizes.emplace_back(dpsIS);

      auto shaders = getShadersInfo(shaderName, offscreen);

      VkPipeline graphicPipeline{VK_NULL_HANDLE};
      VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
      VkDescriptorSetLayout descSetLayout{VK_NULL_HANDLE};

      if (shaderName == "skybox") {
        VkPipelineVertexInputStateCreateInfo* vertexInputInfo{nullptr};
        descSetLayout = createDescriptorSetLayout<DescSetLayoutType::Skybox>();
        std::vector<VkDescriptorSetLayout> dSetLayout = { descSetLayout };
        vertexInputInfo = getVertexInputState<VertexBindingType::Vertex3D>();

        std::vector<VkPushConstantRange> vkPcs = {};
        VkPushConstantRange vkPc;
        vkPc.offset = 0;
        vkPc.size = sizeof(constants);
        vkPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        vkPcs.emplace_back(vkPc);

        pipelineLayout = m_Renderer->createPipelineLayout(dSetLayout, vkPcs);

        graphicPipeline = m_Renderer->createGraphicsPipeline(
          pipelineLayout,
          shaderName,
          shaders,
          *vertexInputInfo,
          VK_CULL_MODE_BACK_BIT,
          true, true, true,
          VK_POLYGON_MODE_FILL);

      } else if (shaderName == "grid" || shaderName == "2d") {
        VkPipelineVertexInputStateCreateInfo* vertexInputInfo{nullptr};
        descSetLayout = createDescriptorSetLayout<DescSetLayoutType::HUD>();
        std::vector<VkDescriptorSetLayout> dSetLayout = { descSetLayout };
        vertexInputInfo = getVertexInputState<VertexBindingType::Vertex2D>();

        std::vector<VkPushConstantRange> vkPcs = {};
        VkPushConstantRange vkPc;
        vkPc.offset = 0;
        vkPc.size = sizeof(constants);
        vkPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        vkPcs.emplace_back(vkPc);

        pipelineLayout = m_Renderer->createPipelineLayout(dSetLayout, vkPcs);

        graphicPipeline = m_Renderer->createGraphicsPipeline(
          pipelineLayout,
          shaderName,
          shaders,
          *vertexInputInfo,
          VK_CULL_MODE_BACK_BIT,
          true, true, true,
          VK_POLYGON_MODE_FILL);
      } else {
        VkPipelineVertexInputStateCreateInfo* vertexInputInfo{nullptr};

        bool hasColorAttachment{true};
        bool hasDynamicDepthBias{false};

        VkDescriptorPoolSize dpsSB;
        dpsSB.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        dpsSB.descriptorCount = 10;

        poolSizes.emplace_back(dpsSB);

        std::vector<VkPushConstantRange> vkPcs = {};
        VkPushConstantRange vkPc;
        vkPc.offset = 0;
        vkPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        vkPc.size = sizeof(constants);

        if (shaderName == "shadowMap" || shaderName == "shadowMapSpot") {
          hasColorAttachment = false;
          hasDynamicDepthBias = false;
          cullMode = VK_CULL_MODE_FRONT_BIT;
          descSetLayout = createDescriptorSetLayout<DescSetLayoutType::Offscreen>();
        } else {
          descSetLayout = createDescriptorSetLayout<DescSetLayoutType::Entity>();
        }
        vertexInputInfo = getVertexInputState<VertexBindingType::Vertex3D>();

        vkPcs.emplace_back(vkPc);
        std::vector<VkDescriptorSetLayout> dSetLayout = { descSetLayout };
        pipelineLayout = m_Renderer->createPipelineLayout(dSetLayout, vkPcs);
        
        graphicPipeline = m_Renderer->createGraphicsPipeline(
          pipelineLayout,
          shaderName,
          shaders,
          *vertexInputInfo,
          cullMode,
          true, true, true,
          VK_POLYGON_MODE_FILL,
          hasColorAttachment,
          hasDynamicDepthBias);
      }
    
      auto descriptorPool = m_Renderer->createDescriptorPool(poolSizes, 10000);

      VulkanPipeline pipeline{};
      pipeline.pipeline = graphicPipeline;
      pipeline.pipelineLayout = pipelineLayout;
      pipeline.descPool = descriptorPool;
      pipeline.descSetLayout = descSetLayout;
      pipeline.shaders = shaders;

      if (shaderName == "shadowMap" || shaderName == "shadowMapSpot") {
        pipeline.descSet = m_Renderer->createDescriptorSets(pipeline.descPool, { pipeline.descSetLayout }, 1);
      }
      m_Renderer->addPipeline(shaderName, pipeline);
    }

    std::vector<VkPipelineShaderStageCreateInfo> ShaderManager::getShadersInfo(std::string const & shaderName, bool offscreen)
    {
      std::vector<VkPipelineShaderStageCreateInfo> shadersStageInfos;

      VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
      vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
      vertShaderStageInfo.module = m_Shaders->shaders[shaderName][0];
      vertShaderStageInfo.pName = "main";
      shadersStageInfos.emplace_back(vertShaderStageInfo);

      if (!offscreen) {
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = m_Shaders->shaders[shaderName][1];
        fragShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(fragShaderStageInfo);
      }

      return shadersStageInfos;
    }

    template <VertexBindingType T>
    VkPipelineVertexInputStateCreateInfo* ShaderManager::getVertexInputState()
    {
      VkPipelineVertexInputStateCreateInfo* vertexInputInfo = new VkPipelineVertexInputStateCreateInfo();

      if constexpr (T == VertexBindingType::Vertex3D) {
      std::array<VkVertexInputAttributeDescription, 6>* attDesc = new std::array<VkVertexInputAttributeDescription, 6>(Vertex::getAttributeDescriptions());
      VkVertexInputBindingDescription* bDesc = new VkVertexInputBindingDescription(Vertex::getBindingDescription());

      vertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertexInputInfo->vertexBindingDescriptionCount = 1;
      vertexInputInfo->vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::getAttributeDescriptions().size());
      vertexInputInfo->pVertexBindingDescriptions = bDesc;
      vertexInputInfo->pVertexAttributeDescriptions = attDesc->data();
    } else if constexpr (T == VertexBindingType::Vertex2D) {
      std::array<VkVertexInputAttributeDescription, 3>* attDesc = new std::array<VkVertexInputAttributeDescription, 3>(Vertex2D::getAttributeDescriptions());
      VkVertexInputBindingDescription* bDesc = new VkVertexInputBindingDescription(Vertex2D::getBindingDescription());

      vertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertexInputInfo->vertexBindingDescriptionCount = 1;
      vertexInputInfo->vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex2D::getAttributeDescriptions().size());
      vertexInputInfo->pVertexBindingDescriptions = bDesc;
      vertexInputInfo->pVertexAttributeDescriptions = attDesc->data();
    } else {
      throw std::runtime_error("unknown vertex input state type");
    }

    return vertexInputInfo;
  }
}
