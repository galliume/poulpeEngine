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

        DeviceMemory* get(VkDevice& device, VkDeviceSize size, uint32_t memoryType,
          VkBufferUsageFlags usage, DeviceBufferType memType = DeviceBufferType::UNIFORM, bool forceNew = false);

        std::unordered_map<uint32_t, std::unordered_map<VkBufferUsageFlags,
          std::vector<std::unique_ptr<DeviceMemory>>>>* getPool() { return & m_Pool; }

        void clear(DeviceMemory * deviceMemory);
        void clear();

    private:
        std::unordered_map<uint32_t, std::unordered_map<VkBufferUsageFlags, std::vector<std::unique_ptr<DeviceMemory>>>> m_Pool;
        VkPhysicalDeviceProperties2 m_DeviceProperties;
        [[maybe_unused]] VkPhysicalDeviceMaintenance3Properties m_MaintenanceProperties;
        VkPhysicalDeviceMemoryProperties m_MemProperties;
        VkDeviceSize m_MemoryAllocationCount{ 0 };
        VkDeviceSize m_MemorySizeAllocated{ 0 };
        std::vector<VkDeviceSize> m_MemoryAllocationSize{0};
        unsigned int m_DeviceMemoryCount{ 0 };
        unsigned int const m_MaxUniform{ 1000 };
        unsigned int const m_MaxStorage{ 2 };
        unsigned int const m_MaxStaging{ 5 };
    };
}
