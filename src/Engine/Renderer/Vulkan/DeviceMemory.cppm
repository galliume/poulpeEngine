export module Engine.Renderer.VulkanDeviceMemory;

import std;

import Engine.Core.Volk;

namespace Poulpe
{
  export class DeviceMemory
  {
  public:
    DeviceMemory();
    DeviceMemory(
      VkDevice device,
      VkMemoryPropertyFlags memory_type,
      VkDeviceSize max_size,
      std::uint32_t index,
      VkDeviceSize alignment
    );
    DeviceMemory(DeviceMemory&& dm) noexcept
      : _index(std::move(dm._index))
      , _alignment(std::move(dm._alignment))
      , _memory_type(std::move(dm._memory_type))
      , _device(std::move(dm._device))
    {

    }
    DeviceMemory& operator=(DeviceMemory&& other) noexcept
    {
      if (this != &other) {
        _index = std::move(other._index);
        _alignment = std::move(other._alignment);
        _memory_type = std::move(other._memory_type);
        _device = std::move(other._device);
      }
      return *this;
    }
    ~DeviceMemory() = default;

    std::uint32_t bindBufferToMemory(VkBuffer buffer, VkDeviceSize const size, VkDeviceSize const alignment) __attribute__((no_thread_safety_analysis));
    void bindImageToMemory(VkImage image, VkDeviceSize const size, VkDeviceSize const alignment) __attribute__((no_thread_safety_analysis));
    void clear();
    VkDeviceSize getID() const;
    VkDeviceMemory* getMemory();
    VkDeviceSize getOffset() const;
    VkDeviceSize getSize() const;
    VkDeviceSize getSpaceLeft() const;
    VkMemoryPropertyFlags getType() const;
    bool hasEnoughSpaceLeft(VkDeviceSize size, VkDeviceSize const alignment) __attribute__((no_thread_safety_analysis));
    bool isFull() const;
    void lock();
    void unLock();
    VkBuffer getBuffer(std::size_t index);
    std::size_t getOffset(std::size_t index) const;

  private:
    void allocateToMemory() __attribute__((no_thread_safety_analysis));

    std::uint32_t _index{0};

    std::atomic<bool> _is_allocated{false};
    std::atomic<bool> _is_full{false};

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
