#include "DeviceMemory.hpp"

namespace Poulpe
{
    DeviceMemory::DeviceMemory(
        VkDevice device,
        uint32_t memoryType,
        VkDeviceSize maxSize,
        unsigned int index,
        VkDeviceSize alignment
    ) : m_Index(index),
        m_Alignment(alignment),
        m_MemoryType(memoryType),
        m_Device(device)
    {
        if (!m_Memory) {
            m_Memory = std::make_unique<VkDeviceMemory>();
            m_MaxSize = maxSize;
            allocateToMemory();
        }
    }

    VkDeviceMemory* DeviceMemory::getMemory()
    {
        if (!m_Memory) {
            m_Memory = std::make_unique<VkDeviceMemory>();
            m_Offset = 0;
            m_IsFull = false;
            m_IsAllocated = false;
            allocateToMemory();
        }

        return m_Memory.get();
    }

    void DeviceMemory::allocateToMemory()
    {
      {
        std::lock_guard guard(m_MutexMemory);
        if (!m_IsAllocated) {
          VkMemoryAllocateInfo allocInfo{};
          allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
          allocInfo.allocationSize = m_MaxSize;
          allocInfo.memoryTypeIndex = m_MemoryType;
          PLP_DEBUG("m_MaxSize: {}", m_MaxSize);
          VkResult result = vkAllocateMemory(m_Device, &allocInfo, nullptr, m_Memory.get());

          if (VK_SUCCESS != result) {
            PLP_FATAL("error while allocating memory {}", result);
            throw std::runtime_error("failed to allocate buffer memory!");
          }

          m_IsAllocated = true;
        }
        else {
          PLP_WARN("trying to re allocate memory already allocated.");
        }
      }
    }

    void DeviceMemory::bindBufferToMemory(VkBuffer & buffer, VkDeviceSize size)
    {
      {
        std::lock_guard guard(m_MutexMemory);
        VkResult result = vkBindBufferMemory(m_Device, buffer, *m_Memory, m_Offset);

        if (VK_SUCCESS != result) {
          PLP_ERROR("BindBuffer memory failed in bindBufferToMemory");
        }

        m_Offset += size;

        if (m_Offset >= m_MaxSize) {
          m_IsFull = true;
        }

        m_Buffer.emplace_back(buffer);
      }
    }

    void DeviceMemory::bindImageToMemory(VkImage & image, VkDeviceSize size)
    {
      {
        std::lock_guard guard(m_MutexMemory);
        vkBindImageMemory(m_Device, image, *m_Memory, m_Offset);
        
        m_Offset += size;// ((size / m_Alignment) + 1)* m_Alignment;

        if (m_Offset >= m_MaxSize) {
            m_IsFull = true;
        }
      }
    }

    bool DeviceMemory::hasEnoughSpaceLeft(VkDeviceSize size)
    { 
        bool hasEnoughSpaceLeft = m_MaxSize - m_Offset >= size;
        if (!hasEnoughSpaceLeft) m_IsFull = true;

        return hasEnoughSpaceLeft;
    }

    void DeviceMemory::clear()
    {
        for (auto buffer : m_Buffer) {
            if (VK_NULL_HANDLE != buffer)
            {
                vkDestroyBuffer(m_Device, buffer, nullptr);
            }
        }

        vkFreeMemory(m_Device, *m_Memory.get(), nullptr);

        m_MaxSize = 0;
        m_Offset = 0;
        m_IsFull = false;

        m_Memory.reset();
    }
}
