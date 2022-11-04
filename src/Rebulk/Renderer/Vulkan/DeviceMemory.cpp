#include "rebulkpch.h"
#include "DeviceMemory.h"
#include <volk.h>

namespace Rbk
{
    DeviceMemory::DeviceMemory(VkDevice device, uint32_t memoryType, VkBufferUsageFlags usage)
        : m_Device(device), m_MemoryType(memoryType), m_Usage(usage)
    {
        if (nullptr == m_Memory) {
            m_Memory = std::make_shared<VkDeviceMemory>();
            AllocateToMemory();
        }
    }

    std::shared_ptr<VkDeviceMemory> DeviceMemory::GetMemory()
    {
        if (nullptr == m_Memory) {
            m_Memory = std::make_shared<VkDeviceMemory>();
            m_Offset = 0;
            m_IsFull = false;
            AllocateToMemory();
        }

        return m_Memory;
    }

    void DeviceMemory::AllocateToMemory()
    {
        if (!m_IsAllocated) {
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = m_MaxMemorySize;
            allocInfo.memoryTypeIndex = m_MemoryType;

            if (vkAllocateMemory(m_Device, &allocInfo, nullptr, m_Memory.get()) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate buffer memory!");
            }

            m_IsAllocated = true;
        } else {
            Rbk::Log::GetLogger()->warn("trying to re allocate memory already allocated.");
        }
    }

    void DeviceMemory::BindBufferToMemory(VkBuffer buffer, VkDeviceSize size)
    {
        vkBindBufferMemory(m_Device, buffer, *m_Memory, m_Offset);
        m_Offset += size;

        if (m_Offset >= m_MaxMemorySize) {
            m_IsFull = true;
        }
    }
}