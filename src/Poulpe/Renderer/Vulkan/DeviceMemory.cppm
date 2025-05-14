module;

#include <volk.h>

#include <memory>
#include <mutex>
#include <stdexcept>
#include <vector>

export module Poulpe.Renderer.Vulkan.DeviceMemory;

namespace Poulpe
{
  export class DeviceMemory
  {
  public:
    DeviceMemory(
      VkDevice device,
      VkMemoryPropertyFlags memory_type,
      VkDeviceSize max_size,
      unsigned int index,
      VkDeviceSize alignment
    );

    unsigned int bindBufferToMemory(VkBuffer& buffer, VkDeviceSize const offset);
    void bindImageToMemory(VkImage& image, VkDeviceSize const offset);
    void clear();
    unsigned int getID() const;
    VkDeviceMemory* getMemory();
    uint32_t getOffset() const;
    VkDeviceSize getSize() const;
    VkDeviceSize getSpaceLeft() const;
    VkMemoryPropertyFlags getType() const;
    bool hasEnoughSpaceLeft(VkDeviceSize size);
    bool isFull() const;
    void lock();
    void unLock();
    VkBuffer& getBuffer(unsigned int index);
    unsigned int getOffset(unsigned int index) const;

  private:
    void allocateToMemory();

    unsigned int _index{0};

    bool _is_allocated{false};
    bool _is_full{false};

    VkDeviceSize _alignment;
    VkDeviceSize _max_size;
    VkMemoryPropertyFlags _memory_type;
    //@todo check with deviceProps.limits.bufferImageGranularity;
    VkDeviceSize _offset{0};
    unsigned int _buffer_count{ 0 };
    std::vector<VkDeviceSize> _buffer_offsets;
    std::vector<VkBuffer> _buffer;
    VkDevice _device;
    std::unique_ptr<VkDeviceMemory> _memory;
    std::mutex _mutex_memory;
  };
}
