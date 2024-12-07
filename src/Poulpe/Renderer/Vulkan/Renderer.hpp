#pragma once
#pragma once

#include "Poulpe/Component/Camera.hpp"

#include "Poulpe/Core/PlpTypedef.hpp"

#include "Poulpe/GUI/Window.hpp"

#include "Poulpe/Manager/EntityManager.hpp"
#include "Poulpe/Manager/LightManager.hpp"
#include "Poulpe/Manager/TextureManager.hpp"

#include "Poulpe/Renderer/Vulkan/VulkanAPI.hpp"

#include <future>
#include <latch>
#include <stb_image.h>
#include <vulkan/vulkan.h>

namespace Poulpe
{
  struct DrawCommands {

    public:
      std::vector<VkCommandBuffer*> cmd_buffers{};
      std::vector<std::vector<VkPipelineStageFlags>> stage_flags{};
      std::vector<VkSemaphore*> semaphores{};

      DrawCommands(size_t const size)
        : _size(size)
      {
        init();
      }

      void insert(
        VkCommandBuffer* cmd_buffer,
        VkSemaphore* semaphore,
        unsigned int const thread_id,
        std::vector<VkPipelineStageFlags> flags = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT })
      {
        {
          std::lock_guard guard(_m);
          cmd_buffers[thread_id] = cmd_buffer;
          stage_flags[thread_id] = flags;
          semaphores[thread_id] = semaphore;
        }
      }

      void clear()
      {
        cmd_buffers.clear();
        stage_flags.clear();
        semaphores.clear();

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
        semaphores.resize(_size, nullptr);
      }

