module Engine.Managers.ShaderManager;

import std;

import Engine.Core.Vertex;

import Engine.Core.Json;
import Engine.Core.Logger;
import Engine.Core.PlpTypedef;
import Engine.Core.Tools;
import Engine.Core.Volk;

import Engine.Managers.ConfigManagerLocator;

namespace Poulpe
{
  ShaderManager::ShaderManager(Renderer& renderer)
    : _renderer(renderer)
  {
    _shaders = std::make_unique<VulkanShaders>();
  }

  void ShaderManager::addShader(
    std::string const& name,
    std::string const& vert_path,
    std::string const& frag_path,
    std::string const& geom_path,
    std::string const& tese_path,
    std::string const& tesc_path)
  {

    if (!std::filesystem::exists(vert_path)) {
      Logger::critical("vertex shader file {} does not exits.", vert_path);
      return;
    }

    if (!std::filesystem::exists(frag_path)) {
      Logger::critical("fragment shader file {} does not exits.", frag_path);
      return;
    }

    auto const vert_shader { Tools::readFile(vert_path) };
    auto const frag_shader { Tools::readFile(frag_path) };

    _shaders->shaders[name]["vert"] = _renderer.getAPI()->createShaderModule(vert_shader);
    _shaders->shaders[name]["frag"] = _renderer.getAPI()->createShaderModule(frag_shader);
    if (!geom_path.empty() && std::filesystem::exists(geom_path)) {
      auto const geom_shader { Tools::readFile(geom_path) };

      if (!geom_shader.empty()) {
        _shaders->shaders[name]["geom"] = _renderer.getAPI()->createShaderModule(geom_shader);
      }
    } else {
      Logger::warn("geometry shader file {} does not exits.", geom_path);
      }

    if (!tese_path.empty() && std::filesystem::exists(tese_path)) {
      auto const tese_shader { Tools::readFile(tese_path) };
      if (!tese_shader.empty()) {
        _shaders->shaders[name]["tese"] =  _renderer.getAPI()->createShaderModule(tese_shader);
      }
    } else {
      Logger::warn("tese shader file {} does not exits.", tese_path);
    }

    if (!tesc_path.empty() && std::filesystem::exists(tesc_path)) {
      auto const tesc_shader { Tools::readFile(tesc_path) };

      if (!tesc_shader.empty()) {
        _shaders->shaders[name]["tesc"] = _renderer.getAPI()->createShaderModule(tesc_shader);
      }
    } else {
      Logger::warn("tesc shader file {} does not exits.", tesc_path);
    }

    createGraphicPipeline(name);
  }

  void ShaderManager::clear()
  {
    _shaders->shaders.clear();
  }

  std::function<void(std::latch& count_down)> ShaderManager::load(json config)
  {
    _config = config;

    auto const root_path { ConfigManagerLocator::get()->rootPath() };

    return [this, root_path](std::latch& count_down) {
      for (auto & shader : _config["shader"].items()) {

        auto key = static_cast<std::string>(shader.key());
        auto data = shader.value();

        addShader(
          key,
          root_path + "/" + data["vert"].get<std::string>(),
          root_path + "/" + data["frag"].get<std::string>(),
          root_path + "/" + data["geom"].get<std::string>(),
          root_path + "/" + data["tese"].get<std::string>(),
          root_path + "/" + data["tesc"].get<std::string>());
      }
      count_down.count_down();
    };
  }

