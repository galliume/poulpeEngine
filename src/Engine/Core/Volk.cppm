module;

#include <volk.h>
#include <vulkan/vulkan.h>

// Capture macro values (using inline to avoid TU-local exposure and name collisions)
inline constexpr auto VK_API_VERSION_1_0_plp = VK_API_VERSION_1_0;
inline constexpr auto VK_API_VERSION_1_1_plp = VK_API_VERSION_1_1;
inline constexpr auto VK_API_VERSION_1_2_plp = VK_API_VERSION_1_2;
inline constexpr auto VK_API_VERSION_1_3_plp = VK_API_VERSION_1_3;
inline constexpr auto VK_API_VERSION_1_4_plp = VK_API_VERSION_1_4;
inline constexpr auto VK_EXT_DEBUG_REPORT_EXTENSION_NAME_plp = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
inline constexpr auto VK_EXT_DEBUG_UTILS_EXTENSION_NAME_plp = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
inline constexpr auto VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME_plp = VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME;
inline constexpr auto VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME_plp = VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME;
inline constexpr auto VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME_plp = VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME;
inline constexpr auto VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME_plp = VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME;
inline constexpr auto VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME_plp = VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME;
inline constexpr auto VK_FALSE_plp = VK_FALSE;
inline constexpr auto VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME_plp = VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME;
inline constexpr auto VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME_plp = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
inline constexpr auto VK_KHR_MAINTENANCE_4_EXTENSION_NAME_plp = VK_KHR_MAINTENANCE_4_EXTENSION_NAME;
inline constexpr auto VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME_plp = VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME;
inline constexpr auto VK_KHR_SWAPCHAIN_EXTENSION_NAME_plp = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
inline constexpr auto VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME_plp = VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME;
inline constexpr auto VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME_plp = VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME;
inline constexpr auto VK_LOD_CLAMP_NONE_plp = VK_LOD_CLAMP_NONE;
inline constexpr auto VK_QUEUE_FAMILY_IGNORED_plp = VK_QUEUE_FAMILY_IGNORED;
inline constexpr auto VK_REMAINING_ARRAY_LAYERS_plp = VK_REMAINING_ARRAY_LAYERS;
inline constexpr auto VK_REMAINING_MIP_LEVELS_plp = VK_REMAINING_MIP_LEVELS;
inline constexpr auto VK_SUBPASS_EXTERNAL_plp = VK_SUBPASS_EXTERNAL;
inline constexpr auto VK_TRUE_plp = VK_TRUE;
inline constexpr auto VK_UUID_SIZE_plp = VK_UUID_SIZE;
inline constexpr auto VK_WHOLE_SIZE_plp = VK_WHOLE_SIZE;
inline auto const VK_NULL_HANDLE_plp = VK_NULL_HANDLE;

// Undefine macros
#undef VK_API_VERSION_1_0
#undef VK_API_VERSION_1_1
#undef VK_API_VERSION_1_2
#undef VK_API_VERSION_1_3
#undef VK_API_VERSION_1_4
#undef VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#undef VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#undef VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME
#undef VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME
#undef VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME
#undef VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME
#undef VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME
#undef VK_FALSE
#undef VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
#undef VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
#undef VK_KHR_MAINTENANCE_4_EXTENSION_NAME
#undef VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME
#undef VK_KHR_SWAPCHAIN_EXTENSION_NAME
#undef VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME
#undef VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME
#undef VK_LOD_CLAMP_NONE
#undef VK_QUEUE_FAMILY_IGNORED
#undef VK_REMAINING_ARRAY_LAYERS
#undef VK_REMAINING_MIP_LEVELS
#undef VK_SUBPASS_EXTERNAL
#undef VK_TRUE
#undef VK_UUID_SIZE
#undef VK_WHOLE_SIZE
#undef VK_NULL_HANDLE
#undef VK_MAKE_VERSION
#undef VK_VERSION_MAJOR
#undef VK_VERSION_MINOR
#undef VK_VERSION_PATCH

