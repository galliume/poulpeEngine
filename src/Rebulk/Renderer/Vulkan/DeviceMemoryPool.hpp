#pragma once
#include "DeviceMemory.hpp"

namespace Rbk
{
    class DeviceMemoryPool
    {
    public:
        DeviceMemoryPool(VkPhysicalDeviceProperties2 deviceProperties, VkPhysicalDeviceMaintenance3Properties maintenceProperties);
        std::shared_ptr<DeviceMemory> Get(VkDevice& device, VkDeviceSize size, uint32_t memoryType, VkBufferUsageFlags usage, bool forceNew = false);
        std::unordered_map<uint32_t, std::unordered_map<VkBufferUsageFlags, std::vector<std::shared_ptr<DeviceMemory>>>> GetPool() { return m_Pool; }
        void Clear();

    private:
        std::unordered_map<uint32_t, std::unordered_map<VkBufferUsageFlags, std::vector<std::shared_ptr<DeviceMemory>>>> m_Pool;
        VkPhysicalDeviceProperties2 m_DeviceProperties;
        VkPhysicalDeviceMaintenance3Properties m_MaintenceProperties;
        VkDeviceSize m_MemoryAllocationCount = 0;
        VkDeviceSize m_MemoryAllocationSize = 0;
    };
}
