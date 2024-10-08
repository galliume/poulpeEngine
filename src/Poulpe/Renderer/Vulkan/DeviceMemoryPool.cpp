#include "DeviceMemoryPool.hpp"

namespace Poulpe
{
    DeviceMemoryPool::DeviceMemoryPool(
      VkPhysicalDeviceProperties2 deviceProperties,
      VkPhysicalDeviceMaintenance3Properties maintenanceProperties,
      VkPhysicalDeviceMemoryProperties memProperties)
      : m_DeviceProperties(deviceProperties),
        m_MaintenanceProperties(maintenanceProperties),
        m_MemProperties(memProperties)
    {
        m_MemoryAllocationSize.resize(m_MemProperties.memoryTypeCount);
    }

    DeviceMemory* DeviceMemoryPool::get(
      VkDevice const & device,
      VkDeviceSize const size,
      uint32_t const memoryType,
      VkBufferUsageFlags const usage,
      VkDeviceSize const alignment,
      DeviceBufferType const bufferType,
      bool forceNew)
    {
        if (m_MemoryAllocationCount > m_DeviceProperties.properties.limits.maxMemoryAllocationCount) {
          throw std::runtime_error("Max number of active allocations reached");
        }

       VkDeviceSize bufferSize;
       VkDeviceSize maxBufferSize = m_MemProperties.memoryHeaps[m_MemProperties.memoryTypes[memoryType].heapIndex].size;
       std::string bufferTypeDebug;

       switch (bufferType) {
           case DeviceBufferType::STORAGE:
             bufferSize = maxBufferSize / m_MaxStorage;
             bufferTypeDebug = "STORAGE";
           break;
           case DeviceBufferType::UNIFORM:
             bufferSize = m_DeviceProperties.properties.limits.maxUniformBufferRange * m_MaxUniform;

             if (size > bufferSize) {
               VkDeviceSize const countBuffers{ (size / m_DeviceProperties.properties.limits.maxUniformBufferRange) + 1 };
               bufferSize = m_DeviceProperties.properties.limits.maxUniformBufferRange * countBuffers;
             }
             bufferTypeDebug = "UNIFORM";
           break;
           case DeviceBufferType::STAGING:
             bufferSize = m_MaintenanceProperties.maxMemoryAllocationSize / m_MaxStaging;
              if (size > bufferSize) {
               VkDeviceSize const countBuffers{ (size / m_MaintenanceProperties.maxMemoryAllocationSize) + 1 };
               bufferSize = m_MaintenanceProperties.maxMemoryAllocationSize * countBuffers;
             }

             bufferTypeDebug = "STAGING";
           break;
      }

      auto poolType = m_Pool.find(memoryType);

      if (m_Pool.end() != poolType && !forceNew) {
        auto poolUsage = poolType->second.find(usage);
        if (poolType->second.end() != poolUsage) {
          for (size_t i{ 0 }; i < poolUsage->second.size(); ++i) {
            auto& dm = poolUsage->second.at(i);
            if (!dm->isFull() && dm->hasEnoughSpaceLeft(size)) {
              //PLP_DEBUG("DM REUSE OK: id {}, {}, type {} usage {} size {}/{}",
              //  dm.get()->getID(), bufferTypeDebug, memoryType, usage, size, dm->getSpaceLeft());
              return dm.get();
            } else {
              /*PLP_DEBUG("DM REUSE KO: id {}, {}, type {} usage {} size {}/{}",
                dm.get()->getID(), bufferTypeDebug, memoryType, usage, size, dm->getSpaceLeft());*/
            }
          }
        }

        if (m_MemoryAllocationSize.at(memoryType) + bufferSize > maxBufferSize) {
            throw std::runtime_error("Max size of memory allocation reached");
        }

        m_Pool[memoryType][usage].emplace_back(std::make_unique<DeviceMemory>(
          device, memoryType, bufferSize, m_DeviceMemoryCount, alignment));
        m_MemoryAllocationCount += 1;
        m_MemoryAllocationSize.at(memoryType) += bufferSize;
        /*PLP_DEBUG("DM CREATION: id {}, {}, type {} usage {} size {}", m_DeviceMemoryCount, bufferTypeDebug, memoryType, usage, bufferSize);*/
        m_DeviceMemoryCount += 1;
        m_MemorySizeAllocated += size;
        return m_Pool[memoryType][usage].back().get();
      } else {

        if (m_MemoryAllocationSize.at(memoryType) + bufferSize > maxBufferSize) {
          throw std::runtime_error("Max size of memory allocation reached");
        }

        m_Pool[memoryType][usage].emplace_back(std::make_unique<DeviceMemory>(
          device, memoryType, bufferSize, m_DeviceMemoryCount, alignment));
        m_MemoryAllocationCount += 1;
        m_MemoryAllocationSize.at(memoryType) += bufferSize;
        /*PLP_DEBUG("DM CREATION: id {}, {}, type {} usage {} size {}", m_DeviceMemoryCount, bufferTypeDebug, memoryType, usage, bufferSize);*/
        m_DeviceMemoryCount += 1;
        m_MemorySizeAllocated += size;
        return m_Pool[memoryType][usage].back().get();
      }
    }

    void DeviceMemoryPool::clear()
    {
      PLP_WARN("MEMORY POOL CLEANING");

      for (auto& memoryType : m_Pool) {
        for (auto& usage : memoryType.second) {
          for (auto& mem : usage.second) {
              clear(mem.get());
          }
        }
      }

      m_Pool.clear();

      m_MemoryAllocationCount = 0; //should be zero anyway
      m_MemoryAllocationSize.clear();
      m_DeviceMemoryCount = 0; //should be zero anyway
    }

    void DeviceMemoryPool::clear(DeviceMemory * deviceMemory)
    {
        m_MemoryAllocationSize.at(deviceMemory->getType()) -= deviceMemory->getSize();
        m_MemoryAllocationCount -= 1;
        m_DeviceMemoryCount -= 1;
        deviceMemory->clear();
        PLP_DEBUG("Clearing device memory id {}. {} left", deviceMemory->getID(), m_DeviceMemoryCount);
    }
}
