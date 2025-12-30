module;


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

#include <volk.h>

export module Engine.Renderer.VulkanRenderer;

import std;

import Engine.Renderer.VulkanDeviceMemoryPool;
import Engine.Renderer.VulkanAPI;

import Engine.Component.Camera;

import Engine.Core.PlpTypedef;

import Engine.GUI.Window;

import Engine.Renderer.Vulkan.Mesh;

namespace Poulpe
{
  export struct DrawCommands {

    public:
      std::vector<VkCommandBuffer> cmd_buffers{};
      std::vector<std::vector<VkPipelineStageFlags>> stage_flags{};
      std::vector<bool> is_attachments{ };

      DrawCommands(std::size_t const size)
        : _size(size)
      {
        init();
      }

      void insert(
        VkCommandBuffer& cmd_buffer,
        uint32_t const thread_id,
        bool const is_attachment,
        std::vector<VkPipelineStageFlags> flags = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT })
      {
        {
          //std::lock_guard<std::mutex> guard(_m);
          cmd_buffers[thread_id] = cmd_buffer;
          stage_flags[thread_id] = flags;
          is_attachments[thread_id] = is_attachment;
        }
      }

      void clear()
      {
        cmd_buffers.clear();
        stage_flags.clear();

        init();
      }

      bool has_cmd() const
      {
        bool has_cmd{ false };

        std::ranges::for_each(cmd_buffers, [&has_cmd](auto const& cmd) {
          if (cmd != nullptr) {
            has_cmd = true;
          }
        });
        return has_cmd;
      }
    private:

      void init()
      {
        cmd_buffers.resize(_size, nullptr);
        stage_flags.resize(_size, { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT });
        is_attachments.resize(_size, false);
      }

