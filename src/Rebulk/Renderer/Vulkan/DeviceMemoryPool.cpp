#include "rebulkpch.h"
#include "DeviceMemoryPool.h"

namespace Rbk
{
    std::shared_ptr<DeviceMemory> DeviceMemoryPool::Get(VkDevice device, VkDeviceSize size, uint32_t memoryType, VkBufferUsageFlags usage)
    {
        if (0 != m_Pool[memoryType].size()) {
            for (int i = 0; i < m_Pool[memoryType][usage].size(); ++i) {
                auto dm = m_Pool[memoryType][usage].at(i);
                if (!dm->IsFull() && dm->HasEnoughSpaceLeft(size)) {
                    return dm;
                }
            }

            //all are full, add a new one
            auto dm = std::make_shared<DeviceMemory>(device, memoryType, usage);
            m_Pool[memoryType][usage].emplace_back(dm);
            return dm;
        } else {
            auto dm = std::make_shared<DeviceMemory>(device, memoryType, usage);
            m_Pool[memoryType][usage].emplace_back(dm);
            return dm;
        }
    }
}