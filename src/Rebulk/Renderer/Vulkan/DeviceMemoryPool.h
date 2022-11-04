#pragma once
#include "DeviceMemory.h"

namespace Rbk
{
    class DeviceMemoryPool
    {
    public:
        std::shared_ptr<DeviceMemory> Get(VkDevice device, VkDeviceSize size, uint32_t memoryType, VkBufferUsageFlags usage);

    private:
        std::map<uint32_t, std::map<VkBufferUsageFlags, std::vector<std::shared_ptr<DeviceMemory>>>> m_Pool;
    };
}