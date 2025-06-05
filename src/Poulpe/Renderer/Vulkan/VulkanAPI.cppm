module;

#include <GLFW/glfw3.h>
#include <ktx.h>
#include <stb_image.h>
#include <volk.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>

export module Poulpe.Renderer.VulkanAPI;

import Poulpe.Renderer.VulkanDeviceMemoryPool;
import Poulpe.Renderer.VulkanDeviceMemory;

import Poulpe.Component.Vertex;
import Poulpe.Core.Logger;
import Poulpe.Core.MeshTypes;
import Poulpe.Core.PlpTypedef;
import Poulpe.GUI.Window;

namespace Poulpe
{
  export VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerCreateInfoEXT const * pCreateInfo,
    VkAllocationCallbacks const * pAllocator,
    VkDebugUtilsMessengerEXT* pCallback);

  export struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  export struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  export class VulkanAPI
  {
  public:
    VulkanAPI(Window* window);
    ~VulkanAPI();

    /**
    * Vulkan init functions, before main loop.
    **/
    VkRenderPass* createRenderPass(VkSampleCountFlagBits const& msaaSamples);

    VkShaderModule createShaderModule(std::vector<char> const& code);

    VkDescriptorPool createDescriptorPool(
      std::vector<VkDescriptorPoolSize> const& pool_sizes,
      uint32_t const max_sets = 100,
      VkDescriptorPoolCreateFlags const flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);

    VkDescriptorSetLayout createDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> const& pBindings);

    VkDescriptorSet createDescriptorSets(
      VkDescriptorPool const& descriptor_pool,
      std::vector<VkDescriptorSetLayout> const& descset_layout,
      uint32_t const count = 100);

    void updateDescriptorSets(
      std::vector<Buffer>& uniform_buffers,
      VkDescriptorSet& descset,
      std::vector<VkDescriptorImageInfo>& image_info,
      VkDescriptorType const type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    void updateDescriptorSets(
      std::vector<Buffer>& uniformBuffers,
      std::vector<Buffer>& storageBuffers,
      VkDescriptorSet& descriptorSet,
      std::vector<VkDescriptorImageInfo>& imageInfo,
      std::vector<VkDescriptorImageInfo>& depth_map_image_info,
      std::vector<VkDescriptorImageInfo>& cube_map_image_info);

    void updateDescriptorSet(
      Buffer& uniform_buffer,
      VkDescriptorSet& descset,
      std::vector<VkDescriptorImageInfo>& image_info,
      VkDescriptorType const type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    void updateDescriptorSet(
      Buffer& uniform_buffer,
      Buffer& storage_buffer,
      VkDescriptorSet& descset,
      std::vector<VkDescriptorImageInfo>& image_info);

    void updateStorageDescriptorSets(
      std::vector<Buffer>& uniform_buffers,
      VkDescriptorSet& desc_set,
      VkDescriptorType const type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    VkPipelineLayout createPipelineLayout(
      std::vector<VkDescriptorSetLayout> const& descset_layouts,
      std::vector<VkPushConstantRange> const& push_const);

    VkPipeline createGraphicsPipeline(PipeLineCreateInfo const& pipeline_create_info);

    VkSwapchainKHR createSwapChain(
      std::vector<VkImage>& swapchain_images,
      VkSwapchainKHR const& old_swapchain = VK_NULL_HANDLE);

    std::vector<VkFramebuffer> createFramebuffers(
      VkRenderPass& rdr_pass,
      std::vector<VkImageView>& swapchain_image_views,
      std::vector<VkImageView>& depth_imageview,
      std::vector<VkImageView>& color_image_view);

    VkCommandPool createCommandPool();

    std::vector<VkCommandBuffer> allocateCommandBuffers(
      VkCommandPool& cmd_pool,
      uint32_t const size = 1,
      bool const is_secondary = false);

    VkBuffer createBuffer(VkDeviceSize const size, VkBufferUsageFlags const usage);

    void createBuffer(
      VkDeviceSize const size,
      VkBufferUsageFlags const usage,
      VkMemoryPropertyFlags const properties,
      VkBuffer& buffer,
      VkDeviceMemory& device_memory);

    Buffer createVertexBuffer(
      VkCommandPool& commandPool,
      std::vector<Vertex> const& vertices);

    void updateVertexBuffer(
      VkCommandPool& commandPool,
      std::vector<Vertex> const& new_vertices,
      VkBuffer& buffer_to_update);

    Buffer createVertex2DBuffer(
      VkCommandPool& cmd_pool,
      std::vector<Vertex2D> const& vertices);

    Buffer createIndexBuffer(
      VkCommandPool& cmd_pool,
      std::vector<uint32_t> const& indices);

    VkImageMemoryBarrier setupImageMemoryBarrier(
      VkImage& image,
      VkAccessFlags const src_access_mask,
      VkAccessFlags const dst_access_mask,
      VkImageLayout const old_layout,
      VkImageLayout const new_layout,
      uint32_t const mip_lvl = VK_REMAINING_MIP_LEVELS,
      VkImageAspectFlags const aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT);

    void copyBuffer(
      VkCommandPool& cmd_pool,
      VkBuffer& src_buffer,
      VkBuffer& dst_buffer,
      VkDeviceSize const size,
      VkDeviceSize dst_offset = 0,
      size_t const queue_index = 0);

    bool souldResizeSwapChain();

    Buffer createUniformBuffers(
      uint64_t const uniform_buffers_count,
      VkCommandPool& cmd_pool);

    Buffer createCubeUniformBuffers(uint32_t const uniform_buffers_count);

    void updateUniformBuffer(Buffer& buffer, std::vector<UniformBufferObject>* uniform_buffer_objects);

    void createImage(
      uint32_t const width,
      uint32_t const height,
      uint32_t const mip_lvl,
      VkSampleCountFlagBits const num_samples,
      VkFormat const format,
      VkImageTiling const tiling,
      VkImageUsageFlags const usage,
      VkMemoryPropertyFlags const properties,
      VkImage& image);

    VkImageView createImageView(
      VkImage& image,
      VkFormat const format,
      uint32_t const mip_lvl,
      uint32_t scale,
      VkImageAspectFlags const aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT);

    void createTextureImage(
      VkCommandBuffer& cmd_buffer,
      stbi_uc* pixels,
      uint32_t const tex_width,
      uint32_t const tex_height,
      uint32_t const mip_lvl,
      VkImage& texture_image,
      VkFormat const format,
      uint32_t const scale);

    void copyBufferToImage(
      VkCommandBuffer& cmd_buffer,
      VkBuffer& buffer, VkImage& image,
      uint32_t const width,
      uint32_t const height,
      uint32_t const mipLevels = 0);

    VkSampler createTextureSampler(uint32_t const mip_lvl);
    VkImageView createDepthResources(VkCommandBuffer& cmd_buffer);

    VkFormat findSupportedFormat(
      std::vector<VkFormat>const& candidates,
      VkImageTiling const tiling,
      VkFormatFeatureFlags const features);

    VkFormat findDepthFormat();

    bool hasStencilComponent(VkFormat const format);

    VkDeviceSize getMaxMemoryHeap() { return _max_memory_heap; }

    void initMemoryPool();

    Buffer createStorageBuffers(
      ObjectBuffer const& storage_buffer,
      VkCommandPool& command_pool);

    Buffer createIndirectCommandsBuffer(
      std::vector<VkDrawIndexedIndirectCommand> const& drawCommands);

    void updateStorageBuffer(Buffer& buffer, ObjectBuffer& object_buffer);

    void setResolution(uint32_t const width, uint32_t const height);

    /**
    * Vulkan drawing functions, in main loop
    **/
    void resetCommandPool(VkCommandPool& commandPool);

    void beginCommandBuffer(
      VkCommandBuffer& cmd_buffer,
      VkCommandBufferUsageFlagBits const flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
      VkCommandBufferInheritanceInfo const inheritance_info = {});

    void endCommandBuffer(VkCommandBuffer& cmd_buffer);

    void beginRenderPass(
      VkRenderPass& rdr_pass,
      VkCommandBuffer& cmd_buffer,
      VkFramebuffer& frame_buffer);

    void endRenderPass(VkCommandBuffer& cmd_buffer);

    void beginRendering(
      VkCommandBuffer& cmd_buffer,
      VkImageView& color_imageview,
      VkImageView& depth_imageview,
      VkAttachmentLoadOp const load_op,
      VkAttachmentStoreOp const store_op,
      VkImageLayout const color_image_layout,
      bool const has_depth_attachment);

    void endRendering(VkCommandBuffer& cmd_buffer);

    void setViewPort(VkCommandBuffer& cmd_buffer);

    void setScissor(VkCommandBuffer& cmd_buffer);

    void bindPipeline(VkCommandBuffer& cmd_buffer, VkPipeline& pipeline);

    void draw(
      VkCommandBuffer& cmd_buffer,
      Data const * data,
      bool const is_indexed = true,
      uint32_t const index = 0);

    VkResult queueSubmit(VkCommandBuffer& cmd_buffer, size_t const queue_index = 0);

    void submit(
      VkQueue& queue,
      std::vector<VkSubmitInfo> const& submit_infos,
      VkPresentInfoKHR const& present_info,
      VkFence& fence);

    void addPipelineBarriers(
      VkCommandBuffer& cmd_buffer,
      std::vector<VkImageMemoryBarrier>& rdr_barriers,
      VkPipelineStageFlags const src_stage_mask,
      VkPipelineStageFlags const dst_stage_mask,
      VkDependencyFlags const depency_flags);

    void generateMipmaps(
      VkCommandBuffer& cmd_buffer,
      VkFormat const image_format,
      VkImage& image,
      uint32_t const tex_width,
      uint32_t const tex_height,
      uint32_t const mip_lvl,
      uint32_t const layer_count = 1);

    //@todo redo all detroying mechanism...
    void destroy();

    /*
    * Helper functions.
    */
    inline const std::vector<const char*> getValidationLayers() const { return _validation_layers; }

    inline const std::vector<VkExtensionProperties> getExtensions() const { return _extensions; }

    inline const std::vector<VkLayerProperties> getLayersAvailable() const { return _layers_available; }

    inline bool isInstanceCreated() const { return _instance_created; }

    inline bool isValidationLayersEnabled() const { return _enable_validation_layers; }

    inline uint32_t getExtensionCount() const { return _extension_count; }

    inline uint32_t getQueueFamily() const { return _queue_family_indices.graphicsFamily.value(); }

    inline VkInstance getInstance() const { return _instance; }

    inline VkPhysicalDevice getPhysicalDevice() const { return _physical_device; }

    inline VkDevice getDevice() const { return _device; }

    inline std::vector<VkQueue> getGraphicsQueues() const { return _graphics_queues; }

    inline VkPhysicalDeviceProperties getDeviceProperties() const { return _device_props; }

    inline VkPhysicalDeviceFeatures getDeviceFeatures() const { return _device_features; }

    inline bool isFramebufferResized() { return false; }

    inline VkExtent2D getSwapChainExtent() const { return _swapchain_extent; }

    inline VkSurfaceKHR getSurface() const { return _surface; }

    inline VkSurfaceFormatKHR getSurfaceFormat() const { return _surface_format; }

    inline void resetCurrentFrameIndex() { _current_frame = 0; }

    inline int32_t getCurrentFrame() const { return _current_frame; }

    inline VkFormat getSwapChainImageFormat() const { return _swapchain_iImage_format; }

    inline VkSampleCountFlagBits getMsaaSamples() const { return _sample_count; }

    void initDetails();

    VkSampleCountFlagBits getMaxUsableSampleCount();

    const SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

    uint32_t getImageCount() const;

    std::string getAPIVersion();

    DeviceMemoryPool* getDeviceMemoryPool() { return _device_memory_pool.get(); }

    void startMarker(
      VkCommandBuffer& buffer,
      std::string const& name,
      float const r,
      float const g,
      float const b,
      float const a = 1.0f);

    void endMarker(VkCommandBuffer& buffer);

    uint32_t getQueueCount() { return _queue_count; }

    std::vector<VkQueue> getPresentQueue() { return _present_queues; }

    void waitIdle();

    void createDepthMapImage(VkImage& image);

    VkImageView createDepthMapImageView(VkImage& image);

    VkSampler createDepthMapSampler();

    void createDepthMapFrameBuffer(
      VkRenderPass& rdr_pass,
      VkImageView& imageview,
      VkFramebuffer& frame_buffer);

    static const std::string getVendor(int vendorID)
    {
      std::unordered_map<int, std::string> vendors;
      vendors[0x1002] = "AMD";
      vendors[0x1010] = "ImgTec";
      vendors[0x10DE] = "NVIDIA";
      vendors[0x13B5] = "ARM";
      vendors[0x5143] = "Qualcomm";
      vendors[0x8086] = "INTEL";

      return vendors[vendorID];
    }

    void transitionImageLayout(
      VkCommandBuffer& cmd_buffer,
      VkImage& image,
      VkImageLayout const old_layout,
      VkImageLayout const new_layout,
      VkImageAspectFlags const aspect_flags);


    //KTX
    void createKTXImage(
      VkCommandBuffer& cmd_buffer,
      ktxTexture2* ktx_texture,
      VkImage& image);

    VkImageView createKTXImageView(
      ktxTexture2* ktx_texture,
      VkImage& image,
      VkImageAspectFlags aspect_flags);

    //Font
    void createFontImage(
      VkCommandBuffer& cmd_buffer,
      std::vector<FontCharacter> const& characters,
      uint32_t const width,
      uint32_t const height,
      VkImage& image);

    VkImageView createFontImageView(
      VkImage& image,
      VkImageAspectFlags aspect_flags);

    VkSamplerAddressMode getSamplerAddressMode(TextureWrapMode mode);
    VkSampler createKTXSampler(
      TextureWrapMode const wrap_mode_u,
      TextureWrapMode const wrap_mode_v,
      uint32_t const mip_lvl);

  public:
    //bool _FramebufferResized = false;

    //VK_COLOR_SPACE_HDR10_ST2084_EXT
    VkColorSpaceKHR PLP_VK_COLOR_SPACE{ VK_COLOR_SPACE_HDR10_ST2084_EXT };

    //https://panoskarabelas.com/blog/posts/hdr_in_under_10_minutes/
    //VK_FORMAT_A2B10G10R10_UNORM_PACK32
    VkFormat PLP_VK_FORMAT_SURFACE { VK_FORMAT_A2B10G10R10_UNORM_PACK32 };
    VkFormat PLP_VK_FORMAT_COLOR { VK_FORMAT_A2B10G10R10_UNORM_PACK32 };
    //VK_FORMAT_D16_UNORM, VK_FORMAT_D32_SFLOAT
    VkFormat PLP_VK_FORMAT_DEPTH { VK_FORMAT_D32_SFLOAT }; //VK_FORMAT_D16_UNORM
    //VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
    VkFormat PLP_VK_FORMAT_DEPTH_STENCIL { VK_FORMAT_D32_SFLOAT_S8_UINT };
    VkFormat PLP_VK_FORMAT_FONT{ VK_FORMAT_R8_UNORM };

  private:
    bool isDeviceSuitable(VkPhysicalDevice& device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice& device);
    bool checkValidationLayerSupport();
    void createInstance();
    void enumerateExtensions();
    void loadRequiredExtensions();
    void setupDebugMessenger();
    void createLogicalDevice();
    void pickPhysicalDevice();
    void createSurface();

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice& device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR> const& availablePresentModes);
    uint32_t findMemoryType(uint32_t const type_filter, VkMemoryPropertyFlags const properties);
    VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR const& capabilities);

    VkDeviceSize align_to(VkDeviceSize const size, VkDeviceSize const alignment);

    int32_t _current_frame{ 0 };
    uint32_t _extension_count;
    std::string _api_version;
    const uint32_t _queue_count{ 1 };

    Window* _window{ nullptr };

    const std::vector<const char*> _validation_layers{ "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> _device_extensions{
        VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
        VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME,
        VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME
    };

    bool _instance_created{ false };
    bool _enable_validation_layers{ false };

    VkInstance _instance = VK_NULL_HANDLE;
    VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties _device_props;
    VkPhysicalDeviceFeatures _device_features;
    VkPhysicalDeviceProperties2 _device_props2;
    VkPhysicalDeviceMaintenance3Properties _device_maintenance3_props;

    VkDevice _device{ VK_NULL_HANDLE };
    std::vector<VkQueue> _graphics_queues{};
    std::vector<VkQueue> _present_queues{};
    VkSurfaceKHR _surface{ VK_NULL_HANDLE };
    VkFormat _swapchain_iImage_format{};
    VkExtent2D _swapchain_extent{};
    VkDebugUtilsMessengerEXT _debug_msg_callback{ VK_NULL_HANDLE };
    QueueFamilyIndices _queue_family_indices{};
    VkSurfaceFormatKHR _surface_format{};
    VkPresentModeKHR _present_mode{};
    SwapChainSupportDetails _swapchain_support{};

    std::vector<VkLayerProperties> _layers_available{};
    std::vector<VkExtensionProperties> _extensions{};
    std::vector<const char*> _required_extensions{};
    std::vector<VkFence> _in_flight_fences{};
    std::vector<VkFence> _images_in_flight{};

    VkSampleCountFlagBits _sample_count{ VK_SAMPLE_COUNT_8_BIT };

    std::mutex _mutex_queue_submit{};

    VkDeviceSize _max_memory_heap{};
    std::unique_ptr<DeviceMemoryPool> _device_memory_pool{ nullptr };

    VkFence _fence_acquire_image{};
    VkFence _fence_submit{};
    VkFence _fence_buffer{};

    //@todo move to config file
    //uint32_t _width{ 800 };
    //uint32_t _height{ 600 };
      uint32_t _width{ 2560 };
      uint32_t _height{ 1440 };

    //VkMemoryRequirements _MemRequirements;
  };
}
