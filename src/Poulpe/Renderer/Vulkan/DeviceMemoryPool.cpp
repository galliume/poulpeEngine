#include "DeviceMemoryPool.hpp"

namespace Poulpe
{
    DeviceMemoryPool::DeviceMemoryPool(
      VkPhysicalDeviceProperties2 deviceProperties,
      VkPhysicalDeviceMaintenance3Properties maintenanceProperties,
      VkPhysicalDeviceMemoryProperties memProperties)
      : _DeviceProperties(deviceProperties),
        _MaintenanceProperties(maintenanceProperties),
        _MemProperties(memProperties)
    {
        _MemoryAllocationSize.resize(_MemProperties.memoryTypeCount);
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
        if (_MemoryAllocationCount > _DeviceProperties.properties.limits.maxMemoryAllocationCount) {
          throw std::runtime_error("Max number of active allocations reached");
        }

       VkDeviceSize bufferSize;
       VkDeviceSize maxBufferSize = _MemProperties.memoryHeaps[_MemProperties.memoryTypes[memoryType].heapIndex].size;
       std::string bufferTypeDebug;

       switch (bufferType) {
           case DeviceBufferType::STORAGE:
             bufferSize = maxBufferSize / _MaxStorage;
             bufferTypeDebug = "STORAGE";
           break;
           case DeviceBufferType::UNIFORM:
             bufferSize = _DeviceProperties.properties.limits.maxUniformBufferRange * _MaxUniform;

             if (size > bufferSize) {
               VkDeviceSize const countBuffers{ (size / _DeviceProperties.properties.limits.maxUniformBufferRange) + 1 };
               bufferSize = _DeviceProperties.properties.limits.maxUniformBufferRange * countBuffers;
             }
             bufferTypeDebug = "UNIFORM";
           break;
           case DeviceBufferType::STAGING:
             bufferSize = _MaintenanceProperties.maxMemoryAllocationSize / _MaxStaging;
              if (size > bufferSize) {
               VkDeviceSize const countBuffers{ (size / _MaintenanceProperties.maxMemoryAllocationSize) + 1 };
               bufferSize = _MaintenanceProperties.maxMemoryAllocationSize * countBuffers;
             }

             bufferTypeDebug = "STAGING";
           break;
      }

      auto poolType = _Pool.find(memoryType);

      if (_Pool.end() != poolType && !forceNew) {
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

        if (_MemoryAllocationSize.at(memoryType) + bufferSize > maxBufferSize) {
            throw std::runtime_error("Max size of memory allocation reached");
        }

        _Pool[memoryType][usage].emplace_back(std::make_unique<DeviceMemory>(
          device, memoryType, bufferSize, _DeviceMemoryCount, alignment));
        _MemoryAllocationCount += 1;
        _MemoryAllocationSize.at(memoryType) += bufferSize;
        /*PLP_DEBUG("DM CREATION: id {}, {}, type {} usage {} size {}", _DeviceMemoryCount, bufferTypeDebug, memoryType, usage, bufferSize);*/
        _DeviceMemoryCount += 1;
        _MemorySizeAllocated += size;
        return _Pool[memoryType][usage].back().get();
      } else {

        if (_MemoryAllocationSize.at(memoryType) + bufferSize > maxBufferSize) {
          throw std::runtime_error("Max size of memory allocation reached");
        }

        _Pool[memoryType][usage].emplace_back(std::make_unique<DeviceMemory>(
          device, memoryType, bufferSize, _DeviceMemoryCount, alignment));
        _MemoryAllocationCount += 1;
        _MemoryAllocationSize.at(memoryType) += bufferSize;
        /*PLP_DEBUG("DM CREATION: id {}, {}, type {} usage {} size {}", _DeviceMemoryCount, bufferTypeDebug, memoryType, usage, bufferSize);*/
        _DeviceMemoryCount += 1;
        _MemorySizeAllocated += size;
        return _Pool[memoryType][usage].back().get();
      }
    }

    void DeviceMemoryPool::clear()
    {
      PLP_WARN("MEMORY POOL CLEANING");

      for (auto& memoryType : _Pool) {
        for (auto& usage : memoryType.second) {
          for (auto& mem : usage.second) {
              clear(mem.get());
          }
        }
      }

      _Pool.clear();

      _MemoryAllocationCount = 0; //should be zero anyway
      _MemoryAllocationSize.clear();
      _DeviceMemoryCount = 0; //should be zero anyway
    }

    void DeviceMemoryPool::clear(DeviceMemory * deviceMemory)
    {
        _MemoryAllocationSize.at(deviceMemory->getType()) -= deviceMemory->getSize();
        _MemoryAllocationCount -= 1;
        _DeviceMemoryCount -= 1;
        deviceMemory->clear();
        PLP_DEBUG("Clearing device memory id {}. {} left", deviceMemory->getID(), _DeviceMemoryCount);
    }
}
