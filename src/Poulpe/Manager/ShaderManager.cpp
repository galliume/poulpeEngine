#include "ShaderManager.hpp"

#include "Poulpe/Core/Log.hpp"
#include "Poulpe/Core/Tools.hpp"

#include "Poulpe/Component/Vertex2D.hpp"

namespace Poulpe
{
  ShaderManager::ShaderManager()
  {
    _shaders = std::make_unique<VulkanShaders>();
  }

  void ShaderManager::addShader(
    std::string const& name,
    std::string const& vert_path,
    std::string const& frag_path,
    std::string const& geom_path)
  {

    if (!std::filesystem::exists(vert_path)) {
      PLP_FATAL("vertex shader file {} does not exits.", vert_path);
      return;
    }

    if (!std::filesystem::exists(frag_path)) {
      PLP_FATAL("fragment shader file {} does not exits.", frag_path);
      return;
    }

    auto vert_shader = Tools::readFile(vert_path);
    auto frag_shader = Tools::readFile(frag_path);

    VkShaderModule vertex_module = _renderer->getAPI()->createShaderModule(vert_shader);
    VkShaderModule frag_module = _renderer->getAPI()->createShaderModule(frag_shader);

    std::vector<VkShaderModule> shaders{vertex_module, frag_module};

    if (!geom_path.empty() && std::filesystem::exists(geom_path)) {
      auto geom_shader = Tools::readFile(geom_path);

      VkShaderModule geom_module = _renderer->getAPI()->createShaderModule(geom_shader);
      shaders.emplace_back(geom_module);
    } else {
      PLP_WARN("geometry shader file {} does not exits.", geom_path);
     }

    _shaders->shaders[name] = shaders;

    createGraphicPipeline(name);
  }

  void ShaderManager::clear()
  {
    _shaders->shaders.clear();
  }

  std::function<void(std::latch& count_down)> ShaderManager::load(nlohmann::json config)
  {
    _config = config;

    return [this](std::latch& count_down) {
      for (auto & shader : _config["shader"].items()) {

        auto key = static_cast<std::string>(shader.key());
        auto data = shader.value();

        addShader(key, data["vert"], data["frag"], data["geom"]);
      }
      count_down.count_down();
    };
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
      uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;

      VkDescriptorSetLayoutBinding samplerLayoutBinding{};
      samplerLayoutBinding.binding = 1;
      samplerLayoutBinding.descriptorCount = 8;
      samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      samplerLayoutBinding.pImmutableSamplers = nullptr;
      samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding storageLayoutBinding{};
      storageLayoutBinding.binding = 2;
      storageLayoutBinding.descriptorCount = 1;
      storageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      storageLayoutBinding.pImmutableSamplers = nullptr;
      storageLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding depthMapsamplerLayoutBinding{};
      depthMapsamplerLayoutBinding.binding = 3;
      depthMapsamplerLayoutBinding.descriptorCount = 1;
      depthMapsamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      depthMapsamplerLayoutBinding.pImmutableSamplers = nullptr;
      depthMapsamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      bindings = { uboLayoutBinding, samplerLayoutBinding, storageLayoutBinding, depthMapsamplerLayoutBinding };
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

    return _renderer->getAPI()->createDescriptorSetLayout(bindings);
  }