  template <DescSetLayoutType T>
  VkDescriptorSetLayout ShaderManager::createDescriptorSetLayout()
  {
    std::vector<VkDescriptorSetLayoutBinding> bindings {};

    if constexpr (T == DescSetLayoutType::Entity) {
      VkDescriptorSetLayoutBinding ubo_binding{};
      ubo_binding.binding = 0;
      ubo_binding.descriptorCount = 1;
      ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ubo_binding.pImmutableSamplers = nullptr;
      ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

      VkDescriptorSetLayoutBinding sampler_binding{};
      sampler_binding.binding = 1;
      sampler_binding.descriptorCount = 8;
      sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      sampler_binding.pImmutableSamplers = nullptr;
      sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding storage_binding{};
      storage_binding.binding = 2;
      storage_binding.descriptorCount = 2;
      storage_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      storage_binding.pImmutableSamplers = nullptr;
      storage_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding depth_map_binding{};
      depth_map_binding.binding = 3;
      depth_map_binding.descriptorCount = 1;
      depth_map_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      depth_map_binding.pImmutableSamplers = nullptr;
      depth_map_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding cubemap_binding{};
      cubemap_binding.binding = 4;
      cubemap_binding.descriptorCount = 1;
      cubemap_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      cubemap_binding.pImmutableSamplers = nullptr;
      cubemap_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding light_storage_binding{};
      light_storage_binding.binding = 5;
      light_storage_binding.descriptorCount = 1;
      light_storage_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      light_storage_binding.pImmutableSamplers = nullptr;
      light_storage_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding csm_binding{};
      csm_binding.binding = 6;
      csm_binding.descriptorCount = 1;
      csm_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      csm_binding.pImmutableSamplers = nullptr;
      csm_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      bindings = {
        ubo_binding,
        sampler_binding,
        storage_binding,
        depth_map_binding,
        cubemap_binding,
        light_storage_binding,
        csm_binding };

    } else if constexpr (T == DescSetLayoutType::Debug) {
      VkDescriptorSetLayoutBinding ubo_binding{};
      ubo_binding.binding = 0;
      ubo_binding.descriptorCount = 1;
      ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ubo_binding.pImmutableSamplers = nullptr;
      ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;

      VkDescriptorSetLayoutBinding sampler_binding{};
      sampler_binding.binding = 1;
      sampler_binding.descriptorCount = 8;
      sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      sampler_binding.pImmutableSamplers = nullptr;
      sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding storage_binding{};
      storage_binding.binding = 2;
      storage_binding.descriptorCount = 2;
      storage_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      storage_binding.pImmutableSamplers = nullptr;
      storage_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding depth_map_binding{};
      depth_map_binding.binding = 3;
      depth_map_binding.descriptorCount = 1;
      depth_map_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      depth_map_binding.pImmutableSamplers = nullptr;
      depth_map_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding cubemap_binding{};
      cubemap_binding.binding = 4;
      cubemap_binding.descriptorCount = 1;
      cubemap_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      cubemap_binding.pImmutableSamplers = nullptr;
      cubemap_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding light_storage_binding{};
      light_storage_binding.binding = 5;
      light_storage_binding.descriptorCount = 1;
      light_storage_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      light_storage_binding.pImmutableSamplers = nullptr;
      light_storage_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding csm_binding{};
      csm_binding.binding = 6;
      csm_binding.descriptorCount = 1;
      csm_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      csm_binding.pImmutableSamplers = nullptr;
      csm_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      bindings = {
        ubo_binding,
        sampler_binding,
        storage_binding,
        depth_map_binding,
        cubemap_binding,
        light_storage_binding,
        csm_binding };

    } else if constexpr (T == DescSetLayoutType::Text) {
      VkDescriptorSetLayoutBinding ubo_binding{};
      ubo_binding.binding = 0;
      ubo_binding.descriptorCount = 1;
      ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ubo_binding.pImmutableSamplers = nullptr;
      ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

      VkDescriptorSetLayoutBinding sampler_binding{};
      sampler_binding.binding = 1;
      sampler_binding.descriptorCount = 1;
      sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      sampler_binding.pImmutableSamplers = nullptr;
      sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      bindings = { ubo_binding, sampler_binding };
    } else if constexpr (T == DescSetLayoutType::Skybox) {
      VkDescriptorSetLayoutBinding ubo_binding{};
      ubo_binding.binding = 0;
      ubo_binding.descriptorCount = 1;
      ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ubo_binding.pImmutableSamplers = nullptr;
      ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

      VkDescriptorSetLayoutBinding sampler_binding{};
      sampler_binding.binding = 1;
      sampler_binding.descriptorCount = 1;
      sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      sampler_binding.pImmutableSamplers = nullptr;
      sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      bindings = { ubo_binding, sampler_binding };
    } else if constexpr (T == DescSetLayoutType::Offscreen) {
      VkDescriptorSetLayoutBinding ubo_binding{};
      ubo_binding.binding = 0;
      ubo_binding.descriptorCount = 1;
      ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ubo_binding.pImmutableSamplers = nullptr;
      ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

      VkDescriptorSetLayoutBinding storage_binding{};
      storage_binding.binding = 1;
      storage_binding.descriptorCount = 1;
      storage_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      storage_binding.pImmutableSamplers = nullptr;
      storage_binding.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT
        | VK_SHADER_STAGE_FRAGMENT_BIT;

      bindings = { ubo_binding, storage_binding };
    } else if constexpr (T == DescSetLayoutType::Terrain) {
      VkDescriptorSetLayoutBinding ubo_binding{};
      ubo_binding.binding = 0;
      ubo_binding.descriptorCount = 1;
      ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ubo_binding.pImmutableSamplers = nullptr;
      ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
                              | VK_SHADER_STAGE_FRAGMENT_BIT
                              | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
                              | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

      VkDescriptorSetLayoutBinding sampler_binding{};
      sampler_binding.binding = 1;
      sampler_binding.descriptorCount = 7;
      sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      sampler_binding.pImmutableSamplers = nullptr;
      sampler_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
                                  | VK_SHADER_STAGE_FRAGMENT_BIT
                                  | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
                                  | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

      VkDescriptorSetLayoutBinding env_sampler_binding{};
      env_sampler_binding.binding = 2;
      env_sampler_binding.descriptorCount = 1;
      env_sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      env_sampler_binding.pImmutableSamplers = nullptr;
      env_sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding storage_binding{};
      storage_binding.binding = 3;
      storage_binding.descriptorCount = 1;
      storage_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      storage_binding.pImmutableSamplers = nullptr;
      storage_binding.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT
        | VK_SHADER_STAGE_FRAGMENT_BIT
        | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

      VkDescriptorSetLayoutBinding csm_binding{};
      csm_binding.binding = 4;
      csm_binding.descriptorCount = 1;
      csm_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      csm_binding.pImmutableSamplers = nullptr;
      csm_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding depth_map_binding{};
      depth_map_binding.binding = 5;
      depth_map_binding.descriptorCount = 1;
      depth_map_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      depth_map_binding.pImmutableSamplers = nullptr;
      depth_map_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      bindings = { ubo_binding, sampler_binding, env_sampler_binding, storage_binding, csm_binding, depth_map_binding };

    }
    else if constexpr (T == DescSetLayoutType::Water) {
      VkDescriptorSetLayoutBinding ubo_binding{};
      ubo_binding.binding = 0;
      ubo_binding.descriptorCount = 1;
      ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ubo_binding.pImmutableSamplers = nullptr;
      ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
        | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
        | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

      VkDescriptorSetLayoutBinding sampler_binding{};
      sampler_binding.binding = 1;
      sampler_binding.descriptorCount = 5;
      sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      sampler_binding.pImmutableSamplers = nullptr;
      sampler_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
        | VK_SHADER_STAGE_FRAGMENT_BIT
        | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
        | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

      VkDescriptorSetLayoutBinding env_sampler_binding{};
      env_sampler_binding.binding = 2;
      env_sampler_binding.descriptorCount = 1;
      env_sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      env_sampler_binding.pImmutableSamplers = nullptr;
      env_sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding storage_binding{};
      storage_binding.binding = 3;
      storage_binding.descriptorCount = 1;
      storage_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      storage_binding.pImmutableSamplers = nullptr;
      storage_binding.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT
        | VK_SHADER_STAGE_FRAGMENT_BIT
        | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

      VkDescriptorSetLayoutBinding csm_binding{};
      csm_binding.binding = 4;
      csm_binding.descriptorCount = 1;
      csm_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      csm_binding.pImmutableSamplers = nullptr;
      csm_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      
      VkDescriptorSetLayoutBinding depth_map_binding{};
      depth_map_binding.binding = 5;
      depth_map_binding.descriptorCount = 1;
      depth_map_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      depth_map_binding.pImmutableSamplers = nullptr;
      depth_map_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      bindings = { ubo_binding, sampler_binding, env_sampler_binding, storage_binding, csm_binding, depth_map_binding };

    } else if constexpr (T == DescSetLayoutType::Text) {
      VkDescriptorSetLayoutBinding ubo_binding{};
      ubo_binding.binding = 0;
      ubo_binding.descriptorCount = 1;
      ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ubo_binding.pImmutableSamplers = nullptr;
      ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

      VkDescriptorSetLayoutBinding sampler_binding{};
      sampler_binding.binding = 1;
      sampler_binding.descriptorCount = 1;
      sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      sampler_binding.pImmutableSamplers = nullptr;
      sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      bindings = { ubo_binding, sampler_binding };
    } else {
      Logger::critical("unknown descSetLayoutType");
      throw std::runtime_error("unknown descSetLayoutType");
    }

    return _renderer.getAPI()->createDescriptorSetLayout(bindings);
  }

