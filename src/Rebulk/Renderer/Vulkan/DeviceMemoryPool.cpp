#include "rebulkpch.h"
#include "DeviceMemoryPool.h"

namespace Rbk
{
    DeviceMemoryPool::DeviceMemoryPool(VkPhysicalDeviceProperties2 deviceProperties, VkPhysicalDeviceMaintenance3Properties maintenceProperties)
        : m_DeviceProperties(deviceProperties), m_MaintenceProperties(maintenceProperties)
    {

    }
    std::shared_ptr<DeviceMemory> DeviceMemoryPool::Get(VkDevice& device, VkDeviceSize size, uint32_t memoryType, VkBufferUsageFlags usage)
    {
        if (m_MemoryAllocationCount > m_DeviceProperties.properties.limits.maxMemoryAllocationCount) {
            throw std::runtime_error("Max number of active allocation reached");
        }

        VkDeviceSize maxSize;

        maxSize = m_MaintenceProperties.maxMemoryAllocationSize / 20.f;

        auto poolType = m_Pool.find(memoryType);

        if (m_Pool.end() != poolType) {
            auto poolUsage = poolType->second.find(usage);
            if (poolType->second.end() != poolUsage) {
                for (int i = 0; i < poolUsage->second.size(); ++i) {
                    auto dm = poolUsage->second.at(i);
                    if (!dm->IsFull() && dm->HasEnoughSpaceLeft(size)) {
                        //Rbk::Log::GetLogger()->debug("memory allocation recycle: size {} type {} usage {} full {} space left {}", size, memoryType, usage, dm->IsFull(), dm->HasEnoughSpaceLeft(size));
                        return dm;
                    }
                }
            }

            if (m_MemoryAllocationSize + maxSize > m_MaintenceProperties.maxMemoryAllocationSize) {
                std::cout << maxSize << " " << m_MemoryAllocationSize << " " << m_MaintenceProperties.maxMemoryAllocationSize << std::endl;
                throw std::runtime_error("Max size of memory allocation reached");
            }

            auto dm = std::make_shared<DeviceMemory>(device, memoryType, usage, maxSize);
            m_Pool[memoryType][usage].emplace_back(dm);
            m_MemoryAllocationCount += 1;
            m_MemoryAllocationSize += maxSize;
            //Rbk::Log::GetLogger()->debug("memory allocation creation: count {}, size {} type {} usage {} ", m_MemoryAllocationCount, m_MemoryAllocationSize, memoryType, usage);
            return dm;
        } else {

            if (m_MemoryAllocationSize + maxSize > m_MaintenceProperties.maxMemoryAllocationSize) {
                //throw std::runtime_error("Max size of memory allocation reached");
            }

            auto dm = std::make_shared<DeviceMemory>(device, memoryType, usage, maxSize);
            m_Pool[memoryType][usage].emplace_back(dm);
            m_MemoryAllocationCount += 1;
            m_MemoryAllocationSize += maxSize;
            //Rbk::Log::GetLogger()->debug("memory allocation creation: count {}, size {} type {} usage {} ", m_MemoryAllocationCount, m_MemoryAllocationSize, memoryType, usage);
            return dm;
        }
    }
}