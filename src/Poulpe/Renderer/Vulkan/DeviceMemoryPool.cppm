
module;
#include <volk.h>

#include <memory>
#include <unordered_map>
#include <vector>

export module Poulpe.Renderer:DeviceMemoryPool;

//import Poulpe.Core.Logger;
import Poulpe.Renderer.Vulkan.DeviceMemory;

namespace Poulpe
{
  //@todo manage memory fragmentation
  export class DeviceMemoryPool
  {
  public:
    enum class DeviceBufferType : uint8_t
    {
      UNIFORM, STAGING
    };

    DeviceMemoryPool(
      VkPhysicalDeviceProperties2 device_props,
      VkPhysicalDeviceMaintenance3Properties maintenance_props,
      VkPhysicalDeviceMemoryProperties memory_props);

    DeviceMemory* get(
      VkDevice const & device,
      VkDeviceSize const size,
      VkMemoryPropertyFlags  const memory_type,
      VkBufferUsageFlags const usage,
      VkDeviceSize const alignment,
      DeviceBufferType const buffer_type,
      bool force_new = false);

    std::unordered_map<VkMemoryPropertyFlags, std::unordered_map<VkBufferUsageFlags,
      std::vector<std::unique_ptr<DeviceMemory>>>>* getPool() { return & _pool; }

    void clear(DeviceMemory * device_memory);
    void clear();

  private:
    std::unordered_map<VkMemoryPropertyFlags, std::unordered_map<VkBufferUsageFlags, std::vector<std::unique_ptr<DeviceMemory>>>> _pool;
    VkPhysicalDeviceProperties2 _device_props;
    VkPhysicalDeviceMaintenance3Properties _maintenance_props;
    VkPhysicalDeviceMemoryProperties _memory_props;
    VkDeviceSize _memory_allocation_count{ 0 };
    VkDeviceSize _memory_size_allocated{ 0 };
    std::vector<VkDeviceSize> _memory_allocation_size{0};

    unsigned int _device_memory_count{ 0 };
    unsigned int const _max_staging{ 16 };
  };
}
