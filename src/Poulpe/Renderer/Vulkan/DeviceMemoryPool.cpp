#include "DeviceMemoryPool.hpp"

namespace Poulpe
{
    DeviceMemoryPool::DeviceMemoryPool(VkPhysicalDeviceProperties2 deviceProperties,
      VkPhysicalDeviceMaintenance3Properties maintenanceProperties,
      VkPhysicalDeviceMemoryProperties memProperties)
      : m_DeviceProperties(deviceProperties),
        m_MaintenanceProperties(maintenanceProperties),
        m_MemProperties(memProperties)
    {
        m_MemoryAllocationSize.resize(m_MemProperties.memoryTypeCount);
    }

    DeviceMemory* DeviceMemoryPool::get(VkDevice& device, VkDeviceSize size,
      uint32_t memoryType, VkBufferUsageFlags usage, DeviceBufferType bufferType,
      bool forceNew)
    {
        if (m_MemoryAllocationCount > m_DeviceProperties.properties.limits.maxMemoryAllocationCount) {
          throw std::runtime_error("Max number of active allocation reached");
        }

       unsigned int bufferSize;
       VkDeviceSize maxBufferSize = m_MemProperties.memoryHeaps[m_MemProperties.memoryTypes[memoryType].heapIndex].size;
       std::string bufferTypeDebug;

       switch (bufferType) {
           case DeviceBufferType::STORAGE:
             bufferSize = m_DeviceProperties.properties.limits.maxStorageBufferRange * m_MaxStorage;
             bufferTypeDebug = "STORAGE";
           break;
           case DeviceBufferType::UNIFORM:
             bufferSize = m_DeviceProperties.properties.limits.maxUniformBufferRange * m_MaxUBOS;
             bufferTypeDebug = "UNIFORM";
           break;
           case DeviceBufferType::STAGING:
             bufferSize = m_MaintenanceProperties.maxMemoryAllocationSize;
             bufferTypeDebug = "STAGING";
           break;
           default:
             throw std::runtime_error("Memory type unknown");
      }

      auto poolType = m_Pool.find(memoryType);

      if (m_Pool.end() != poolType && !forceNew) {
        auto poolUsage = poolType->second.find(usage);
        if (poolType->second.end() != poolUsage) {
          for (size_t i = 0; i < poolUsage->second.size(); ++i) {
            auto& dm = poolUsage->second.at(i);
            if (!dm->isFull() && dm->hasEnoughSpaceLeft(size)) {
              PLP_DEBUG("DM REUSE: id {}, type {}, size {} type {} usage {} full {} space left {}", 
                dm.get()->getID(), bufferTypeDebug, size, memoryType, usage, dm->isFull(), dm->getSpaceLeft());
              return dm.get();
            }
          }
        }

        if (m_MemoryAllocationSize.at(memoryType) + bufferSize > maxBufferSize) {
            throw std::runtime_error("Max size of memory allocation reached");
        }

        m_Pool[memoryType][usage].emplace_back(std::make_unique<DeviceMemory>(device, memoryType, usage, bufferSize, m_DeviceMemoryCount));
        m_MemoryAllocationCount += 1;
        m_MemoryAllocationSize.at(memoryType) += bufferSize;
        PLP_DEBUG("DM CREATION: id {}, type {}, id {}, size {} type {} usage {} ", 
          m_DeviceMemoryCount, bufferTypeDebug, m_MemoryAllocationCount, bufferSize, memoryType, usage);
        m_DeviceMemoryCount += 1;
        return m_Pool[memoryType][usage].back().get();
      } else {

        if (m_MemoryAllocationSize.at(memoryType) + bufferSize > maxBufferSize) {
          throw std::runtime_error("Max size of memory allocation reached");
        }

        m_Pool[memoryType][usage].emplace_back(std::make_unique<DeviceMemory>(device, memoryType, usage, bufferSize, m_DeviceMemoryCount));
        m_MemoryAllocationCount += 1;
        m_MemoryAllocationSize.at(memoryType) += bufferSize;
        PLP_DEBUG("DM CREATION: id {}, type {}, id {}, size {} type {} usage {} ",
        m_DeviceMemoryCount, bufferTypeDebug, m_MemoryAllocationCount, bufferSize, memoryType, usage);
        m_DeviceMemoryCount += 1;
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
