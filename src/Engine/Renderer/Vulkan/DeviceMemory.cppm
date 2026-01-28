module;
#include <volk.h>

export module Engine.Renderer.VulkanDeviceMemory;

import std;

namespace Poulpe
{
  export class DeviceMemory
  {
  public:
    DeviceMemory(
      VkDevice device,
      VkMemoryPropertyFlags memory_type,
      VkDeviceSize max_size,
      std::uint32_t index,
      VkDeviceSize alignment
    );

    std::uint32_t bindBufferToMemory(VkBuffer& buffer, VkDeviceSize const offset) __attribute__((no_thread_safety_analysis));
    void bindImageToMemory(VkImage& image, VkDeviceSize const offset) __attribute__((no_thread_safety_analysis));
    void clear();
    VkDeviceSize getID() const;
    VkDeviceMemory* getMemory();
    VkDeviceSize getOffset() const;
    VkDeviceSize getSize() const;
    VkDeviceSize getSpaceLeft() const;
    VkMemoryPropertyFlags getType() const;
    bool hasEnoughSpaceLeft(VkDeviceSize size) __attribute__((no_thread_safety_analysis));
    bool isFull() const;
    void lock();
    void unLock();
    VkBuffer& getBuffer(std::size_t index);
    std::size_t getOffset(std::size_t index) const;

  private:
    void allocateToMemory() __attribute__((no_thread_safety_analysis));

    std::uint32_t _index{0};

    bool _is_allocated{false};
    bool _is_full{false};

    VkDeviceSize _alignment;
    VkDeviceSize _max_size;
    VkMemoryPropertyFlags _memory_type;
    //@todo check with deviceProps.limits.bufferImageGranularity;
    VkDeviceSize _offset{0};
    std::uint32_t _buffer_count{ 0 };
    std::vector<VkDeviceSize> _buffer_offsets;
    std::vector<VkBuffer> _buffer;
    VkDevice _device;
    std::unique_ptr<VkDeviceMemory> _memory;
    std::mutex _mutex_memory;
  };
}
