module;

#include <volk.h>

#include <memory>
#include <mutex>
#include <stdexcept>

module Poulpe.Renderer.Vulkan.DeviceMemory;

namespace Poulpe
{
  DeviceMemory::DeviceMemory(
      VkDevice device,
      VkMemoryPropertyFlags memory_type,
      VkDeviceSize max_size,
      uint32_t index,
      VkDeviceSize alignment
  ) : _index(index),
      _alignment(alignment),
      _memory_type(memory_type),
      _device(device)
  {
    if (!_memory) {
        _memory = std::make_unique<VkDeviceMemory>();
        _max_size = max_size;
        allocateToMemory();
    }
    _buffer_offsets.emplace_back(0);
  }

  VkDeviceMemory* DeviceMemory::getMemory()
  {
    if (!_memory) {
      _memory = std::make_unique<VkDeviceMemory>();
      _offset = 0;
      _is_full = false;
      _is_allocated = false;
      allocateToMemory();
    }

    return _memory.get();
  }

  void DeviceMemory::allocateToMemory()
  {
    {
      std::lock_guard<std::mutex> guard(_mutex_memory);
      if (!_is_allocated) {
        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = _max_size;
        alloc_info.memoryTypeIndex = _memory_type;

        VkResult result = vkAllocateMemory(_device, &alloc_info, nullptr, _memory.get());

        if (VK_SUCCESS != result) {
          //@todo use fmt::format specifier
          //Logger::critical("error while allocating memory {}", static_cast<int>(result));
          throw std::runtime_error("failed to allocate buffer memory!");
        }

        _is_allocated = true;
      } else {
        //Logger::warn("trying to re allocate memory already allocated.");
      }
    }
  }

  uint32_t DeviceMemory::bindBufferToMemory(VkBuffer & buffer, VkDeviceSize const offset)
  {
    {
      std::lock_guard<std::mutex> guard(_mutex_memory);

      auto const remainder {_offset % offset};

      if (remainder != 0) {
        _offset += (offset - remainder);
      }

      VkResult result = vkBindBufferMemory(_device, buffer, *_memory, _offset);

      if (VK_SUCCESS != result) {
        //Logger::debug("BindBuffer memory failed in bindBufferToMemory");
      }

      _buffer_offsets.emplace_back(_offset);
      _offset += offset;

      if (_offset >= _max_size) {
        _is_full = true;
      }

      _buffer.emplace_back(buffer);
      _buffer_count += 1;
      return _buffer_count - 1;
    }
  }

  void DeviceMemory::bindImageToMemory(VkImage & image, VkDeviceSize const offset)
  {
    {
      std::lock_guard<std::mutex> guard(_mutex_memory);

      auto const remainder {_offset % offset};
      if (remainder != 0) {
        _offset += (offset - remainder);
      }

      VkResult result = vkBindImageMemory(_device, image, *_memory, _offset);

      if (VK_SUCCESS != result) {
        //Logger::debug("BindImageMemory failed in bindImageToMemory");
      }

      _offset += offset;

      if (_offset >= _max_size) {
          _is_full = true;
      }
    }
  }

  bool DeviceMemory::hasEnoughSpaceLeft(VkDeviceSize size)
  {
    {
      std::lock_guard<std::mutex> guard(_mutex_memory);
      auto offset { _offset };

      auto const remainder {offset % size};

      if (remainder != 0) {
        offset += (size - remainder);
      }

      bool has_enought_space_left { _max_size - offset > size };

      //Logger::debug("max size: {} offset: {} size {} :{}", _max_size, offset, size, (_max_size - offset));
      if (!has_enought_space_left) _is_full = true;

      return has_enought_space_left;
    }
  }

  void DeviceMemory::clear()
  {
    for (auto buffer : _buffer) {
      if (VK_NULL_HANDLE != buffer) {
        vkDestroyBuffer(_device, buffer, nullptr);
      }
    }

    vkFreeMemory(_device, *_memory.get(), nullptr);

    _max_size = 0;
    _offset = 0;
    _is_full = false;

    _memory.reset();
  }

  VkDeviceSize DeviceMemory::getID() const
  { 
    return _index;
  }

  VkDeviceSize DeviceMemory::getOffset() const
  { 
    return _offset;
  }

  VkDeviceSize DeviceMemory::getSize() const
  {
    return _max_size;
  }

  VkDeviceSize DeviceMemory::getSpaceLeft() const
  {
    return _max_size - _offset;
  }

  VkMemoryPropertyFlags DeviceMemory::getType() const 
  {
    return _memory_type;
  }

  bool DeviceMemory::isFull() const
  {
    return _is_full;
  }

  void DeviceMemory::lock()
  {
    _mutex_memory.lock();
  }
  
  void DeviceMemory::unLock()
  {
    _mutex_memory.unlock();
  }

  VkBuffer& DeviceMemory::getBuffer(size_t index)
  {
    return _buffer.at(index);
  }

  uint32_t DeviceMemory::getOffset(size_t index) const
  {
    return _buffer_offsets.at(index);
  }
}