      std::mutex _m;
      size_t _size;
  };

  class ComponentManager;
  class DeviceMemoryPool;
  class Entity;
  class Vertex2D;
  class VulkanPipeline;

  class Renderer: public std::enable_shared_from_this<Renderer>
  {

  public:

    Renderer(
      Window* const window,
      EntityManager* const entity_manager,
      ComponentManager* const component_manager,
      LightManager* const light_manager,
      TextureManager* const texture_manager
    );
    ~Renderer()  = default;

    inline void addCamera(Camera* const camera) { _camera = camera; }
    void addEntities(std::vector<Entity*> entities) ;
    void addEntity(Entity* entity, bool const is_last) ;
    
    void updateData(
      std::string const& name,
      UniformBufferObject const& ubo,
      std::vector<Vertex> const& vertices) ;
    
    void addPipeline(
      std::string const& shaderName,
      VulkanPipeline& pipeline) ;
    
    void beginRendering(
      VkCommandBuffer& cmd_buffer,
      VkImageView& imageview,
      VkImage& image,
      VkImageView& depth_imageview,
      VkImage& depthImage,
      VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
      VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      bool const has_depth_attachment = true,
      bool continuousCmdBuffer = false);

    void clear();
    void destroy();

    void draw(
      VkCommandBuffer& cmd_buffer,
      DrawCommands& draw_cmds,
      VkImageView& colorview,
      VkImage& color,
      VkImageView& depthview,
      VkImage& depth,
      std::vector<Entity*> const& entities,
      VkAttachmentLoadOp const load_op,
      VkAttachmentStoreOp const store_op,
      std::latch& count_down,
      unsigned int const thread_id,
      bool const has_depth_attachment = true);

    void drawShadowMap(
      VkCommandBuffer& cmd_buffer,
      DrawCommands& draw_cmds,
      VkImageView& depthview,
      VkImage& depth,
      std::vector<Entity*> const& entities,
      std::latch& count_down,
      unsigned int const thread_id);

    void endRendering(
      VkCommandBuffer& cmd_buffer,
      VkImage& image,
      VkImage& depth_image,
      bool const has_depth_attachment = true);

    inline Camera* getCamera() { return _camera; }
    inline uint32_t getCurrentFrameIndex() const { return _current_frame; }
    inline VkImageView getDepthMapImageViews()  { return  _depthmap_imageviews.at(_current_frame); }
    inline VkSampler getDepthMapSamplers()  { return _depthmap_samplers.at(_current_frame); }
    inline VkImageView getDepthImageViews()  { return _depth_imageviews.at(_current_frame); }
    inline VkSampler getDepthSamplers()  { return  _depth_samplers.at(_current_frame); }
    inline std::vector<VkDescriptorSetLayout>* getDescriptorSetLayouts()  { return & _descriptorset_layouts; }
    inline VkDevice getDevice()  { return _vulkan->getDevice(); }
    inline glm::mat4 getPerspective()  { return _perspective; }
    VulkanPipeline* getPipeline(std::string const & shaderName)  { return & _pipelines[shaderName]; }
    void immediateSubmit(std::function<void(VkCommandBuffer cmd)> && function, int queueIndex = 0) ;
    void init();
    void renderScene();
    
    void setDeltatime(float const delta_time);

    void setRayPick(
      float const x,
      float const y,
      float const z,
      int const width,
      int const height);
    
    void showGrid(bool const show);

    VulkanAPI * const getAPI() const { return _vulkan.get(); }

  private:
    const uint32_t _MAX_FRAMES_IN_FLIGHT{ 2 };
    const size_t _MAX_RENDER_THREAD{ 3 };

    void onFinishRender();
    void setPerspective();
    void submit(DrawCommands const& drawCmds);

    void swapBufferEntities();

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

    std::vector<VkCommandBuffer> _cmd_buffer_entities{};
    std::vector<VkCommandBuffer> _cmd_buffer_entities2{};
    std::vector<VkCommandBuffer> _cmd_buffer_entities3{};

    VkCommandPool _cmd_pool_shadowmap{ nullptr };
    std::vector<VkCommandBuffer> _cmd_buffer_shadowmap{};

    uint32_t _current_frame{ 0 };
    uint32_t _image_index;
    std::pair<std::vector<VkBuffer>, std::vector<VkDeviceMemory>> _unifor_buffers{};

    Camera* _camera{ nullptr };
    Window* _window{ nullptr };
    EntityManager* _entity_manager{ nullptr };
    ComponentManager* _component_manager{ nullptr };
    LightManager* _light_manager{ nullptr };
    TextureManager* _texture_manager{ nullptr };

    //@todo move to meshManager
    std::vector<VkImageView> _depth_imageviews{};
    std::vector<VkImage> _depth_images{};
    std::vector<VkImageView> _depth_imageviews2{};
    std::vector<VkImage> _depth_images2{};
    std::vector<VkImageView> _depth_imageviews3{};
    std::vector<VkImage> _depth_images3{};

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

    //std::vector<IObserver*> _Observers{};

    std::vector<VkImage> _depthmap_images;
    std::vector<VkImageView> _depthmap_imageviews;
    std::vector<VkSampler> _depthmap_samplers;

    std::vector<VkImage> _depthmap_images2;
    std::vector<VkImageView> _depthmap_imageviews2;
    std::vector<VkSampler> _depthmap_samplers2;

    std::vector<VkImage> _depthmap_Images3;
    std::vector<VkImageView> _depthmap_imageviews3;
    std::vector<VkSampler> _depthmap_samplers3;
    bool _update_shadow_map { true };

    std::unordered_map<std::string, VulkanPipeline> _pipelines;

    std::vector<VkSemaphore> _image_available;
    std::vector<VkSemaphore> _shadowmap_sema_img_available;

    std::vector<Entity*> _entities{};
    std::vector<Entity*> _entities_buffer{};
    unsigned int const _entities_buffer_swap_treshold{ 50 };
    bool _force_entities_buffer_swap{ false };
    
    std::vector<VkSemaphore> _entities_sema_finished;
    
    std::vector<VkFence> _images_in_flight{};
    std::vector<VkFence> _fences_in_flight{};

    std::mutex _mutex_queue_submit;
    std::mutex _mutex_entity_submit;

    DrawCommands _draw_cmds{3};
  };
}
