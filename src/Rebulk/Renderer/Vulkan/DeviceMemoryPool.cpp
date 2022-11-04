#include "rebulkpch.h"
#include "DeviceMemoryPool.h"

namespace Rbk
{
    DeviceMemoryPool::DeviceMemoryPool(VkDeviceSize maxMemoryAllocationCount, VkDeviceSize maxMemoryAllocationSize)
        : m_MaxMemoryAllocationCount(maxMemoryAllocationCount), m_MaxMemoryAllocationSize(maxMemoryAllocationSize)
    {

    }
    std::shared_ptr<DeviceMemory> DeviceMemoryPool::Get(VkDevice device, VkDeviceSize size, uint32_t memoryType, VkBufferUsageFlags usage)
    {
        if (m_MemoryAllocationCount > m_MaxMemoryAllocationCount) {
            throw std::runtime_error("Max allocation");
        }
        auto poolType = m_Pool.find(memoryType);

        if (m_Pool.end() != poolType) {
            auto poolUsage = poolType->second.find(usage);
            if (poolType->second.end() != poolUsage) {
                for (int i = 0; i < poolUsage->second.size(); ++i) {
                    auto dm = poolUsage->second.at(i);
                    if (!dm->IsFull() && dm->HasEnoughSpaceLeft(size)) {
                        return dm;
                    }
                }
            }

            auto dm = std::make_shared<DeviceMemory>(device, memoryType, usage, m_MaxMemoryAllocationSize);
            m_Pool[memoryType][usage].emplace_back(dm);
            m_MemoryAllocationCount += 1;
            return dm;
        } else {
            auto dm = std::make_shared<DeviceMemory>(device, memoryType, usage, m_MaxMemoryAllocationSize);
            m_Pool[memoryType][usage].emplace_back(dm);
            m_MemoryAllocationCount += 1;
            return dm;
        }
    }
}