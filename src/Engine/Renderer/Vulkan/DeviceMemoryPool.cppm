export module Engine.Renderer.VulkanDeviceMemoryPool;

import std;

import Engine.Core.Volk;

//import Engine.Core.Logger;
import Engine.Renderer.VulkanDeviceMemory;

namespace Poulpe
{
  //@todo manage memory fragmentation
  export class DeviceMemoryPool
  {
  public:
    enum class DeviceBufferType : std::uint8_t
    {
      UNIFORM, STAGING, VERTEX
    };

    DeviceMemoryPool(
      VkPhysicalDeviceProperties2 device_props,
      VkPhysicalDeviceMaintenance3Properties maintenance_props,
      VkPhysicalDeviceMemoryProperties memory_props);

    DeviceMemory* get(
      VkDevice const & device,
      VkDeviceSize const size,
      std::uint32_t  const memory_type,
      VkBufferUsageFlags const usage,
      VkDeviceSize const alignment,
      DeviceBufferType const buffer_type,
      bool force_new = false) __attribute__((no_thread_safety_analysis));

    std::unordered_map<VkMemoryPropertyFlags, std::unordered_map<VkBufferUsageFlags,
      std::vector<std::unique_ptr<DeviceMemory>>>>* getPool() { return & _pool; }

    void clear(DeviceMemory * device_memory);
    void clear();

  private:
    std::unordered_map<VkMemoryPropertyFlags, std::unordered_map<VkBufferUsageFlags, std::vector<std::unique_ptr<DeviceMemory>>>> _pool;
    VkPhysicalDeviceProperties2 _device_props;
    VkPhysicalDeviceMaintenance3Properties _maintenance_props;
    VkPhysicalDeviceMemoryProperties _memory_props;
    std::atomic<VkDeviceSize> _memory_allocation_count{ 0 };
    std::atomic<VkDeviceSize> _memory_size_allocated{ 0 };
    std::vector<VkDeviceSize> _memory_allocation_size;

    std::atomic<std::uint32_t> _device_memory_count{ 0 };
    std::uint32_t const _max_staging{ 8 };
    std::mutex _mutex;
  };
}
