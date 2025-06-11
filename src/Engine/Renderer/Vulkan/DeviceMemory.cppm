module;

#include <volk.h>

#include <memory>
#include <mutex>
#include <stdexcept>
#include <vector>

export module Engine.Renderer.VulkanDeviceMemory;

namespace Poulpe
{
  export class DeviceMemory
  {
  public:
    DeviceMemory(
      VkDevice device,
      VkMemoryPropertyFlags memory_type,
      VkDeviceSize max_size,
      uint32_t index,
      VkDeviceSize alignment
    );

    uint32_t bindBufferToMemory(VkBuffer& buffer, VkDeviceSize const offset);
    void bindImageToMemory(VkImage& image, VkDeviceSize const offset);
    void clear();
    VkDeviceSize getID() const;
    VkDeviceMemory* getMemory();
    VkDeviceSize getOffset() const;
    VkDeviceSize getSize() const;
    VkDeviceSize getSpaceLeft() const;
    VkMemoryPropertyFlags getType() const;
    bool hasEnoughSpaceLeft(VkDeviceSize size);
    bool isFull() const;
    void lock();
    void unLock();
    VkBuffer& getBuffer(size_t index);
    size_t getOffset(size_t index) const;

  private:
    void allocateToMemory();

    uint32_t _index{0};

    bool _is_allocated{false};
    bool _is_full{false};

    VkDeviceSize _alignment;
    VkDeviceSize _max_size;
    VkMemoryPropertyFlags _memory_type;
    //@todo check with deviceProps.limits.bufferImageGranularity;
    VkDeviceSize _offset{0};
    uint32_t _buffer_count{ 0 };
    std::vector<VkDeviceSize> _buffer_offsets;
    std::vector<VkBuffer> _buffer;
    VkDevice _device;
    std::unique_ptr<VkDeviceMemory> _memory;
    std::mutex _mutex_memory;
  };
}