      std::mutex _m;
      std::size_t _size;
  };

  export struct RendererInfo
  {
    Mesh* mesh;
    Camera* camera{};
    glm::mat4 camera_view{};
    Light sun_light{};
    std::span<Light, std::dynamic_extent> point_lights{};
    std::span<Light, std::dynamic_extent> spot_lights{};
    double elapsed_time{0.0};
    VkShaderStageFlags stage_flag_bits;
    bool normal_debug{};
    bool has_alpha_blend {false};
    std::uint32_t env_options{}; //env options, see below
  };

  /** env options config :
    HAS_FOG << 0
  **/

  export enum class SHADOW_TYPE {
    SPOT_LIGHT,
    POINT_LIGHT,
    CSM
  };

  export class Renderer
  {
  public:

    Renderer(Window* const window);
    ~Renderer()  = default;

    void addPipeline(
      std::string const& shaderName,
      VulkanPipeline& pipeline);

    void clear();
    void destroy();

    void start();

    void startRender();
    void startShadowMap(SHADOW_TYPE const shadow_type);

    void draw(RendererInfo const& renderer_info);
    void drawShadowMap(
      RendererInfo const& renderer_info,
      SHADOW_TYPE const shadow_type);

    void endRender();
    void endShadowMap(SHADOW_TYPE const shadow_type);

    void endRendering(
      VkCommandBuffer& cmd_buffer,
      VkImage& image,
      VkImage& depth_image,
      bool const is_attachment,
      bool const has_depth_attachment = true);

    void submit();

    inline uint32_t getCurrentFrameIndex() const { return _current_frame; }
    inline VkSampler& getCurrentSampler() { return _samplers[_previous_frame]; }
    inline VkImageView& getCurrentImageView() { return _imageviews[_previous_frame]; }
    inline VkImageView& getDepthMapImageViews() { return  _depthmap_imageviews.at(_current_frame); }
    inline VkSampler& getDepthMapSamplers() { return _depthmap_samplers.at(_current_frame); }
    inline VkImageView& getDepthImageViews() { return _depth_imageviews.at(_current_frame); }
    inline VkImageView& getDepthImageViews2() { return _depth_imageviews2.at(_current_frame); }
    inline VkSampler& getDepthSamplers() { return  _depth_samplers.at(_current_frame); }
    inline VkSampler& getDepthSamplers2() { return  _depth_samplers2.at(_current_frame); }
    inline VkImageView& getVisibleDepthImageView() { return _visible_depth_imageview; }
    inline VkSampler& getCSMSamplers() { return _csm_samplers.at(_current_frame); }
    inline VkImageView& getCSMImageViews() { return _csm_imageviews.at(_current_frame); }
    inline std::vector<VkDescriptorSetLayout>* getDescriptorSetLayouts() { return & _descriptorset_layouts; }
    inline VkDevice getDevice()  { return _vulkan->getDevice(); }
    inline glm::mat4 getPerspective() { return _perspective; }
    VulkanPipeline* getPipeline(std::string const & shaderName) { return & _pipelines[shaderName]; }
    void immediateSubmit(std::function<void(VkCommandBuffer cmd)> && function, int queueIndex = 0) ;
    void init();
    //void renderScene();

    void setDeltatime(float const delta_time);

    void setRayPick(
      float const x,
      float const y,
      float const z,
      int const width,
      int const height);

    VulkanAPI * getAPI() const { return _vulkan.get(); }

    void clearScreen();

  private:
    const uint32_t _max_frames_in_flight{ 2 };
    //const std::size_t _max_render_thread{ 4 };

    void onFinishRender();
    void setPerspective();

  private:
    std::unique_ptr<VulkanAPI> _vulkan{ nullptr };
    VkSwapchainKHR _swapchain{ nullptr };

    std::vector<VkImage> _images{};
    std::vector<VkImageView> _imageviews{};
    std::vector<VkImage> _images2{};
    std::vector<VkImageView> _imageviews2{};
    std::vector<VkImage> _images3{};
    std::vector<VkImageView> _imageviews3{};

    VkCommandPool _cmd_pool_entities{ nullptr };
    VkCommandPool _cmd_pool_entities2{ nullptr };
    VkCommandPool _cmd_pool_entities3{ nullptr };
    VkCommandPool _cmd_pool_entities4{ nullptr };

    std::vector<VkCommandBuffer> _cmd_buffer_entities{};
    std::vector<VkCommandBuffer> _cmd_buffer_entities2{};
    std::vector<VkCommandBuffer> _cmd_buffer_entities3{};
    std::vector<VkCommandBuffer> _cmd_buffer_entities4{};

    VkCommandPool _cmd_pool_shadowmap{ nullptr };
    std::vector<VkCommandBuffer> _cmd_buffer_shadowmap{};

    uint32_t _current_frame{ 0 };
    uint32_t _previous_frame{ 0 };
    uint32_t _image_index;
    std::pair<std::vector<VkBuffer>, std::vector<VkDeviceMemory>> _unifor_buffers{};

    //@todo move to meshManager
    std::vector<VkImageView> _depth_imageviews{};
    std::vector<VkImage> _depth_images{};
    std::vector<VkImageView> _depth_imageviews2{};
    std::vector<VkImage> _depth_images2{};
    std::vector<VkImageView> _depth_imageviews3{};
    std::vector<VkImage> _depth_images3{};

    VkImageView _visible_depth_imageview;
    VkImage _visible_depth_image;

    glm::mat4 _perspective;
    //glm::mat4 _lastLookAt;
    float _delta_time{ 0.0f };
    std::vector<VkDescriptorPool>_descriptor_pools;
    std::vector<VkDescriptorSetLayout>_descriptorset_layouts;

    std::vector<VkSampler> _samplers{};
    std::vector<VkSampler> _depth_samplers{};
    std::vector<VkSampler> _samplers2{};
    std::vector<VkSampler> _depth_samplers2{};
    std::vector<VkSampler> _samplers3{};
    std::vector<VkSampler> _depth_samplers3{};

    VkSampler _visible_depth_sampler{};

    //std::vector<IObserver*> _Observers{};

    std::vector<VkImage> _depthmap_images;
    std::vector<VkImageView> _depthmap_imageviews;//sampling
    std::vector<VkImageView> _depthmap_imageviews_rendering;
    std::vector<VkSampler> _depthmap_samplers;

    std::vector<VkImage> _csm_images;
    std::vector<VkImageView> _csm_imageviews;
    std::vector<VkImageView> _csm_imageviews_rendering;
    std::vector<VkSampler> _csm_samplers;

    std::vector<VkImage> _depthmap_Images3;
    std::vector<VkImageView> _depthmap_imageviews3;
    std::vector<VkSampler> _depthmap_samplers3;
    bool _update_shadow_map { true };

    std::unordered_map<std::string, VulkanPipeline> _pipelines;

    std::vector<VkSemaphore> _image_available;
    std::vector<VkSemaphore> _sema_present_completes;
    std::vector<std::vector<VkSemaphore>> _sema_render_completes;

    std::vector<VkFence> _images_in_flight{};
    std::vector<VkFence> _fences_in_flight{};

    std::mutex _mutex_queue_submit;

    DrawCommands _draw_cmds{4};

    std::vector<VkSemaphore> _timeline_semaphores;
    std::vector<std::uint64_t> _current_timeline_values;
  };
}