export module Engine.Core.Volk;

export
{

  using ::volkInitialize;
  using ::volkLoadInstance;
  using ::volkLoadDevice;


  // Macros redefined as constexpr constants
  inline constexpr auto VK_API_VERSION_1_0 = VK_API_VERSION_1_0_plp;
  inline constexpr auto VK_API_VERSION_1_1 = VK_API_VERSION_1_1_plp;
  inline constexpr auto VK_API_VERSION_1_2 = VK_API_VERSION_1_2_plp;
  inline constexpr auto VK_API_VERSION_1_3 = VK_API_VERSION_1_3_plp;
  inline constexpr auto VK_API_VERSION_1_4 = VK_API_VERSION_1_4_plp;
  inline constexpr auto VK_EXT_DEBUG_REPORT_EXTENSION_NAME = VK_EXT_DEBUG_REPORT_EXTENSION_NAME_plp;
  inline constexpr auto VK_EXT_DEBUG_UTILS_EXTENSION_NAME = VK_EXT_DEBUG_UTILS_EXTENSION_NAME_plp;
  inline constexpr auto VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME = VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME_plp;
  inline constexpr auto VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME = VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME_plp;
  inline constexpr auto VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME = VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME_plp;
  inline constexpr auto VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME = VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME_plp;
  inline constexpr auto VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME = VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME_plp;
  inline constexpr auto VK_FALSE = VK_FALSE_plp;
  inline constexpr auto VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME = VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME_plp;
  inline constexpr auto VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME_plp;
  inline constexpr auto VK_KHR_MAINTENANCE_4_EXTENSION_NAME = VK_KHR_MAINTENANCE_4_EXTENSION_NAME_plp;
  inline constexpr auto VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME = VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME_plp;
  inline constexpr auto VK_KHR_SWAPCHAIN_EXTENSION_NAME = VK_KHR_SWAPCHAIN_EXTENSION_NAME_plp;
  inline constexpr auto VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME = VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME_plp;
  inline constexpr auto VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME = VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME_plp;
  inline constexpr auto VK_LOD_CLAMP_NONE = VK_LOD_CLAMP_NONE_plp;
  inline constexpr auto VK_QUEUE_FAMILY_IGNORED = VK_QUEUE_FAMILY_IGNORED_plp;
  inline constexpr auto VK_REMAINING_ARRAY_LAYERS = VK_REMAINING_ARRAY_LAYERS_plp;
  inline constexpr auto VK_REMAINING_MIP_LEVELS = VK_REMAINING_MIP_LEVELS_plp;
  inline constexpr auto VK_SUBPASS_EXTERNAL = VK_SUBPASS_EXTERNAL_plp;
  inline constexpr auto VK_TRUE = VK_TRUE_plp;
  inline constexpr auto VK_UUID_SIZE = VK_UUID_SIZE_plp;
  inline constexpr auto VK_WHOLE_SIZE = VK_WHOLE_SIZE_plp;
  inline constexpr auto VK_NULL_HANDLE = nullptr;

  // Function-like macros implemented as inline constexpr functions
  inline constexpr uint32_t VK_MAKE_VERSION(uint32_t major, uint32_t minor, uint32_t patch) {
    return (((major) << 22U) | ((minor) << 12U) | (patch));
  }
  inline constexpr uint32_t VK_VERSION_MAJOR(uint32_t version) {
    return (uint32_t)(version) >> 22U;
  }
  inline constexpr uint32_t VK_VERSION_MINOR(uint32_t version) {
    return ((uint32_t)(version) >> 12U) & 0x3FFU;
  }
  inline constexpr uint32_t VK_VERSION_PATCH(uint32_t version) {
    return (uint32_t)(version) & 0xFFFU;
  }

  // Enums and other constants
  using ::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  using ::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
  using ::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  using ::VK_ACCESS_SHADER_READ_BIT;
  using ::VK_ACCESS_SHADER_WRITE_BIT;
  using ::VK_ACCESS_TRANSFER_READ_BIT;
  using ::VK_ACCESS_TRANSFER_WRITE_BIT;
  using ::VK_ATTACHMENT_LOAD_OP_CLEAR;
  using ::VK_ATTACHMENT_STORE_OP_STORE;
  using ::VK_BLEND_FACTOR_ONE;
  using ::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  using ::VK_BLEND_FACTOR_SRC_ALPHA;
  using ::VK_BLEND_FACTOR_ZERO;
  using ::VK_BLEND_OP_ADD;
  using ::VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  using ::VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  using ::VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  using ::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  using ::VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  using ::VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  using ::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  using ::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  using ::VK_COLOR_COMPONENT_A_BIT;
  using ::VK_COLOR_COMPONENT_B_BIT;
  using ::VK_COLOR_COMPONENT_G_BIT;
  using ::VK_COLOR_COMPONENT_R_BIT;
  using ::VK_COLOR_SPACE_HDR10_ST2084_EXT;
  using ::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  using ::VK_COMMAND_BUFFER_LEVEL_SECONDARY;
  using ::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT;
  using ::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  using ::VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
  using ::VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  using ::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  using ::VK_COMPARE_OP_LESS;
  using ::VK_COMPARE_OP_LESS_OR_EQUAL;
  using ::VK_COMPONENT_SWIZZLE_IDENTITY;
  using ::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  using ::VK_CULL_MODE_BACK_BIT;
  using ::VK_CULL_MODE_FRONT_BIT;
  using ::VK_CULL_MODE_NONE;
  using ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  using ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT;
  using ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
  using ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
  using ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
  using ::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
  using ::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  using ::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
  using ::VK_DEPENDENCY_BY_REGION_BIT;
  using ::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
  using ::VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
  using ::VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
  using ::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  using ::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  using ::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  using ::VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT;
  using ::VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT;
  using ::VK_DYNAMIC_STATE_CULL_MODE;
  using ::VK_DYNAMIC_STATE_DEPTH_BIAS;
  using ::VK_DYNAMIC_STATE_SCISSOR;
  using ::VK_DYNAMIC_STATE_VIEWPORT;
  using ::VK_ERROR_EXTENSION_NOT_PRESENT;
  using ::VK_ERROR_OUT_OF_DATE_KHR;
  using ::VK_FENCE_CREATE_SIGNALED_BIT;
  using ::VK_FILTER_LINEAR;
  using ::VK_FORMAT_A2B10G10R10_UNORM_PACK32;
  using ::VK_FORMAT_BC5_UNORM_BLOCK;
  using ::VK_FORMAT_BC7_SRGB_BLOCK;
  using ::VK_FORMAT_D16_UNORM;
  using ::VK_FORMAT_D24_UNORM_S8_UINT;
  using ::VK_FORMAT_D32_SFLOAT;
  using ::VK_FORMAT_D32_SFLOAT_S8_UINT;
  using ::VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
  using ::VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
  using ::VK_FORMAT_R32_SFLOAT;
  using ::VK_FORMAT_R32G32_SFLOAT;
  using ::VK_FORMAT_R32G32B32_SFLOAT;
  using ::VK_FORMAT_R32G32B32A32_SFLOAT;
  using ::VK_FORMAT_R32G32B32A32_SINT;
  using ::VK_FORMAT_R8_UNORM;
  using ::VK_FORMAT_S8_UINT;
  using ::VK_FRONT_FACE_CLOCKWISE;
  using ::VK_IMAGE_ASPECT_COLOR_BIT;
  using ::VK_IMAGE_ASPECT_DEPTH_BIT;
  using ::VK_IMAGE_ASPECT_STENCIL_BIT;
  using ::VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
  using ::VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
  using ::VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
  using ::VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
  using ::VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
  using ::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  using ::VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  using ::VK_IMAGE_LAYOUT_GENERAL;
  using ::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  using ::VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
  using ::VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ;
  using ::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  using ::VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
  using ::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  using ::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  using ::VK_IMAGE_LAYOUT_UNDEFINED;
  using ::VK_IMAGE_TILING_LINEAR;
  using ::VK_IMAGE_TILING_OPTIMAL;
  using ::VK_IMAGE_TYPE_1D;
  using ::VK_IMAGE_TYPE_2D;
  using ::VK_IMAGE_TYPE_3D;
  using ::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  using ::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  using ::VK_IMAGE_USAGE_SAMPLED_BIT;
  using ::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  using ::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  using ::VK_IMAGE_VIEW_TYPE_1D;
  using ::VK_IMAGE_VIEW_TYPE_2D;
  using ::VK_IMAGE_VIEW_TYPE_2D_ARRAY;
  using ::VK_IMAGE_VIEW_TYPE_3D;
  using ::VK_IMAGE_VIEW_TYPE_CUBE;
  using ::VK_INDEX_TYPE_UINT32;
  using ::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  using ::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  using ::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  using ::VK_PIPELINE_BIND_POINT_GRAPHICS;
  using ::VK_PIPELINE_CACHE_HEADER_VERSION_ONE;
  using ::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
  using ::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  using ::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  using ::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
  using ::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  using ::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  using ::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  using ::VK_PIPELINE_STAGE_TRANSFER_BIT;
  using ::VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
  using ::VK_POLYGON_MODE_FILL;
  using ::VK_POLYGON_MODE_LINE;
  using ::VK_PRESENT_MODE_IMMEDIATE_KHR;
  using ::VK_PRESENT_MODE_MAILBOX_KHR;
  using ::VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
  using ::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  using ::VK_QUEUE_COMPUTE_BIT;
  using ::VK_QUEUE_GRAPHICS_BIT;
  using ::VK_QUEUE_TRANSFER_BIT;
  using ::VK_SAMPLE_COUNT_1_BIT;
  using ::VK_SAMPLE_COUNT_16_BIT;
  using ::VK_SAMPLE_COUNT_2_BIT;
  using ::VK_SAMPLE_COUNT_32_BIT;
  using ::VK_SAMPLE_COUNT_4_BIT;
  using ::VK_SAMPLE_COUNT_64_BIT;
  using ::VK_SAMPLE_COUNT_8_BIT;
  using ::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  using ::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  using ::VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
  using ::VK_SAMPLER_ADDRESS_MODE_REPEAT;
  using ::VK_SAMPLER_MIPMAP_MODE_LINEAR;
  using ::VK_SEMAPHORE_TYPE_BINARY;
  using ::VK_SEMAPHORE_TYPE_TIMELINE;
  using ::VK_SHADER_STAGE_FRAGMENT_BIT;
  using ::VK_SHADER_STAGE_GEOMETRY_BIT;
  using ::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
  using ::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
  using ::VK_SHADER_STAGE_VERTEX_BIT;
  using ::VK_SHARING_MODE_CONCURRENT;
  using ::VK_SHARING_MODE_EXCLUSIVE;
  using ::VK_STRUCTURE_TYPE_APPLICATION_INFO;
  using ::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  using ::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  using ::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
  using ::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  using ::VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  using ::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  using ::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  using ::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
  using ::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;
  using ::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
  using ::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES;
  using ::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR;
  using ::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
  using ::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
  using ::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR;
  using ::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
  using ::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
  using ::VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT;
  using ::VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
  using ::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  using ::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  using ::VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  using ::VK_STRUCTURE_TYPE_RENDERING_INFO;
  using ::VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  using ::VK_STRUCTURE_TYPE_SUBMIT_INFO;
  using ::VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  using ::VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO_KHR;
  using ::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  using ::VK_SUBOPTIMAL_KHR;
  using ::VK_SUBPASS_CONTENTS_INLINE;
  using ::VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;
  using ::VK_SUCCESS;
  using ::VK_VERTEX_INPUT_RATE_VERTEX;

  // Types
  using ::VkAccessFlags;
  using ::VkAllocationCallbacks;
  using ::VkApplicationInfo;
  using ::VkAttachmentDescription;
  using ::VkAttachmentLoadOp;
  using ::VkAttachmentReference;
  using ::VkAttachmentStoreOp;
  using ::VkBool32;
  using ::VkBuffer;
  using ::VkBufferCopy;
  using ::VkBufferCreateInfo;
  using ::VkBufferImageCopy;
  using ::VkBufferUsageFlags;
  using ::VkClearColorValue;
  using ::VkClearDepthStencilValue;
  using ::VkClearValue;
  using ::VkColorBlendEquationEXT;
  using ::VkColorSpaceKHR;
  using ::VkCommandBuffer;
  using ::VkCommandBufferAllocateInfo;
  using ::VkCommandBufferBeginInfo;
  using ::VkCommandBufferInheritanceInfo;
  using ::VkCommandBufferUsageFlagBits;
  using ::VkCommandPool;
  using ::VkCommandPoolCreateInfo;
  using ::VkCompareOp;
  using ::VkCullModeFlagBits;
  using ::VkDebugUtilsLabelEXT;
  using ::VkDebugUtilsMessageSeverityFlagBitsEXT;
  using ::VkDebugUtilsMessageTypeFlagsEXT;
  using ::VkDebugUtilsMessengerCallbackDataEXT;
  using ::VkDebugUtilsMessengerCreateInfoEXT;
  using ::VkDebugUtilsMessengerEXT;
  using ::VkDependencyFlags;
  using ::VkDescriptorBindingFlags;
  using ::VkDescriptorBufferInfo;
  using ::VkDescriptorImageInfo;
  using ::VkDescriptorPool;
  using ::VkDescriptorPoolCreateFlags;
  using ::VkDescriptorPoolCreateInfo;
  using ::VkDescriptorPoolSize;
  using ::VkDescriptorSet;
  using ::VkDescriptorSetAllocateInfo;
  using ::VkDescriptorSetLayout;
  using ::VkDescriptorSetLayoutBinding;
  using ::VkDescriptorSetLayoutBindingFlagsCreateInfo;
  using ::VkDescriptorSetLayoutCreateInfo;
  using ::VkDescriptorType;
  using ::VkDevice;
  using ::VkDeviceCreateInfo;
  using ::VkDeviceMemory;
  using ::VkDeviceQueueCreateInfo;
  using ::VkDeviceSize;
  using ::VkDrawIndexedIndirectCommand;
  using ::VkDynamicState;
  using ::VkExtensionProperties;
  using ::VkExtent2D;
  using ::VkFence;
  using ::VkFenceCreateInfo;
  using ::VkFormat;
  using ::VkFormatFeatureFlags;
  using ::VkFormatProperties;
  using ::VkFramebuffer;
  using ::VkFramebufferCreateInfo;
  using ::VkGraphicsPipelineCreateInfo;
  using ::VkImage;
  using ::VkImageAspectFlagBits;
  using ::VkImageAspectFlags;
  using ::VkImageBlit;
  using ::VkImageCreateInfo;
  using ::VkImageLayout;
  using ::VkImageMemoryBarrier;
  using ::VkImageSubresourceLayers;
  using ::VkImageSubresourceRange;
  using ::VkImageTiling;
  using ::VkImageType;
  using ::VkImageUsageFlags;
  using ::VkImageView;
  using ::VkImageViewCreateInfo;
  using ::VkInstance;
  using ::VkInstanceCreateInfo;
  using ::VkLayerProperties;
  using ::VkMemoryAllocateInfo;
  using ::VkMemoryPropertyFlags;
  using ::VkMemoryRequirements;
  using ::VkPhysicalDevice;
  using ::VkPhysicalDeviceDescriptorIndexingFeatures;
  using ::VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
  using ::VkPhysicalDeviceFeatures;
  using ::VkPhysicalDeviceFeatures2;
  using ::VkPhysicalDeviceMaintenance3Properties;
  using ::VkPhysicalDeviceMemoryProperties;
  using ::VkPhysicalDevicePresentWaitFeaturesKHR;
  using ::VkPhysicalDeviceProperties;
  using ::VkPhysicalDeviceProperties2;
  using ::VkPhysicalDeviceShaderDrawParametersFeatures;
  using ::VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR;
  using ::VkPhysicalDeviceVulkan12Features;
  using ::VkPhysicalDeviceVulkan13Features;
  using ::VkPipeline;
  using ::VkPipelineCache;
  using ::VkPipelineCacheCreateInfo;
  using ::VkPipelineColorBlendAttachmentState;
  using ::VkPipelineColorBlendStateCreateInfo;
  using ::VkPipelineDepthStencilStateCreateInfo;
  using ::VkPipelineDynamicStateCreateInfo;
  using ::VkPipelineInputAssemblyStateCreateInfo;
  using ::VkPipelineLayout;
  using ::VkPipelineLayoutCreateInfo;
  using ::VkPipelineMultisampleStateCreateInfo;
  using ::VkPipelineRasterizationDepthClipStateCreateInfoEXT;
  using ::VkPipelineRasterizationStateCreateInfo;
  using ::VkPipelineRenderingCreateInfoKHR;
  using ::VkPipelineShaderStageCreateInfo;
  using ::VkPipelineStageFlags;
  using ::VkPipelineTessellationStateCreateInfo;
  using ::VkPipelineVertexInputStateCreateInfo;
  using ::VkPipelineViewportStateCreateInfo;
  using ::VkPolygonMode;
  using ::VkPresentInfoKHR;
  using ::VkPresentModeKHR;
  using ::VkPrimitiveTopology;
  using ::VkPushConstantRange;
  using ::VkQueue;
  using ::VkQueueFamilyProperties;
  using ::VkRect2D;
  using ::VkRenderingAttachmentInfo;
  using ::VkRenderingInfo;
  using ::VkRenderPass;
  using ::VkRenderPassBeginInfo;
  using ::VkRenderPassCreateInfo;
  using ::VkResult;
  using ::VkSampleCountFlagBits;
  using ::VkSampleCountFlags;
  using ::VkSampler;
  using ::VkSamplerAddressMode;
  using ::VkSamplerCreateInfo;
  using ::VkSemaphore;
  using ::VkSemaphoreCreateInfo;
  using ::VkSemaphoreTypeCreateInfo;
  using ::VkShaderModule;
  using ::VkShaderModuleCreateInfo;
  using ::VkShaderStageFlags;
  using ::VkSubmitInfo;
  using ::VkSubpassDependency;
  using ::VkSubpassDescription;
  using ::VkSurfaceCapabilitiesKHR;
  using ::VkSurfaceFormatKHR;
  using ::VkSurfaceKHR;
  using ::VkSwapchainCreateInfoKHR;
  using ::VkSwapchainKHR;
  using ::VkTimelineSemaphoreSubmitInfoKHR;
  using ::VkVertexInputAttributeDescription;
  using ::VkVertexInputBindingDescription;
  using ::VkViewport;
  using ::VkWriteDescriptorSet;
  using ::vkFreeCommandBuffers;
  using ::vkGetFenceStatus;
  using ::vkDestroyFence;
  
  // Functions
  using ::vkAcquireNextImageKHR;
  using ::vkAllocateCommandBuffers;
  using ::vkAllocateDescriptorSets;
  using ::vkAllocateMemory;
  using ::vkBeginCommandBuffer;
  using ::vkBindBufferMemory;
  using ::vkBindImageMemory;
  using ::vkCmdBeginDebugUtilsLabelEXT;
  using ::vkCmdBeginRendering;
  using ::vkCmdBeginRenderingKHR;
  using ::vkCmdBeginRenderPass;
  using ::vkCmdBindDescriptorSets;
  using ::vkCmdBindIndexBuffer;
  using ::vkCmdBindPipeline;
  using ::vkCmdBindVertexBuffers;
  using ::vkCmdBlitImage;
  using ::vkCmdClearColorImage;
  using ::vkCmdClearDepthStencilImage;
  using ::vkCmdCopyBuffer;
  using ::vkCmdCopyBufferToImage;
  using ::vkCmdDraw;
  using ::vkCmdDrawIndexed;
  using ::vkCmdEndDebugUtilsLabelEXT;
  using ::vkCmdEndRendering;
  using ::vkCmdEndRenderPass;
  using ::vkCmdPipelineBarrier;
  using ::vkCmdPushConstants;
  using ::vkCmdSetColorBlendEnableEXT;
  using ::vkCmdSetColorBlendEquationEXT;
  using ::vkCmdSetDepthBias;
  using ::vkCmdSetDepthClampEnableEXT;
  using ::vkCmdSetScissor;
  using ::vkCmdSetViewport;
  using ::vkCreateBuffer;
  using ::vkCreateCommandPool;
  using ::vkCreateDebugUtilsMessengerEXT;
  using ::vkCreateDescriptorPool;
  using ::vkCreateDescriptorSetLayout;
  using ::vkCreateDevice;
  using ::vkCreateFence;
  using ::vkCreateFramebuffer;
  using ::vkCreateGraphicsPipelines;
  using ::vkCreateImage;
  using ::vkCreateImageView;
  using ::vkCreateInstance;
  using ::vkCreatePipelineCache;
  using ::vkCreatePipelineLayout;
  using ::vkCreateRenderPass;
  using ::vkCreateSampler;
  using ::vkCreateSemaphore;
  using ::vkCreateShaderModule;
  using ::vkCreateSwapchainKHR;
  using ::vkDestroyBuffer;
  using ::vkDestroyCommandPool;
  using ::vkDestroyDebugUtilsMessengerEXT;
  using ::vkDestroyDescriptorPool;
  using ::vkDestroyDescriptorSetLayout;
  using ::vkDestroyImage;
  using ::vkDestroyImageView;
  using ::vkDestroySampler;
  using ::vkDestroyShaderModule;
  using ::vkDeviceWaitIdle;
  using ::vkEndCommandBuffer;
  using ::vkEnumerateDeviceExtensionProperties;
  using ::vkEnumerateInstanceExtensionProperties;
  using ::vkEnumerateInstanceLayerProperties;
  using ::vkEnumerateInstanceVersion;
  using ::vkEnumeratePhysicalDevices;
  using ::vkFreeMemory;
  using ::vkGetBufferMemoryRequirements;
  using ::vkGetDeviceQueue;
  using ::vkGetImageMemoryRequirements;
  using ::vkGetInstanceProcAddr;
  using ::vkGetPhysicalDeviceFeatures;
  using ::vkGetPhysicalDeviceFormatProperties;
  using ::vkGetPhysicalDeviceMemoryProperties;
  using ::vkGetPhysicalDeviceProperties;
  using ::vkGetPhysicalDeviceProperties2;
  using ::vkGetPhysicalDeviceQueueFamilyProperties;
  using ::vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
  using ::vkGetPhysicalDeviceSurfaceFormatsKHR;
  using ::vkGetPhysicalDeviceSurfacePresentModesKHR;
  using ::vkGetPhysicalDeviceSurfaceSupportKHR;
  using ::vkGetPipelineCacheData;
  using ::vkGetSwapchainImagesKHR;
  using ::vkMapMemory;
  using ::vkQueuePresentKHR;
  using ::vkQueueSubmit;
  using ::vkQueueWaitIdle;
  using ::vkResetCommandBuffer;
  using ::vkResetCommandPool;
  using ::vkResetFences;
  using ::vkUnmapMemory;
  using ::vkUpdateDescriptorSets;
  using ::vkWaitForFences;
}
