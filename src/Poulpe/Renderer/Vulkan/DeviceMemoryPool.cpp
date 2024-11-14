#include "DeviceMemoryPool.hpp"

namespace Poulpe
{
  DeviceMemoryPool::DeviceMemoryPool(
    VkPhysicalDeviceProperties2 device_props,
    VkPhysicalDeviceMaintenance3Properties maintenance_props,
    VkPhysicalDeviceMemoryProperties memory_props)
    : _device_props(device_props)
    , _maintenance_props(maintenance_props)
    , _memory_props(memory_props)
  {
    _memory_allocation_size.resize(_memory_props.memoryTypeCount);
  }

  DeviceMemory* DeviceMemoryPool::get(
    VkDevice const & device,
    VkDeviceSize const size,
    uint32_t const memory_type,
    VkBufferUsageFlags const usage,
    VkDeviceSize const alignment,
    DeviceBufferType const buffer_type,
    bool forceNew)
  {
    if (_memory_allocation_count > _device_props.properties.limits.maxMemoryAllocationCount) {
      throw std::runtime_error("Max number of active allocations reached");
    }

    VkDeviceSize buffer_size;
    VkDeviceSize max_buffer_size = _memory_props.memoryHeaps[_memory_props.memoryTypes[memory_type].heapIndex].size;
    std::string buffer_type_debug;

    switch (buffer_type) {
      case DeviceBufferType::STORAGE:
        buffer_size = max_buffer_size / _max_storage;
        buffer_type_debug = "STORAGE";
      break;
      case DeviceBufferType::UNIFORM:
        buffer_size = _device_props.properties.limits.maxUniformBufferRange * _max_uniform;

        if (size > buffer_size) {
          VkDeviceSize const count_buffer{ (size / _device_props.properties.limits.maxUniformBufferRange) + 1 };
          buffer_size = _device_props.properties.limits.maxUniformBufferRange * count_buffer;
        }
        buffer_type_debug = "UNIFORM";
      break;
      case DeviceBufferType::STAGING:
        buffer_size = _maintenance_props.maxMemoryAllocationSize / _max_staging;
        if (size > buffer_size) {
          VkDeviceSize const count_buffer{ (size / _maintenance_props.maxMemoryAllocationSize) + 1 };
          buffer_size = _maintenance_props.maxMemoryAllocationSize * count_buffer;
        }

        buffer_type_debug = "STAGING";
      break;
    }

    auto pool_type = _pool.find(memory_type);

    if (_pool.end() != pool_type && !forceNew) {
      auto poolUsage = pool_type->second.find(usage);
      if (pool_type->second.end() != poolUsage) {
        for (size_t i{ 0 }; i < poolUsage->second.size(); ++i) {
          auto& dm = poolUsage->second.at(i);
          if (!dm->isFull() && dm->hasEnoughSpaceLeft(size)) {
            //PLP_DEBUG("DM REUSE OK: id {}, {}, type {} usage {} size {}/{}",
            //  dm.get()->getID(), buffer_type_debug, memory_type, usage, size, dm->getSpaceLeft());
            return dm.get();
          } else {
            /*PLP_DEBUG("DM REUSE KO: id {}, {}, type {} usage {} size {}/{}",
              dm.get()->getID(), buffer_type_debug, memory_type, usage, size, dm->getSpaceLeft());*/
          }
        }
      }

      if (_memory_allocation_size.at(memory_type) + buffer_size > max_buffer_size) {
          throw std::runtime_error("Max size of memory allocation reached");
      }

      _pool[memory_type][usage].emplace_back(std::make_unique<DeviceMemory>(
        device, memory_type, buffer_size, _device_memory_count, alignment));
      _memory_allocation_count += 1;
      _memory_allocation_size.at(memory_type) += buffer_size;
      /*PLP_DEBUG("DM CREATION: id {}, {}, type {} usage {} size {}", _device_memory_count, buffer_type_debug, memory_type, usage, buffer_size);*/
      _device_memory_count += 1;
      _memory_size_allocated += size;
      return _pool[memory_type][usage].back().get();
    } else {

      if (_memory_allocation_size.at(memory_type) + buffer_size > max_buffer_size) {
        throw std::runtime_error("Max size of memory allocation reached");
      }

      _pool[memory_type][usage].emplace_back(std::make_unique<DeviceMemory>(
        device, memory_type, buffer_size, _device_memory_count, alignment));
      _memory_allocation_count += 1;
      _memory_allocation_size.at(memory_type) += buffer_size;
      /*PLP_DEBUG("DM CREATION: id {}, {}, type {} usage {} size {}", _device_memory_count, buffer_type_debug, memory_type, usage, buffer_size);*/
      _device_memory_count += 1;
      _memory_size_allocated += size;
      return _pool[memory_type][usage].back().get();
    }
  }

  void DeviceMemoryPool::clear()
  {
    PLP_WARN("MEMORY POOL CLEANING");

    for (auto& memory_type : _pool) {
      for (auto& usage : memory_type.second) {
        for (auto& mem : usage.second) {
          clear(mem.get());
        }
      }
    }

    _pool.clear();

    _memory_allocation_count = 0; //should be zero anyway
    _memory_allocation_size.clear();
    _device_memory_count = 0; //should be zero anyway
  }

  void DeviceMemoryPool::clear(DeviceMemory * deviceMemory)
  {
    _memory_allocation_size.at(deviceMemory->getType()) -= deviceMemory->getSize();
    _memory_allocation_count -= 1;
    _device_memory_count -= 1;
    deviceMemory->clear();
    PLP_DEBUG("Clearing device memory id {}. {} left", deviceMemory->getID(), _device_memory_count);
  }
}
