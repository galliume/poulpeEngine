#pragma once
#include "DeviceMemory.hpp"

namespace Poulpe
{
    //@todo manage memory fragmentation
    class DeviceMemoryPool
    {
    public:

        enum class DeviceBufferType {
          UNIFORM, STORAGE, STAGING
        };

        DeviceMemoryPool(VkPhysicalDeviceProperties2 deviceProperties,
          VkPhysicalDeviceMaintenance3Properties maintenanceProperties,
          VkPhysicalDeviceMemoryProperties memProperties);

        DeviceMemory* get(
          VkDevice const & device,
          VkDeviceSize const size,
          uint32_t const memoryType,
          VkBufferUsageFlags const usage,
          VkDeviceSize const alignment,
          DeviceBufferType const bufferType,
          bool forceNew = false);

        std::unordered_map<uint32_t, std::unordered_map<VkBufferUsageFlags,
          std::vector<std::unique_ptr<DeviceMemory>>>>* getPool() { return & _Pool; }

        void clear(DeviceMemory * deviceMemory);
        void clear();

    private:
        std::unordered_map<uint32_t, std::unordered_map<VkBufferUsageFlags, std::vector<std::unique_ptr<DeviceMemory>>>> _Pool;
        VkPhysicalDeviceProperties2 _DeviceProperties;
         VkPhysicalDeviceMaintenance3Properties _MaintenanceProperties;
        VkPhysicalDeviceMemoryProperties _MemProperties;
        VkDeviceSize _MemoryAllocationCount{ 0 };
        VkDeviceSize _MemorySizeAllocated{ 0 };
        std::vector<VkDeviceSize> _MemoryAllocationSize{0};
        unsigned int _DeviceMemoryCount{ 0 };
        unsigned int const _MaxUniform{ 1000 };
        unsigned int const _MaxStorage{ 5 };
        unsigned int const _MaxStaging{ 5 };
    };
}
