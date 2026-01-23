module;

#include <volk.h>

export module Engine.Core.VulkanTypes;

import std;

namespace Poulpe
{
  export struct VulkanShaders
  {
    std::unordered_map<std::string, std::unordered_map<std::string, VkShaderModule>> shaders;
  };

  export struct VulkanPipeline
  {
    VkPipelineLayout pipeline_layout;
    VkDescriptorPool desc_pool;
    VkDescriptorSetLayout descset_layout;
    VkDescriptorSet descset;
    VkPipelineCache pipeline_cache;
    VkPipeline pipeline;
    VkPipeline pipeline_bis;

    std::vector<VkPipelineShaderStageCreateInfo> shaders{};
  };

  export struct PipeLineCreateInfo
  {
    VkPipelineLayout pipeline_layout;
    std::string_view name;
    std::vector<VkPipelineShaderStageCreateInfo> shaders_create_info;
    VkPipelineVertexInputStateCreateInfo* vertex_input_info;
    VkCullModeFlagBits cull_mode = VK_CULL_MODE_BACK_BIT;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode polygone_mode = VK_POLYGON_MODE_FILL;
    VkCompareOp compare_op = VK_COMPARE_OP_LESS_OR_EQUAL;

    bool has_color_attachment{ false };
    bool has_depth_test{ false };
    bool has_depth_write{ false };
    bool has_dynamic_culling{ false };
    bool has_dynamic_depth_bias{ false };
    bool has_stencil_test{ false };
    bool is_patch_list{ false };
  };
}
