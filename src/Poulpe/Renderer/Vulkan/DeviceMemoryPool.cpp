#include "DeviceMemoryPool.hpp"

namespace Poulpe
{
    DeviceMemoryPool::DeviceMemoryPool(VkPhysicalDeviceProperties2 deviceProperties,
      VkPhysicalDeviceMaintenance3Properties maintenceProperties)
        : m_DeviceProperties(deviceProperties), m_MaintenceProperties(maintenceProperties)
    {

    }
    DeviceMemory* DeviceMemoryPool::get(VkDevice & device, VkDeviceSize size, uint32_t memoryType,
      VkBufferUsageFlags usage, bool forceNew)
    {
        if (m_MemoryAllocationCount > m_DeviceProperties.properties.limits.maxMemoryAllocationCount) {
            throw std::runtime_error("Max number of active allocation reached");
        }

        VkDeviceSize maxSize;

        maxSize = m_MaintenceProperties.maxMemoryAllocationSize / 20.f;

        auto poolType = m_Pool.find(memoryType);

        if (m_Pool.end() != poolType && !forceNew) {
            auto poolUsage = poolType->second.find(usage);
            if (poolType->second.end() != poolUsage) {
                for (size_t i = 0; i < poolUsage->second.size(); ++i) {
                    auto & dm = poolUsage->second.at(i);
                    if (!dm->isFull() && dm->hasEnoughSpaceLeft(size)) {
                        //PLP_DEBUG("memory allocation recycle: size {} type {} usage {} full {} space left {}", size, memoryType, usage, dm->IsFull(), dm->HasEnoughSpaceLeft(size));
                        return dm.get();
                    }
                }
            }

            if (m_MemoryAllocationSize + maxSize > m_MaintenceProperties.maxMemoryAllocationSize) {
                throw std::runtime_error("Max size of memory allocation reached");
            }

            m_Pool[memoryType][usage].emplace_back(std::make_unique<DeviceMemory>(device, memoryType, usage, maxSize));
            m_MemoryAllocationCount += 1;
            m_MemoryAllocationSize += maxSize;
            //PLP_DEBUG("memory allocation creation: count {}, size {} type {} usage {} ", m_MemoryAllocationCount, m_MemoryAllocationSize, memoryType, usage);
            return m_Pool[memoryType][usage].back().get();
        } else {

            if (m_MemoryAllocationSize + maxSize > m_MaintenceProperties.maxMemoryAllocationSize) {
                //throw std::runtime_error("Max size of memory allocation reached");
                PLP_ERROR("Max size of memory allocation reached");
            }

            m_Pool[memoryType][usage].emplace_back(std::make_unique<DeviceMemory>(device, memoryType, usage, maxSize));
            m_MemoryAllocationCount += 1;
            m_MemoryAllocationSize += maxSize;
            //PLP_DEBUG("memory allocation creation: count {}, size {} type {} usage {} ", m_MemoryAllocationCount, m_MemoryAllocationSize, memoryType, usage);
            return m_Pool[memoryType][usage].back().get();
        }
    }

    void DeviceMemoryPool::clear()
    {
        PLP_WARN("MEMORY POOL CLEANING");
        for (auto& memoryType : m_Pool) {
            for (auto& usage : memoryType.second) {
                for (auto& mem : usage.second) {
                    mem->clear();
                }
            }
        }
        
        m_Pool.clear();

        m_MemoryAllocationCount = 0;
        m_MemoryAllocationSize = 0;
    }
}