  void ShaderManager::createGraphicPipeline(std::string const & shader_name)
  {
    std::uint32_t pool_size { 1000 };
    Logger::debug("shader_name {}", shader_name);
    bool offscreen = (shader_name == "shadow_map") ? true : false;
    bool need_bis{ false };

    PipeLineCreateInfo pipeline_create_infos{};
    pipeline_create_infos.name = shader_name;
    pipeline_create_infos.cull_mode = VK_CULL_MODE_BACK_BIT;
    pipeline_create_infos.has_depth_test = true;
    pipeline_create_infos.has_depth_write = true;
    pipeline_create_infos.has_stencil_test = true;
    pipeline_create_infos.polygone_mode = VK_POLYGON_MODE_FILL;
    pipeline_create_infos.has_color_attachment = true;
    pipeline_create_infos.has_dynamic_depth_bias = true;
    pipeline_create_infos.has_dynamic_culling = false;

    std::vector<VkDescriptorPoolSize> poolSizes{};
    VkDescriptorPoolSize dpsUbo;
    dpsUbo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dpsUbo.descriptorCount = pool_size;

    VkDescriptorPoolSize dpsIS;
    dpsIS.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dpsIS.descriptorCount = pool_size;

    poolSizes.emplace_back(dpsUbo);

    if (!offscreen) poolSizes.emplace_back(dpsIS);

    auto shaders = getShadersInfo(shader_name, offscreen);

    VkPipeline graphic_pipeline{VK_NULL_HANDLE};
    VkPipeline graphic_pipeline_bis{VK_NULL_HANDLE};
    VkPipelineLayout pipeline_layout{VK_NULL_HANDLE};
    VkDescriptorSetLayout descset_layout{VK_NULL_HANDLE};
    VkPipelineVertexInputStateCreateInfo* vertex_input_info{nullptr};
    VkPushConstantRange push_constants{};

    vertex_input_info = getVertexInputState<VertexBindingType::Vertex3D>();

    //@todo replace shader_name == with a shader_type ==
    if (shader_name == "text") {
      need_bis = true;
      descset_layout = createDescriptorSetLayout<DescSetLayoutType::Text>();

      push_constants.offset = 0;
      push_constants.size = sizeof(constants);
      push_constants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

      pipeline_create_infos.has_depth_test = true;
      pipeline_create_infos.has_depth_write = false;
      pipeline_create_infos.has_stencil_test = false;
      pipeline_create_infos.has_color_attachment = true;
      pipeline_create_infos.has_dynamic_depth_bias = false;
    } else if (shader_name == "skybox") {
      descset_layout = createDescriptorSetLayout<DescSetLayoutType::Skybox>();

      push_constants.offset = 0;
      push_constants.size = sizeof(constants);
      push_constants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

      pipeline_create_infos.has_depth_test = false;
      pipeline_create_infos.has_depth_write = false;
      pipeline_create_infos.has_stencil_test = false;
      pipeline_create_infos.has_color_attachment = true;
      pipeline_create_infos.has_dynamic_depth_bias = false;
      pipeline_create_infos.cull_mode = VK_CULL_MODE_NONE;
      pipeline_create_infos.polygone_mode = VK_POLYGON_MODE_FILL;

    } else if (shader_name == "terrain" || shader_name == "water") {
      if (shader_name == "water") {
        descset_layout = createDescriptorSetLayout<DescSetLayoutType::Water>();
        //pipeline_create_infos.polygone_mode = VK_POLYGON_MODE_LINE;
        pipeline_create_infos.has_depth_write = false;
        pipeline_create_infos.has_dynamic_depth_bias = false;
        
      } else {
        descset_layout = createDescriptorSetLayout<DescSetLayoutType::Terrain>();
        pipeline_create_infos.has_depth_write = true;
        pipeline_create_infos.has_dynamic_depth_bias = false;
      }
      push_constants.offset = 0;
      push_constants.size = sizeof(constants);
      push_constants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
                                  | VK_SHADER_STAGE_FRAGMENT_BIT
                                  | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT
                                  | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

      pipeline_create_infos.cull_mode = VK_CULL_MODE_NONE;
      pipeline_create_infos.has_depth_test = true;
      pipeline_create_infos.has_stencil_test = false;
      pipeline_create_infos.is_patch_list = true;
      pipeline_create_infos.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

      VkDescriptorPoolSize dpsSB;
      dpsSB.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      dpsSB.descriptorCount = pool_size;

      poolSizes.emplace_back(dpsSB);
    } else if (shader_name == "shadow_map" || shader_name == "csm") {

      pipeline_create_infos.cull_mode = VK_CULL_MODE_FRONT_BIT;
      pipeline_create_infos.has_color_attachment = false;
      pipeline_create_infos.has_dynamic_depth_bias = true;
      pipeline_create_infos.has_depth_test = true;
      pipeline_create_infos.has_depth_write = true;

      descset_layout = createDescriptorSetLayout<DescSetLayoutType::Offscreen>();
      push_constants.offset = 0;
      push_constants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      push_constants.size = sizeof(constants);
      need_bis = false;

      VkDescriptorPoolSize dpsSB;
      dpsSB.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      dpsSB.descriptorCount = pool_size;

      poolSizes.emplace_back(dpsSB);
    } else if (shader_name == "normal_debug") { 
      VkDescriptorPoolSize dpsSB;
      dpsSB.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      dpsSB.descriptorCount = pool_size;

      poolSizes.emplace_back(dpsSB);

      push_constants.offset = 0;
      push_constants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;
      push_constants.size = sizeof(constants);

      need_bis = true;
      descset_layout = createDescriptorSetLayout<DescSetLayoutType::Debug>();
    } else {
      VkDescriptorPoolSize dpsSB;
      dpsSB.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      dpsSB.descriptorCount = pool_size;

      poolSizes.emplace_back(dpsSB);

      push_constants.offset = 0;
      push_constants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      push_constants.size = sizeof(constants);

      need_bis = true;
      descset_layout = createDescriptorSetLayout<DescSetLayoutType::Entity>();
    }

    pipeline_layout = _renderer.getAPI()->createPipelineLayout({ descset_layout }, { push_constants });

    pipeline_create_infos.shaders_create_info = std::move(shaders);
    pipeline_create_infos.vertex_input_info = std::move(vertex_input_info);
    pipeline_create_infos.pipeline_layout = pipeline_layout;

    graphic_pipeline = _renderer.getAPI()->createGraphicsPipeline(pipeline_create_infos);
    auto descriptorPool = _renderer.getAPI()->createDescriptorPool(poolSizes, pool_size);

    if (need_bis) {
      pipeline_create_infos.cull_mode = VK_CULL_MODE_NONE;
      pipeline_create_infos.polygone_mode = VK_POLYGON_MODE_FILL;

      graphic_pipeline_bis = _renderer.getAPI()->createGraphicsPipeline(pipeline_create_infos);
    }

    VulkanPipeline pipeline{};
    pipeline.pipeline = graphic_pipeline;
    pipeline.pipeline_bis = graphic_pipeline_bis;
    pipeline.pipeline_layout = pipeline_layout;
    pipeline.desc_pool = descriptorPool;
    pipeline.descset_layout = descset_layout;
    pipeline.shaders = shaders;

    if (shader_name == "shadow_map" || shader_name == "csm") {
      pipeline.descset = _renderer.getAPI()->createDescriptorSets(pipeline, 1);
    }
    _renderer.addPipeline(shader_name, pipeline);
  }

