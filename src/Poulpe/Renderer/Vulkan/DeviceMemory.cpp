#include "DeviceMemory.hpp"

namespace Poulpe
{
    DeviceMemory::DeviceMemory(
        VkDevice device,
        uint32_t memoryType,
        VkDeviceSize maxSize,
        unsigned int index,
        VkDeviceSize alignment
    ) : _Index(index),
        _Alignment(alignment),
        _MemoryType(memoryType),
        _device(device)
    {
        if (!_Memory) {
            _Memory = std::make_unique<VkDeviceMemory>();
            _MaxSize = maxSize;
            allocateToMemory();
        }
    }

    VkDeviceMemory* DeviceMemory::getMemory()
    {
        if (!_Memory) {
            _Memory = std::make_unique<VkDeviceMemory>();
            _Offset = 0;
            _IsFull = false;
            _IsAllocated = false;
            allocateToMemory();
        }

        return _Memory.get();
    }

    void DeviceMemory::allocateToMemory()
    {
      {
        std::lock_guard guard(_MutexMemory);
        if (!_IsAllocated) {
          VkMemoryAllocateInfo allocInfo{};
          allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
          allocInfo.allocationSize = _MaxSize;
          allocInfo.memoryTypeIndex = _MemoryType;

          VkResult result = vkAllocateMemory(_device, &allocInfo, nullptr, _Memory.get());

          if (VK_SUCCESS != result) {
            PLP_FATAL("error while allocating memory {}", result);
            throw std::runtime_error("failed to allocate buffer memory!");
          }

          _IsAllocated = true;
        }
        else {
          PLP_WARN("trying to re allocate memory already allocated.");
        }
      }
    }

    void DeviceMemory::bindBufferToMemory(VkBuffer & buffer, VkDeviceSize size)
    {
      {
        std::lock_guard guard(_MutexMemory);
        VkResult result = vkBindBufferMemory(_device, buffer, *_Memory, _Offset);

        if (VK_SUCCESS != result) {
          PLP_ERROR("BindBuffer memory failed in bindBufferToMemory");
        }

        _Offset += size;

        if (_Offset >= _MaxSize) {
          _IsFull = true;
        }

        _Buffer.emplace_back(buffer);
      }
    }

    void DeviceMemory::bindImageToMemory(VkImage & image, VkDeviceSize size)
    {
      {
        std::lock_guard guard(_MutexMemory);
        vkBindImageMemory(_device, image, *_Memory, _Offset);
        
        _Offset += size;// ((size / _Alignment) + 1)* _Alignment;

        if (_Offset >= _MaxSize) {
            _IsFull = true;
        }
      }
    }

    bool DeviceMemory::hasEnoughSpaceLeft(VkDeviceSize size)
    { 
        bool hasEnoughSpaceLeft = _MaxSize - _Offset >= size;
        if (!hasEnoughSpaceLeft) _IsFull = true;

        return hasEnoughSpaceLeft;
    }

    void DeviceMemory::clear()
    {
        for (auto buffer : _Buffer) {
            if (VK_NULL_HANDLE != buffer)
            {
                vkDestroyBuffer(_device, buffer, nullptr);
            }
        }

        vkFreeMemory(_device, *_Memory.get(), nullptr);

        _MaxSize = 0;
        _Offset = 0;
        _IsFull = false;

        _Memory.reset();
    }
}