  void ShaderManager::createGraphicPipeline(std::string const & shaderName)
  {
    bool offscreen = (shaderName == "shadowMap") ? true : false;
    auto cullMode = VK_CULL_MODE_BACK_BIT;
    bool has_dynamic_culling{ false };

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
    VkPipeline graphicPipelineBis{VK_NULL_HANDLE};
    VkPipelineLayout pipeline_layout{VK_NULL_HANDLE};
    VkDescriptorSetLayout descset_layout{VK_NULL_HANDLE};

    if (shaderName == "skybox") {
      VkPipelineVertexInputStateCreateInfo* vertexInputInfo{nullptr};
      descset_layout = createDescriptorSetLayout<DescSetLayoutType::Skybox>();
      std::vector<VkDescriptorSetLayout> dSetLayout = { descset_layout };
      vertexInputInfo = getVertexInputState<VertexBindingType::Vertex3D>();

      std::vector<VkPushConstantRange> vkPcs = {};
      VkPushConstantRange vkPc;
      vkPc.offset = 0;
      vkPc.size = sizeof(constants);
      vkPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      vkPcs.emplace_back(vkPc);

      pipeline_layout = _renderer->getAPI()->createPipelineLayout(dSetLayout, vkPcs);

      graphicPipeline = _renderer->getAPI()->createGraphicsPipeline(
        pipeline_layout,
        shaderName,
        shaders,
        *vertexInputInfo,
        VK_CULL_MODE_BACK_BIT,
        false, false, false,
        VK_POLYGON_MODE_FILL);

    } else if (shaderName == "grid" || shaderName == "2d") {
      VkPipelineVertexInputStateCreateInfo* vertexInputInfo{nullptr};
      descset_layout = createDescriptorSetLayout<DescSetLayoutType::HUD>();
      std::vector<VkDescriptorSetLayout> dSetLayout = { descset_layout };
      vertexInputInfo = getVertexInputState<VertexBindingType::Vertex2D>();

      std::vector<VkPushConstantRange> vkPcs = {};
      VkPushConstantRange vkPc;
      vkPc.offset = 0;
      vkPc.size = sizeof(constants);
      vkPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      vkPcs.emplace_back(vkPc);

      pipeline_layout = _renderer->getAPI()->createPipelineLayout(dSetLayout, vkPcs);

      graphicPipeline = _renderer->getAPI()->createGraphicsPipeline(
        pipeline_layout,
        shaderName,
        shaders,
        *vertexInputInfo,
        VK_CULL_MODE_BACK_BIT,
        true, true, true,
        VK_POLYGON_MODE_FILL);
    } else {
      VkPipelineVertexInputStateCreateInfo* vertexInputInfo{nullptr};

      bool hasColorAttachment{true};
      bool has_dynamic_depth_bias{false};

      VkDescriptorPoolSize dpsSB;
      dpsSB.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      dpsSB.descriptorCount = 10;

      poolSizes.emplace_back(dpsSB);

      std::vector<VkPushConstantRange> vkPcs = {};
      VkPushConstantRange vkPc;
      vkPc.offset = 0;
      vkPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      vkPc.size = sizeof(constants);
      has_dynamic_culling = true;

      bool need_bis{ true };
      if (shaderName == "shadowMap" || shaderName == "shadowMapSpot") {
        hasColorAttachment = false;
        has_dynamic_depth_bias = true;
        has_dynamic_culling = false;
        cullMode = VK_CULL_MODE_FRONT_BIT;
        descset_layout = createDescriptorSetLayout<DescSetLayoutType::Offscreen>();
        vkPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        vkPc.size = sizeof(shadowMapConstants);
        need_bis = false;
      } else {
        descset_layout = createDescriptorSetLayout<DescSetLayoutType::Entity>();
      }
      vertexInputInfo = getVertexInputState<VertexBindingType::Vertex3D>();

      vkPcs.emplace_back(vkPc);
      std::vector<VkDescriptorSetLayout> dSetLayout = { descset_layout };
      pipeline_layout = _renderer->getAPI()->createPipelineLayout(dSetLayout, vkPcs);

      graphicPipeline = _renderer->getAPI()->createGraphicsPipeline(
        pipeline_layout,
        shaderName,
        shaders,
        *vertexInputInfo,
        cullMode,
        true, true, true,
        VK_POLYGON_MODE_FILL,
        hasColorAttachment,
        has_dynamic_depth_bias,
        has_dynamic_culling);

      if (need_bis) {
        graphicPipelineBis = _renderer->getAPI()->createGraphicsPipeline(
          pipeline_layout,
          shaderName,
          shaders,
          *vertexInputInfo,
          VK_CULL_MODE_NONE,
          true, true, true,
          VK_POLYGON_MODE_FILL,
          hasColorAttachment,
          has_dynamic_depth_bias,
          has_dynamic_culling);
      }
    }

    auto descriptorPool = _renderer->getAPI()->createDescriptorPool(poolSizes, 1000);

    VulkanPipeline pipeline{};
    pipeline.pipeline = graphicPipeline;
    pipeline.pipeline_bis = graphicPipelineBis;
    pipeline.pipeline_layout = pipeline_layout;
    pipeline.desc_pool = descriptorPool;
    pipeline.descset_layout = descset_layout;
    pipeline.shaders = shaders;

    if (shaderName == "shadowMap" || shaderName == "shadowMapSpot" || "normal_debug") {
      pipeline.descset = _renderer->getAPI()->createDescriptorSets(pipeline.desc_pool, { pipeline.descset_layout }, 1);
    }
    _renderer->addPipeline(shaderName, pipeline);
  }

  std::vector<VkPipelineShaderStageCreateInfo> ShaderManager::getShadersInfo(std::string const & shaderName, bool offscreen)
  {
    std::vector<VkPipelineShaderStageCreateInfo> shaders_infos;

    //@todo use pSpecializationInfo

    VkPipelineShaderStageCreateInfo vertex_info{};
    vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_info.module = _shaders->shaders[shaderName][0];
    vertex_info.pName = "main";
    shaders_infos.emplace_back(vertex_info);

    if (!offscreen) {
      VkPipelineShaderStageCreateInfo frag_info{};
      frag_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      frag_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      frag_info.module = _shaders->shaders[shaderName][1];
      frag_info.pName = "main";
      shaders_infos.emplace_back(frag_info);

      //@todo option normal on/off
      if (_shaders->shaders[shaderName].size() > 2) {
        VkPipelineShaderStageCreateInfo geom_info{};
        geom_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        geom_info.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        geom_info.module = _shaders->shaders[shaderName][2];
        geom_info.pName = "main";
        shaders_infos.emplace_back(geom_info);
      }
    }

    return shaders_infos;
  }

  template <VertexBindingType T>
  VkPipelineVertexInputStateCreateInfo* ShaderManager::getVertexInputState()
  {
    VkPipelineVertexInputStateCreateInfo* vertexInputInfo = new VkPipelineVertexInputStateCreateInfo();

    if constexpr (T == VertexBindingType::Vertex3D) {
      std::array<VkVertexInputAttributeDescription, 5>* attDesc = new std::array<VkVertexInputAttributeDescription, 5>(Vertex::getAttributeDescriptions());
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