  std::vector<VkPipelineShaderStageCreateInfo> ShaderManager::getShadersInfo(std::string const & shader_name, bool)
  {
    std::vector<VkPipelineShaderStageCreateInfo> shaders_infos;

    //@todo use pSpecializationInfo

    VkPipelineShaderStageCreateInfo vertex_info{};
    vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_info.module = _shaders->shaders[shader_name]["vert"];
    vertex_info.pName = "main";
    shaders_infos.emplace_back(vertex_info);

    VkPipelineShaderStageCreateInfo frag_info{};
    frag_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_info.module = _shaders->shaders[shader_name]["frag"];
    frag_info.pName = "main";
    shaders_infos.emplace_back(frag_info);

    if (_shaders->shaders[shader_name].contains("geom")) {
      VkPipelineShaderStageCreateInfo geom_info{};
      geom_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      geom_info.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
      geom_info.module = _shaders->shaders[shader_name]["geom"];
      geom_info.pName = "main";
      shaders_infos.emplace_back(geom_info);
    }
    if (_shaders->shaders[shader_name].contains("tese")) {
      VkPipelineShaderStageCreateInfo tese_info{};
      tese_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      tese_info.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
      tese_info.module = _shaders->shaders[shader_name]["tese"];
      tese_info.pName = "main";
      shaders_infos.emplace_back(tese_info);
    }
    if (_shaders->shaders[shader_name].contains("tesc")) {
      VkPipelineShaderStageCreateInfo tesc_info{};
      tesc_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      tesc_info.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
      tesc_info.module = _shaders->shaders[shader_name]["tesc"];
      tesc_info.pName = "main";
      shaders_infos.emplace_back(tesc_info);
    }

    return shaders_infos;
  }

  template <VertexBindingType T>
  VkPipelineVertexInputStateCreateInfo* ShaderManager::getVertexInputState()
  {
    VkPipelineVertexInputStateCreateInfo* vertex_input_info = new VkPipelineVertexInputStateCreateInfo();

    if constexpr (T == VertexBindingType::Vertex3D) {
      std::array<VkVertexInputAttributeDescription, 5>* attDesc = new std::array<VkVertexInputAttributeDescription, 5>(Vertex::getAttributeDescriptions());
      VkVertexInputBindingDescription* bDesc = new VkVertexInputBindingDescription(Vertex::getBindingDescription());

      vertex_input_info->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertex_input_info->vertexBindingDescriptionCount = 1;
      vertex_input_info->vertexAttributeDescriptionCount = static_cast<std::uint32_t>(Vertex::getAttributeDescriptions().size());
      vertex_input_info->pVertexBindingDescriptions = bDesc;
      vertex_input_info->pVertexAttributeDescriptions = attDesc->data();
    } else {
      throw std::runtime_error("unknown vertex input state type");
    }

    return vertex_input_info;
  }
}
